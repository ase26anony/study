/* haifa_scheduler_stress.c
 * Designed to stress GCC's Haifa scheduler and trigger free_sched_context logic
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 haifa_scheduler_stress.c -o haifa_test -fopenmp
 * For FDO: First: gcc -O2 -fprofile-generate haifa_scheduler_stress.c -o haifa_test_gen -fopenmp
 *           Run: ./haifa_test_gen
 *           Then: gcc -O3 -fschedule-insns2 -fprofile-use haifa_scheduler_stress.c -o haifa_test_fdo -fopenmp
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

#define HOT __attribute__((hot))
#define ALWAYS_INLINE __attribute__((always_inline)) inline
#define NOINLINE __attribute__((noinline))

/* Helper functions for inlining */
ALWAYS_INLINE uint32_t mix_bits(uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

ALWAYS_INLINE uint64_t compute_hash(uint64_t x, uint64_t y) {
    uint64_t h = x * 0x9e3779b97f4a7c15ULL;
    h ^= y * 0xbf58476d1ce4e5b9ULL;
    h ^= h >> 32;
    return h;
}

ALWAYS_INLINE int complex_condition(int a, int b, int c) {
    int r = 0;
    r += (a & 0xF) * 3;
    r -= (b | 0x7) / 2;
    r ^= (c << 2) & 0xFF;
    asm volatile("" : "+r"(r) : : "memory");
    return r > 100;
}

/* Complex function with nested control flow */
HOT NOINLINE uint64_t complex_function_1(int *arr, int n, int threshold) {
    uint64_t result = 0;
    int i, j, k;
    
    /* Outer loop with switch inside */
    for (i = 0; i < n; i++) {
        int val = arr[i];
        
        /* Deep switch statement */
        switch (val & 0x7) {
            case 0:
                for (j = 0; j < 8; j++) {
                    result += mix_bits(val + j);
                    if (complex_condition(val, j, result & 0xFF)) {
                        result ^= 0xABCDEF;
                    }
                }
                break;
            case 1:
                /* Nested while loop */
                j = 0;
                while (j < 5) {
                    result += compute_hash(val, j);
                    j += (result & 0x1) + 1;
                }
                break;
            case 2:
                /* Do-while with if chain */
                k = 0;
                do {
                    if (k % 3 == 0) {
                        result += val * k;
                    } else if (k % 3 == 1) {
                        result -= val / (k + 1);
                    } else {
                        result ^= val << (k & 0x3);
                    }
                    k++;
                } while (k < 7);
                break;
            case 3:
                /* Computed goto for irreducible CFG */
                {
                    static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
                    int idx = val % 4;
                    goto *labels[idx];
                    
                    L0:
                        result += 0x1111;
                        goto end_case;
                    L1:
                        result += 0x2222;
                        goto end_case;
                    L2:
                        result += 0x3333;
                        goto end_case;
                    L3:
                        result += 0x4444;
                        goto end_case;
                    end_case:
                        break;
                }
            default:
                /* Deep if-else chain */
                if (val < -10) {
                    result -= 0xAAAA;
                } else if (val < 0) {
                    result -= 0xBBBB;
                } else if (val < 10) {
                    result += 0xCCCC;
                } else if (val < 20) {
                    result += 0xDDDD;
                } else if (val < 30) {
                    result += 0xEEEE;
                } else {
                    result += 0xFFFF;
                }
        }
        
        /* Memory operations to create dependencies */
        arr[i] = (result & 0xFFFFFFFF) ^ val;
        
        /* Artificial scheduling barrier */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Function with vectorizable loops */
HOT NOINLINE uint64_t vectorizable_function(int *a, int *b, int *c, int n) {
    uint64_t sum = 0;
    int i;
    
    /* Candidate for auto-vectorization */
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        a[i] = b[i] * 3 + c[i] / 2;
        sum += a[i];
    }
    
    /* Another loop with carried dependency */
    for (i = 1; i < n; i++) {
        a[i] += a[i-1] * 2;
        sum ^= a[i];
    }
    
    /* Mixed operations in loop */
    for (i = 0; i < n; i++) {
        int t = a[i];
        t = (t << 3) | (t >> 29);  /* rotate */
        t = mix_bits(t);
        t = complex_condition(t, i, sum & 0xFF) ? t + 1 : t - 1;
        a[i] = t;
        sum += t;
    }
    
    return sum;
}

/* Function with OpenMP parallel region */
HOT NOINLINE uint64_t parallel_function(int *data, int n) {
    uint64_t total = 0;
    int i;
    
    #pragma omp parallel for reduction(+:total)
    for (i = 0; i < n; i++) {
        int val = data[i];
        /* Complex computation inside parallel region */
        val = mix_bits(val);
        val += (i & 0xF) * 7;
        val ^= 0x12345678;
        val = (val * 1103515245 + 12345) & 0x7FFFFFFF;
        data[i] = val;
        total += val;
    }
    
    return total;
}

/* Function with irreducible control flow using goto */
HOT NOINLINE uint64_t irreducible_cfg(int *arr, int n) {
    uint64_t acc = 0;
    int i = 0;
    
    /* Create irreducible graph with goto */
    loop_start:
    if (i >= n) goto end;
    
    int x = arr[i];
    
    if (x < 0) goto negative;
    if (x == 0) goto zero;
    if (x > 100) goto large;
    
    /* Medium positive */
    acc += x * 2;
    goto next;
    
    negative:
    acc -= (-x) * 3;
    goto next;
    
    zero:
    acc ^= 0xDEADBEEF;
    goto next;
    
    large:
    {
        int j;
        for (j = 0; j < 3; j++) {
            acc += (x >> (j * 4)) & 0xF;
        }
    }
    
    next:
    i++;
    
    /* Jump back to different point */
    if (i % 7 == 0) goto loop_start;
    if (i % 3 == 0) goto negative_check;
    
    continue_normal:
    arr[i-1] = acc & 0xFFFFFFFF;
    goto loop_start;
    
    negative_check:
    if (x < 0 && (acc & 1)) {
        acc >>= 1;
    }
    goto continue_normal;
    
    end:
    return acc;
}

/* Main orchestrator */
int main() {
    const int sizes[] = {100, 500, 1000, 5000};
    const int num_sizes = sizeof(sizes)/sizeof(sizes[0]);
    uint64_t final_result = 0;
    int iter;
    
    /* Warm-up phase */
    printf("Starting warm-up phase...\n");
    for (iter = 0; iter < 3; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s] / (iter + 1);
            int *arr1 = malloc(n * sizeof(int));
            int *arr2 = malloc(n * sizeof(int));
            int *arr3 = malloc(n * sizeof(int));
            
            /* Initialize with pseudo-random data */
            for (int i = 0; i < n; i++) {
                arr1[i] = (i * 1103515245 + 12345) & 0x7FFF;
                arr2[i] = (i * 1664525 + 1013904223) & 0x7FFF;
                arr3[i] = (i * 22695477 + 1) & 0x7FFF;
            }
            
            /* Call complex functions */
            final_result ^= complex_function_1(arr1, n, 1000);
            final_result += vectorizable_function(arr2, arr1, arr3, n);
            final_result ^= parallel_function(arr3, n);
            final_result += irreducible_cfg(arr1, n);
            
            free(arr1);
            free(arr2);
            free(arr3);
        }
    }
    
    /* Main timed phase */
    printf("Starting main computation phase...\n");
    clock_t start = clock();
    
    for (iter = 0; iter < 10; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int n = sizes[s];
            int *arr1 = malloc(n * sizeof(int));
            int *arr2 = malloc(n * sizeof(int));
            int *arr3 = malloc(n * sizeof(int));
            
            /* Different initialization pattern */
            for (int i = 0; i < n; i++) {
                arr1[i] = ((i ^ iter) * 1103515245 + 12345) & 0xFFFF;
                arr2[i] = ((i + iter) * 1664525 + 1013904223) & 0xFFFF;
                arr3[i] = ((i * iter) * 22695477 + 1) & 0xFFFF;
            }
            
            /* Mix of function calls with different characteristics */
            uint64_t r1 = complex_function_1(arr1, n, 500 + iter * 100);
            uint64_t r2 = vectorizable_function(arr2, arr1, arr3, n);
            uint64_t r3 = parallel_function(arr3, n);
            uint64_t r4 = irreducible_cfg(arr1, n);
            
            /* Combine results with complex operations */
            final_result = mix_bits(final_result ^ r1);
            final_result = compute_hash(final_result, r2);
            final_result += r3 * 3;
            final_result ^= r4 << 32;
            
            /* Memory clobber to prevent too much optimization */
            asm volatile("" ::: "memory");
            
            free(arr1);
            free(arr2);
            free(arr3);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Verification computation */
    uint64_t verify = 0;
    for (int i = 0; i < 1000; i++) {
        verify = mix_bits(verify + i);
        verify = compute_hash(verify, final_result);
    }
    
    printf("Final result: 0x%016llX\n", (unsigned long long)final_result);
    printf("Verification: 0x%016llX\n", (unsigned long long)verify);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("Result is %s\n", (verify & 0xFFFF) == (final_result & 0xFFFF) ? "consistent" : "inconsistent");
    
    return 0;
}
