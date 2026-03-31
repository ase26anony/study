#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes to create varied call sites */

__attribute__((noinline)) 
static int helper1(int a, int b, int c, int d, int e, int f, int g, int h) {
    volatile int result = (a * b) + (c * d) - (e * f) + (g * h);
    return result;
}

__attribute__((noinline))
float helper2(float a, float b, float c, float d, float e, float f) {
    volatile float res = a * b + c * d - e * f;
    return res;
}

__attribute__((noinline))
void* helper3(void* p1, void* p2, void* p3, void* p4, 
               void* p5, void* p6, void* p7) {
    volatile uintptr_t sum = (uintptr_t)p1 + (uintptr_t)p2 + (uintptr_t)p3 +
                            (uintptr_t)p4 + (uintptr_t)p5 + (uintptr_t)p6 +
                            (uintptr_t)p7;
    return (void*)(sum & 0xFFFF);
}

__attribute__((noinline, returns_twice))
int helper4(int a, int b, int c, int d, int e) {
    volatile int x = a + b;
    volatile int y = c + d;
    return x * y - e;
}

static int __attribute__((noinline)) 
helper5(int a, int b, int c, int d, int e, int f, int g, int h, int i, int j) {
    /* Many arguments to force stack passing */
    return a + b + c + d + e + f + g + h + i + j;
}

__attribute__((noinline))
double helper6(double a, double b, double c, double d, double e) {
    volatile double t1 = a * b;
    volatile double t2 = c * d;
    return t1 + t2 - e;
}

/* Function that uses alloca to affect frame pointer */
__attribute__((noinline))
int helper_with_alloca(int size) {
    char* buffer = alloca(size);
    volatile int sum = 0;
    for (int i = 0; i < size && i < 16; i++) {
        buffer[i] = i;
        sum += buffer[i];
    }
    return sum;
}

/* Main function creating maximum register pressure */
int main(void) {
    /* Declare many local variables of mixed types */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile void* p1 = &v1, *p2 = &v2, *p3 = &v3, *p4 = &v4;
    volatile double d1 = 1.01, d2 = 2.02, d3 = 3.03;
    
    int result = 0;
    
    /* Take addresses to force stack frame complexity */
    int* ptrs[] = {&v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8, &v9, &v10};
    
    /* Loop to create basic blocks with calls inside */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* First basic block with computations */
        v1 = v1 * 2 + iteration;
        v2 = v2 / 2 + iteration;
        f1 = f1 * 1.5f + iteration;
        
        /* Inline assembly to clobber specific registers */
        /* This creates artificial live ranges across calls */
        __asm__ volatile (
            "# Clobber caller-saved registers\n"
            "mov $0x1234, %%eax\n"
            "mov $0x5678, %%ecx\n"
            "mov $0x9ABC, %%edx\n"
            : 
            : 
            : "eax", "ecx", "edx", "memory"
        );
        
        /* Function call - creates a call site */
        if (iteration % 2 == 0) {
            result += helper1(v1, v2, v3, v4, v5, v6, v7, v8);
        } else {
            result += helper5(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        }
        
        /* More computations keeping values live */
        v3 = v3 + v1;
        v4 = v4 - v2;
        f2 = f2 * f1 + iteration;
        
        /* Another inline assembly clobber */
        __asm__ volatile (
            "# Clobber more registers\n"
            "mov $0xDEAD, %%r10d\n"
            "mov $0xBEEF, %%r11d\n"
            : 
            : 
            : "r10", "r11", "memory"
        );
        
        /* Different type of function call */
        float fres = helper2(f1, f2, f3, f4, f5, f1 + f2);
        result += (int)fres;
        
        /* Complex conditional creating basic block boundaries */
        if (v1 > v2 && f1 < f3) {
            /* Nested basic block with call */
            void* pres = helper3(p1, p2, p3, p4, &f1, &f2, &iteration);
            result += (int)(uintptr_t)pres;
            
            v5 = v5 * 3 - iteration;
            v6 = v6 / 3 + iteration;
        } else {
            /* Alternative path with different call */
            int alloca_result = helper_with_alloca(32 + iteration);
            result += alloca_result;
            
            v7 = v7 * 4 - iteration;
            v8 = v8 / 4 + iteration;
        }
        
        /* More register-intensive computations */
        d1 = d1 * 2.0 + iteration;
        d2 = d2 / 2.0 - iteration;
        
        /* Double precision call */
        double dres = helper6(d1, d2, d3, d1 + 1.0, d2 - 1.0);
        result += (int)dres;
        
        /* Final computations in the loop */
        v9 = v9 + result % 100;
        v10 = v10 - result % 50;
        f3 = f3 + (float)result / 1000.0f;
        
        /* Another call with returns_twice attribute */
        result += helper4(v9, v10, v1, v2, iteration);
    }
    
    /* Additional control flow outside loop */
    volatile int final_check = 0;
    for (int i = 0; i < 5; i++) {
        /* Mix of calls and computations in loop body */
        if (i % 2 == 0) {
            final_check += helper1(v1 + i, v2 - i, v3 * i, v4 / (i+1),
                                   v5, v6, v7, v8);
        } else {
            final_check += helper5(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10);
        }
        
        /* Force register pressure with inline asm */
        __asm__ volatile (
            "# Final register clobber\n"
            "mov $0xCAFE, %%eax\n"
            "mov $0xBABE, %%ebx\n"
            : 
            : 
            : "eax", "ebx", "memory"
        );
    }
    
    result += final_check;
    
    /* Compute and print checksum */
    int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                   (int)d1 + (int)d2 + (int)d3 + result;
    
    printf("Final checksum: %d\n", checksum);
    return checksum == 0 ? 0 : 1;
}
