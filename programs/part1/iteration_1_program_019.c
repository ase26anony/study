#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Helper functions with different attributes to create diverse call sites */

/* Function that returns a value and uses many arguments */
static int __attribute__((noinline)) 
compute_sum(int a, int b, int c, int d, int e, int f, int g, int h, 
            float fa, float fb, double da, double db) {
    volatile int v1 = a + b;
    volatile int v2 = c + d;
    volatile float v3 = fa + fb;
    volatile double v4 = da + db;
    
    /* Force register pressure in helper too */
    int r1 = v1 * 2;
    int r2 = v2 * 3;
    float r3 = v3 * 1.5f;
    double r4 = v4 * 2.5;
    
    /* Inline assembly that clobbers registers */
    __asm__ volatile (
        "movl $0x12345678, %%eax\n\t"
        "movl $0x9ABCDEF0, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ebx", "memory"
    );
    
    return r1 + r2 + (int)r3 + (int)r4 + e + f + g + h;
}

/* Another function with pointer arguments */
static float __attribute__((noinline))
process_floats(float *arr, int n, float scale, float offset) {
    volatile float result = 0.0f;
    for (int i = 0; i < n; i++) {
        result += arr[i] * scale + offset;
    }
    
    /* More inline assembly clobbering */
    __asm__ volatile (
        "movq $0x1122334455667788, %%r10\n\t"
        "xorq %%r10, %%r10\n\t"
        : /* no outputs */
        : /* no inputs */
        : "r10", "memory"
    );
    
    return result;
}

/* Function using alloca to affect frame pointer */
static void* __attribute__((noinline))
create_buffer(int size) {
    /* Taking address of local variable and using alloca 
       discourages frame pointer omission */
    int local_var = size * 2;
    int *ptr = &local_var;
    
    void *buffer = alloca(size + *ptr);
    
    __asm__ volatile (
        "movl $0xDEADBEEF, %%ecx\n\t"
        "movl $0xCAFEBABE, %%edx\n\t"
        : /* no outputs */
        : /* no inputs */
        : "ecx", "edx", "memory"
    );
    
    return buffer;
}

/* Function with mixed types and many live values */
static double __attribute__((noinline))
complex_calculation(int a, float b, double c, int *d, float *e, double *f) {
    volatile int vi1 = a * 2;
    volatile float vf1 = b * 3.14f;
    volatile double vd1 = c * 2.71828;
    
    /* Create many intermediate values */
    int i1 = vi1 + 1;
    int i2 = vi1 * 2;
    int i3 = i1 + i2;
    int i4 = i3 - vi1;
    
    float f1 = vf1 + 1.0f;
    float f2 = vf1 * 2.0f;
    float f3 = f1 + f2;
    float f4 = f3 - vf1;
    
    double d1 = vd1 + 1.0;
    double d2 = vd1 * 2.0;
    double d3 = d1 + d2;
    double d4 = d3 - vd1;
    
    /* Force these values to be used */
    *d = i1 + i2 + i3 + i4;
    *e = f1 + f2 + f3 + f4;
    *f = d1 + d2 + d3 + d4;
    
    return (double)(*d) + (double)(*e) + *f;
}

/* Main function creating maximum register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f;
    volatile double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;
    volatile int *p1 = &v1, *p2 = &v2;
    volatile float *fp1 = &f1, *fp2 = &f2;
    volatile double *dp1 = &d1, *dp2 = &d2;
    
    int result_int = 0;
    float result_float = 0.0f;
    double result_double = 0.0;
    uint64_t checksum = 0;
    
    /* Control flow to create basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* First basic block with function calls */
            
            /* Inline assembly between computations to force clobbering */
            __asm__ volatile (
                "movl $0xAAAAAAAA, %%eax\n\t"
                "movl $0xBBBBBBBB, %%ebx\n\t"
                "movl $0xCCCCCCCC, %%ecx\n\t"
                "movl $0xDDDDDDDD, %%edx\n\t"
                : /* no outputs */
                : /* no inputs */
                : "eax", "ebx", "ecx", "edx", "memory"
            );
            
            /* Call with many arguments - exceeds register passing on most arches */
            int sum = compute_sum(v1, v2, v3, v4, v5, 
                                 v1+v2, v3+v4, v5*2,
                                 f1, f2, d1, d2);
            
            /* More computations keeping values live */
            v1 = sum % 100;
            v2 = (sum + v1) % 200;
            v3 = v1 * v2 + v3;
            
            __asm__ volatile (
                "movq $0x1111111111111111, %%r10\n\t"
                "movq $0x2222222222222222, %%r11\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r10", "r11", "memory"
            );
            
            /* Create array for next call */
            float arr[8];
            for (int i = 0; i < 8; i++) {
                arr[i] = f1 + i * f2;
            }
            
            /* Another function call */
            result_float += process_floats(arr, 8, f3, f4);
            
        } else {
            /* Second basic block with different calls */
            
            /* Use alloca to affect frame pointer */
            void *buffer = create_buffer(64 + iteration * 16);
            
            /* Prevent optimization of buffer */
            __asm__ volatile (
                "movq %0, %%rsi\n\t"
                "movb $0xFF, (%%rsi)\n\t"
                : /* no outputs */
                : "r" (buffer)
                : "rsi", "memory"
            );
            
            /* Complex calculation with pointer outputs */
            int out_int;
            float out_float;
            double out_double;
            
            result_double += complex_calculation(v4, f4, d4, 
                                                &out_int, &out_float, &out_double);
            
            /* Mix results */
            result_int += out_int;
            result_float += out_float;
            
            __asm__ volatile (
                "movl $0xEEEEEEEE, %%r8d\n\t"
                "movl $0xFFFFFFFF, %%r9d\n\t"
                : /* no outputs */
                : /* no inputs */
                : "r8", "r9", "memory"
            );
        }
        
        /* Loop-carried dependencies to keep values live */
        v4 = v3 + v4;
        f4 = f3 + f4;
        d4 = d3 + d4;
        
        /* More inline assembly clobbering */
        __asm__ volatile (
            "xchgq %%rax, %%rbx\n\t"
            "xchgq %%rbx, %%rax\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "memory"
        );
    }
    
    /* Final computations using all variables */
    checksum = (uint64_t)v1 + (uint64_t)v2 + (uint64_t)v3 + (uint64_t)v4 + (uint64_t)v5;
    checksum += (uint64_t)(f1 * 1000) + (uint64_t)(f2 * 1000) + 
                (uint64_t)(f3 * 1000) + (uint64_t)(f4 * 1000);
    checksum += (uint64_t)(d1 * 1000) + (uint64_t)(d2 * 1000) + 
                (uint64_t)(d3 * 1000) + (uint64_t)(d4 * 1000);
    checksum += (uint64_t)result_int + (uint64_t)(result_float * 1000) + 
                (uint64_t)(result_double * 1000);
    
    /* Use pointer variables */
    checksum += (uint64_t)(*p1) + (uint64_t)(*p2);
    checksum += (uint64_t)(*fp1 * 1000) + (uint64_t)(*fp2 * 1000);
    checksum += (uint64_t)(*dp1 * 1000) + (uint64_t)(*dp2 * 1000);
    
    printf("Checksum: %llu\n", (unsigned long long)checksum);
    
    /* Verify execution */
    if (checksum != 0) {
        printf("Execution completed successfully.\n");
        return 0;
    } else {
        printf("Error: checksum is zero.\n");
        return 1;
    }
}
