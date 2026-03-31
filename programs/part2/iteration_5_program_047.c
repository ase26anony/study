/* Compile with: gcc -O3 -fschedule-insns -funroll-loops=2 -march=nehalem -mtune=nehalem -fdump-rtl-sched -fdump-rtl-sched2 -o scheduler_test scheduler_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Force use of specific scheduling model hooks */
#ifdef __x86_64__
__attribute__((target("arch=nehalem")))
#endif
static void complex_scheduling_kernel(int *arr1, int *arr2, int size) {
    /* High register pressure: many local variables */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    float f0, f1, f2, f3, f4, f5, f6, f7;
    volatile int mem_barrier; /* Prevent optimization across barriers */
    
    /* Initialize with non-trivial values */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5; v5 = 6; v6 = 7; v7 = 8;
    v8 = 9; v9 = 10; v10 = 11; v11 = 12; v12 = 13; v13 = 14; v14 = 15; v15 = 16;
    f0 = 1.1f; f1 = 2.2f; f2 = 3.3f; f3 = 4.4f; f4 = 5.5f; f5 = 6.6f; f6 = 7.7f; f7 = 8.8f;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent branch with hard-to-predict condition */
        int branch_cond = arr1[i] & 0x7F; /* Use lower bits for more randomness */
        
        /* Force scheduler to consider speculative motion */
        if (__builtin_expect((branch_cond > 64), 0)) {
            /* Path A: Integer-heavy operations with memory barriers */
            v0 = v1 + arr2[i];
            v1 = v2 * v3;
            asm volatile("" ::: "memory"); /* Barrier 1 */
            v2 = v4 - v5;
            v3 = v6 ^ v7;
            v4 = v8 | v9;
            asm volatile("" ::: "memory"); /* Barrier 2 */
            v5 = v10 & v11;
            v6 = v12 << 2;
            v7 = v13 >> 1;
            
            /* Floating point ops mixed in */
            f0 = f1 * 1.5f;
            f1 = f2 + f3;
            asm volatile("" ::: "memory"); /* Barrier 3 */
            
            /* Complex dependency chain */
            v8 = v0 + v1 + v2;
            v9 = v3 * v4 - v5;
            v10 = (v6 << 1) | (v7 & 0xFF);
            
            /* Memory operation to increase pressure */
            mem_barrier = arr1[(i + 1) % size];
            v11 = v8 + mem_barrier;
            
            /* Another barrier before merge */
            asm volatile("" ::: "memory");
            
            /* Jump to common code */
            goto merge_point;
        } else {
            /* Path B: Different operation mix */
            v12 = arr1[i] * 3;
            v13 = v14 + v15;
            asm volatile("" ::: "memory"); /* Barrier 4 */
            v14 = v0 ^ v1;
            v15 = v2 & v3;
            
            /* More floating point ops */
            f2 = f3 / 2.0f;
            f3 = f4 - f5;
            asm volatile("" ::: "memory"); /* Barrier 5 */
            
            /* Different dependency pattern */
            v0 = v12 + v13;
            v1 = v14 * v15;
            v2 = (v0 << 3) | (v1 & 0x7F);
            
            /* Another memory access */
            mem_barrier = arr2[(i + 2) % size];
            v3 = v2 - mem_barrier;
            
            /* Switch statement to create control flow complexity */
            switch (branch_cond & 0x3) {
                case 0:
                    v4 = v3 + 1;
                    break;
                case 1:
                    v4 = v3 * 2;
                    break;
                case 2:
                    v4 = v3 - 1;
                    break;
                default:
                    v4 = v3 ^ 0xFF;
                    break;
            }
            
            /* More barriers */
            asm volatile("" ::: "memory");
            
            /* Fall through to merge point */
        }
        
    merge_point:
        /* Common merging code with more operations */
        f4 = f0 + f1;
        f5 = f2 * f3;
        v5 = v4 + v11;
        v6 = v5 * v0;
        
        /* Final barrier in loop */
        asm volatile("" ::: "memory");
        
        /* Store result back to prevent elimination */
        arr1[i] = (v6 + (int)f4) & 0xFFFF;
    }
    
    /* Use all variables to prevent dead code elimination */
    int checksum = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    checksum += (int)(f0 + f1 + f2 + f3 + f4 + f5 + f6 + f7);
    
    /* Volatile store to force computation */
    volatile int *volatile_ptr = &mem_barrier;
    *volatile_ptr = checksum;
}

/* Additional kernel with different pattern to increase scheduling complexity */
#ifdef __x86_64__
__attribute__((target("arch=core2")))
#endif
static void second_kernel(int *arr, int size) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    for (int idx = 0; idx < size; idx += 2) {
        /* Unrolled loop with data-dependent operations */
        int val1 = arr[idx];
        int val2 = arr[idx + 1];
        
        if (__builtin_expect((val1 & 1), 1)) {
            a = b + val1;
            b = c * d;
            c = e ^ f;
            asm volatile("" ::: "memory");
            d = g | h;
            e = i & j;
        } else {
            f = k - val2;
            g = l << 1;
            h = m >> 2;
            asm volatile("" ::: "memory");
            i = n + o;
            j = p * 2;
        }
        
        /* Cross-path dependencies */
        k = a + f;
        l = b + g;
        m = c + h;
        n = d + i;
        o = e + j;
        
        asm volatile("" ::: "memory");
        
        /* Store with computation */
        arr[idx] = (k + l + m) & 0xFF;
        arr[idx + 1] = (n + o + p) & 0xFF;
        
        /* Rotate values to create live range conflicts */
        p = o; o = n; n = m; m = l; l = k;
        k = j; j = i; i = h; h = g; g = f;
        f = e; e = d; d = c; c = b; b = a;
    }
}

int main() {
    const int SIZE = 256;
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random values */
    unsigned int seed = time(NULL);
    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        array1[i] = (seed >> 16) & 0x7FFF;
        seed = seed * 1103515245 + 12345;
        array2[i] = (seed >> 16) & 0x7FFF;
    }
    
    /* Call scheduling-intensive kernels multiple times */
    for (int iter = 0; iter < 100; iter++) {
        complex_scheduling_kernel(array1, array2, SIZE);
        second_kernel(array2, SIZE);
        
        /* Swap arrays to create different data patterns */
        int *temp = array1;
        array1 = array2;
        array2 = temp;
    }
    
    /* Compute final checksum to prevent optimization */
    int final_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        final_sum += array1[i] + array2[i];
    }
    
    printf("Result checksum: %d\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
