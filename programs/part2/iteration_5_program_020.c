/* haifa_sched_trigger.c
 * Program designed to trigger GCC Haifa scheduler state save/restore
 * and execute the uncovered cleanup code in haifa-sched.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force complex scheduling decisions with architecture-specific tuning */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#elif defined(__aarch64__)
__attribute__((target("arch=armv8-a+crc")))
#endif
static void compute_intensive_kernel(int *arr1, int *arr2, int size, int *result) {
    /* High register pressure: many local variables in a small scope */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    int sum = 0;
    
    /* Initialize with volatile reads to prevent optimization */
    volatile int init_val = 42;
    v0 = init_val;
    v1 = v0 + 1;
    v2 = v1 * 2;
    v3 = v2 - v0;
    
    /* Create data-dependent control flow with unpredictable branches */
    for (int i = 0; i < size; i++) {
        /* Hard-to-predict branch using array data */
        if (__builtin_expect((arr1[i] & 0x7F) > (arr2[i] & 0x3F), 0)) {
            /* Path A: Complex integer arithmetic chain */
            v4 = arr1[i] * v0;
            v5 = arr2[i] + v1;
            v6 = v4 ^ v5;
            v7 = v6 << (arr1[i] & 0x3);
            v8 = v7 - v2;
            v9 = v8 * v3;
            v10 = v9 >> 1;
            v11 = v10 | v4;
            v12 = v11 & 0xFFFF;
            
            /* Mix in floating point operations */
            f0 = (float)v4 * 1.5f;
            f1 = (float)v5 * 0.75f;
            f2 = f0 + f1;
            f3 = f2 - (float)v6;
            
            /* Memory barrier to force serialization point */
            asm volatile("" ::: "memory");
            
            /* More operations after barrier */
            v13 = (int)f3 + v7;
            v14 = v13 * v8;
            v15 = v14 ^ v9;
            
            f4 = (float)v10 * 2.0f;
            f5 = (float)v11 * 1.25f;
            f6 = f4 - f5;
            f7 = f6 + (float)v12;
            
            sum += v15 + (int)f7;
        } else {
            /* Path B: Different arithmetic pattern */
            v4 = arr1[i] + v0;
            v5 = arr2[i] - v1;
            v6 = v4 | v5;
            v7 = v6 >> (arr2[i] & 0x3);
            v8 = v7 + v2;
            v9 = v8 / (v3 ? v3 : 1);
            v10 = v9 ^ v4;
            v11 = v10 & 0x7FFF;
            v12 = v11 << 2;
            
            /* Different floating point pattern */
            f0 = (float)v5 * 2.5f;
            f1 = (float)v6 * 0.5f;
            f2 = f0 - f1;
            f3 = f2 + (float)v7;
            
            /* Memory barrier at different position */
            asm volatile("" ::: "memory");
            
            /* Continue with different operations */
            v13 = (int)f3 * v8;
            v14 = v13 - v9;
            v15 = v14 | v10;
            
            f4 = (float)v11 / 3.0f;
            f5 = (float)v12 * 1.75f;
            f6 = f4 + f5;
            f7 = f6 - (float)v13;
            
            sum += v15 - (int)f7;
        }
        
        /* Complex switch statement creating control flow convergence */
        switch (arr1[i] & 0x3) {
            case 0:
                v0 = v0 + v15;
                goto common_label;
            case 1:
                v1 = v1 ^ v14;
                /* Fall through */
            case 2:
                v2 = v2 * v13;
                common_label:
                v3 = v3 | v12;
                break;
            case 3:
                v0 = v0 - v11;
                v1 = v1 + v10;
                v2 = v2 ^ v9;
                v3 = v3 & v8;
                break;
        }
        
        /* Another memory barrier before loop continues */
        asm volatile("" ::: "memory");
    }
    
    /* Use all variables to prevent dead code elimination */
    *result = sum + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
              v11 + v12 + v13 + v14 + v15 + (int)f0 + (int)f1 + (int)f2 + 
              (int)f3 + (int)f4 + (int)f5 + (int)f6 + (int)f7;
}

/* Function pointer to inhibit optimization */
typedef void (*compute_func_t)(int*, int*, int, int*);
volatile compute_func_t func_ptr = compute_intensive_kernel;

int main(void) {
    const int SIZE = 256;
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    int result1 = 0, result2 = 0;
    
    /* Initialize with pseudo-random values using LCG */
    uint32_t seed = 123456789;
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr1[i] = (int)(seed & 0x7FFF);
        seed = seed * 1103515245 + 12345;
        arr2[i] = (int)(seed & 0x3FFF);
    }
    
    /* Call through volatile function pointer to prevent inlining */
    func_ptr(arr1, arr2, SIZE, &result1);
    
    /* Second call with modified data to create different scheduling decisions */
    for (int i = 0; i < SIZE; i += 3) {
        arr1[i] ^= 0x5555;
        arr2[i] += 0x3333;
    }
    
    func_ptr(arr1, arr2, SIZE, &result2);
    
    /* Compute final checksum to prevent optimization */
    int final_result = result1 ^ result2;
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", final_result);
    
    free(arr1);
    free(arr2);
    
    return final_result != 0 ? 0 : 1;
}
