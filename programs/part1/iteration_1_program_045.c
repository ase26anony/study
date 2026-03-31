#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Helper functions with different attributes and calling conventions */

/* Function that returns a value and takes many arguments (exceeds register passing) */
static int __attribute__((noinline)) 
many_args_func(int a, int b, int c, int d, int e, int f, int g, int h, 
                int i, int j, int k, int l, float m, double n, void* p) {
    volatile int result = a + b + c + d;
    result += e + f + g + h;
    result += i + j + k + l;
    result += (int)m + (int)n + (int)(intptr_t)p;
    return result;
}

/* Function that clobbers registers via inline assembly */
static void __attribute__((noinline)) 
clobber_registers(void) {
    /* Force clobbering of call-clobbered registers */
    __asm__ volatile (
        "# Clobber caller-saved registers\n"
        "mov $0x12345678, %%eax\n"
        "mov $0x87654321, %%ecx\n"
        "mov $0x11111111, %%edx\n"
        : 
        : 
        : "eax", "ecx", "edx", "memory"
    );
}

/* Function with frame pointer interaction */
static int __attribute__((noinline))
use_frame_pointer(int x) {
    int local1 = x * 2;
    int local2 = x * 3;
    volatile int* ptr = &local1;  /* Taking address forces frame pointer */
    int local3 = *ptr + local2;
    
    /* Use alloca to force frame pointer usage */
    char* buffer = (char*)alloca(64);
    for (int i = 0; i < 64; i++) {
        buffer[i] = (char)(local3 + i);
    }
    
    return local3 + (int)buffer[0];
}

/* Function that returns float */
static float __attribute__((noinline))
float_computation(float a, float b, float c, float d, float e) {
    volatile float result = a * b + c / d - e;
    
    /* More inline assembly clobbering */
    __asm__ volatile (
        "# Clobber more registers\n"
        "mov $0xAAAAAAAA, %%r10d\n"
        "mov $0xBBBBBBBB, %%r11d\n"
        : 
        : 
        : "r10", "r11", "memory"
    );
    
    return result * 2.0f;
}

/* Static function that can be inlined */
static int inline_candidate(int x, int y) {
    return x * y + (x >> 3) - (y << 2);
}

/* Main computation with high register pressure */
int main(void) {
    /* Declare many local variables to maximize register pressure */
    volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile int* p1 = &v1;
    volatile float* p2 = &f1;
    volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    volatile double d1 = 1.234, d2 = 5.678;
    volatile int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    
    int result = 0;
    
    /* Create complex control flow with basic blocks containing calls */
    for (int iteration = 0; iteration < 3; iteration++) {
        if (iteration % 2 == 0) {
            /* First basic block with function calls */
            v1 = v2 + v3;
            v4 = v5 * v6;
            
            /* Call with many arguments - forces register pressure */
            int call_result = many_args_func(v1, v2, v3, v4, v5, v6, v7, v8,
                                            v9, v10, v11, v12, f1, d1, (void*)p1);
            
            /* Use inline assembly between computations and calls */
            __asm__ volatile (
                "# Intermediate clobber\n"
                "mov %0, %%eax\n"
                "add $100, %%eax\n"
                : 
                : "r" (v1)
                : "eax", "memory"
            );
            
            f1 = float_computation(f1, f2, f3, f4, f5);
            
            v7 = inline_candidate(v7, v8);
            
            /* Another call with frame pointer interaction */
            v9 = use_frame_pointer(v9);
            
            result += call_result + (int)f1 + v7 + v9;
        } else {
            /* Different basic block path */
            v2 = v3 - v4;
            v5 = v6 / 2;
            
            /* Force register clobbering */
            clobber_registers();
            
            /* Another many-argument call */
            int call_result2 = many_args_func(v15, v14, v13, v12, v11, v10,
                                             v9, v8, v7, v6, v5, v4, f5, d2, (void*)p2);
            
            /* More computations between calls */
            f2 = f3 * f4 - f5;
            
            __asm__ volatile (
                "# More register clobbering\n"
                "mov $0xDEADBEEF, %%ecx\n"
                "mov $0xCAFEBABE, %%edx\n"
                : 
                : 
                : "ecx", "edx", "memory"
            );
            
            v10 = use_frame_pointer(v10);
            v11 = inline_candidate(v11, v12);
            
            result += call_result2 + (int)f2 + v10 + v11;
        }
        
        /* Loop creates additional basic blocks */
        for (int inner = 0; inner < 2; inner++) {
            /* Nested loop with calls creates complex CFG */
            v12 = v13 + v14 + inner;
            
            if (inner == 0) {
                f3 = float_computation(f2, f3, f4, f5, f1);
                v13 = many_args_func(v12, v11, v10, v9, v8, v7, v6, v5,
                                    v4, v3, v2, v1, f3, d1, (void*)&v15);
            } else {
                clobber_registers();
                v14 = use_frame_pointer(v14);
            }
            
            result += v12 + (int)f3 + v13 + v14;
        }
    }
    
    /* Final computation and output */
    printf("Result checksum: %d\n", result);
    
    /* Verify with expected value */
    int expected = 0;  /* Calculate based on logic */
    if (result != expected) {
        printf("Verification failed! Got %d, expected %d\n", result, expected);
        return 1;
    }
    
    return 0;
}
