/* caller-save-test.c - ISO C99 compliant test for GCC caller-save register allocation */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Force noinline to prevent optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Helper functions with different signatures and calling conventions */
NOINLINE static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = a + b - c + d - e + f - g + h;
    return result;
}

NOINLINE float helper2(float a, float b, float c, float d, float e, float f) {
    volatile float sum = a + b + c + d + e + f;
    return sum * 0.5f;
}

NOINLINE static double helper3(double a, double b, double c, double* d, float* e) {
    volatile double temp = a * b + c;
    *d = temp;
    *e = (float)temp;
    return temp;
}

NOINLINE int helper4(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    /* Many args to force stack passing on most ABIs */
    volatile int sum = a + b + c + d + e + f + g + h + i + j;
    return sum;
}

NOINLINE static void* helper5(void* p1, void* p2, void* p3, int i, float f) {
    volatile intptr_t addr = (intptr_t)p1 + (intptr_t)p2 - (intptr_t)p3 + i + (intptr_t)(f * 100.0f);
    return (void*)addr;
}

/* Function that uses alloca to affect frame pointer */
NOINLINE static int use_alloca(int size) {
    volatile char* buf = alloca(size);
    for (int i = 0; i < size && i < 16; i++) {
        buf[i] = (char)(i * 3);
    }
    return buf[size > 0 ? size - 1 : 0];
}

/* Main test function with high register pressure */
NOINLINE static int test_caller_save(int iter) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = iter * 2;
    volatile int v2 = iter + 100;
    volatile int v3 = iter - 50;
    volatile int v4 = iter * 3;
    volatile int v5 = iter / 2;
    volatile float f1 = iter * 1.5f;
    volatile float f2 = iter * 2.5f;
    volatile float f3 = iter * 0.75f;
    volatile double d1 = iter * 1.234;
    volatile double d2 = iter * 3.456;
    volatile int* p1 = &v1;
    volatile int* p2 = &v2;
    volatile float* pf1 = &f1;
    volatile float* pf2 = &f2;
    volatile int result = 0;
    volatile float fresult = 0.0f;
    volatile double dresult = 0.0;
    
    /* Create control flow with basic blocks containing calls */
    if (iter % 3 == 0) {
        /* Block 1: Integer computations and call */
        v1 = v1 + v2 * v3 - v4;
        
        /* Inline assembly to clobber registers */
        __asm__ volatile (
            "# Clobber caller-saved registers\n"
            "movl $0x12345678, %%eax\n"
            "movl $0x9ABCDEF0, %%ecx\n"
            "movl $0x11111111, %%edx\n"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ecx", "edx", "memory"
        );
        
        /* Function call with many arguments */
        result = helper1(v1, v2, v3, v4, v5, iter, iter+1, iter+2);
        
        v1 = result + v3;
        v2 = v1 * 2;
    } else if (iter % 3 == 1) {
        /* Block 2: Floating point computations and calls */
        f1 = f1 + f2 * f3;
        
        /* Another inline assembly clobber */
        __asm__ volatile (
            "# Clobber more registers\n"
            "movq $0xDEADBEEF, %%rax\n"
            "movq $0xCAFEBABE, %%r10\n"
            : /* no outputs */
            : /* no inputs */
            : "rax", "r10", "memory"
        );
        
        fresult = helper2(f1, f2, f3, f1*0.5f, f2*1.5f, f3*2.0f);
        
        /* Mix integer and float operations */
        v1 = (int)fresult + v4;
        f1 = (float)v1 * 0.25f;
    } else {
        /* Block 3: Mixed type computations */
        d1 = d1 + d2 * 0.333;
        
        /* Use alloca to affect frame pointer */
        int alloca_result = use_alloca(iter % 64 + 16);
        
        /* Function with pointer arguments */
        double dtemp;
        float ftemp;
        dresult = helper3(d1, d2, d1*0.5, &dtemp, &ftemp);
        
        v1 = (int)dresult + alloca_result;
        f1 = ftemp * 2.0f;
    }
    
    /* Loop to create more basic blocks with calls */
    for (int i = 0; i < 3; i++) {
        volatile int loop_var = i * iter;
        
        if (i == 0) {
            /* Call with many arguments (exceeds register passing) */
            result = helper4(v1, v2, v3, v4, v5, 
                           loop_var, loop_var+1, loop_var+2,
                           iter, i);
            v1 = result ^ v2;
        } else if (i == 1) {
            /* Pointer manipulation call */
            void* ptr_result = helper5((void*)p1, (void*)p2, (void*)&loop_var,
                                      v1, f1);
            v3 = (int)(intptr_t)ptr_result;
            
            /* Clobber registers between computations */
            __asm__ volatile (
                "# Final register clobber\n"
                "movl $0x55555555, %%eax\n"
                "movl $0xAAAAAAAA, %%ebx\n"
                "movl $0x33333333, %%esi\n"
                "movl $0xCCCCCCCC, %%edi\n"
                : /* no outputs */
                : /* no inputs */
                : "eax", "ebx", "esi", "edi", "memory"
            );
        } else {
            /* Mixed computation block */
            fresult = helper2(f1, f2, f3, (float)v1, (float)v2, (float)v3);
            v4 = (int)fresult + loop_var;
        }
        
        /* Prevent loop unrolling from simplifying too much */
        __asm__ volatile ("# Loop barrier" ::: "memory");
    }
    
    /* Final computation using all variables */
    int final_result = v1 + v2 + v3 + v4 + v5 + 
                      (int)f1 + (int)f2 + (int)f3 +
                      (int)d1 + (int)d2 + result;
    
    return final_result;
}

/* Another function with different characteristics */
NOINLINE static int secondary_test(int base) {
    volatile int a = base * 11;
    volatile int b = base * 13;
    volatile int c = base * 17;
    volatile int d = base * 19;
    volatile float fa = (float)base * 1.1f;
    volatile float fb = (float)base * 1.3f;
    
    /* Multiple calls in sequence */
    int r1 = helper1(a, b, c, d, base, base+1, base+2, base+3);
    
    __asm__ volatile (
        "# Secondary test clobber\n"
        "movl $0x77777777, %%eax\n"
        "movl $0x88888888, %%ecx\n"
        ::: "eax", "ecx", "memory"
    );
    
    float r2 = helper2(fa, fb, fa*2.0f, fb*3.0f, (float)a, (float)b);
    
    int r3 = helper4(a, b, c, d, r1, (int)r2, base, base*2, base*3, base*4);
    
    return r1 + (int)r2 + r3;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing caller-save register allocation...\n");
    
    /* Multiple iterations with different values */
    for (int i = 0; i < 10; i++) {
        volatile int iter = i * 7 + 3; /* Prevent constant propagation */
        
        /* Call test function with high register pressure */
        int result1 = test_caller_save(iter);
        
        /* Call secondary test function */
        int result2 = secondary_test(iter + 5);
        
        /* Mix results */
        total_checksum += result1 ^ result2;
        
        /* Occasional alloca use in main */
        if (i % 4 == 0) {
            volatile char* temp = alloca(32);
            for (int j = 0; j < 32; j++) {
                temp[j] = (char)((i + j) & 0xFF);
            }
            total_checksum += temp[0] + temp[31];
        }
        
        /* Memory barrier to prevent reordering */
        __asm__ volatile ("# Main loop barrier" ::: "memory");
    }
    
    printf("Final checksum: %d\n", total_checksum);
    
    /* Verify with a simple computation */
    volatile int verify = total_checksum % 10007;
    if (verify == 0) {
        printf("Verification passed (mod 10007)\n");
    } else {
        printf("Verification result: %d\n", verify);
    }
    
    return 0;
}
