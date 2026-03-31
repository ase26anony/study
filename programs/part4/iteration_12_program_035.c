/* haifa_scheduler_test.c
 * Designed to trigger free_sched_context logic in GCC's Haifa scheduler
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 haifa_scheduler_test.c -o haifa_test -fopenmp
 * For FDO: First: gcc -O2 -fprofile-generate haifa_scheduler_test.c -o haifa_test_gen -fopenmp
 *           Run: ./haifa_test_gen
 *           Then: gcc -O3 -fschedule-insns2 -fprofile-use haifa_scheduler_test.c -o haifa_test_fdo -fopenmp
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define WARMUP_ITERATIONS 1000

/* Always inline helper functions to create complex dataflow */
static inline uint32_t __attribute__((always_inline)) 
compute_hash(uint32_t x, uint32_t y) {
    /* Mix of arithmetic and bitwise operations */
    x = (x ^ (x >> 16)) * 0x85ebca6b;
    y = (y ^ (y >> 13)) * 0xc2b2ae35;
    return (x ^ y) + (x << 6) + (y >> 2);
}

static inline int __attribute__((always_inline))
complex_condition(int a, int b, int c) {
    /* Deep conditional chain */
    if (a > b) {
        if (b < c) {
            return a * c - b;
        } else if (a == c) {
            return (a << 3) | (b & 0xF);
        } else {
            return (a ^ b ^ c) + 1;
        }
    } else if (a == b) {
        return (c * 7) >> 2;
    } else {
        if (c > 0) {
            return (a + b) * c;
        } else {
            return (a - b) | c;
        }
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void process_array_complex(int* restrict arr, int size, int threshold) {
    volatile int barrier = 0; /* Memory barrier */
    int i, j, k;
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Switch statement with multiple cases */
        switch (i % 7) {
            case 0:
                arr[i] = compute_hash(arr[i], i);
                break;
            case 1:
                arr[i] = arr[i] * 3 + 1;
                break;
            case 2:
                arr[i] = arr[i] ^ (arr[i] >> 1);
                break;
            case 3:
                arr[i] = complex_condition(arr[i], i, threshold);
                break;
            case 4:
                /* Memory operation with pointer arithmetic */
                *(arr + i) = *(arr + ((i + 1) % size)) + 1;
                break;
            case 5:
                /* Inline asm to create scheduling barrier */
                asm volatile("" ::: "memory");
                arr[i] = arr[i] * arr[(i * 3) % size];
                break;
            case 6:
                arr[i] = (arr[i] << 4) | (arr[i] >> 28);
                break;
        }
        
        /* Inner loop with carried dependency */
        for (j = 0; j < (i % 5); j++) {
            arr[i] += (j * arr[i]) % 256;
        }
        
        /* Conditional goto for irreducible control flow */
        if (arr[i] > threshold * 2) {
            goto process_large;
        } else if (arr[i] < -threshold) {
            goto process_small;
        }
        
        continue;
        
    process_large:
        arr[i] >>= 2;
        continue;
        
    process_small:
        arr[i] <<= 1;
        continue;
    }
    
    /* Computed goto for irreducible control flow */
    static void* labels[] = { &&loop_start, &&loop_end, &&process_again };
    
    k = 0;
loop_start:
    if (k >= size) goto loop_end;
    
    /* Mix of operations to stress scheduler */
    arr[k] = (arr[k] * 13 + 7) & 0xFF;
    
    /* Function call from multiple sites */
    arr[k] = complex_condition(arr[k], k, arr[(k + 1) % size]);
    
    /* Memory clobber */
    asm volatile("" ::: "memory");
    
    goto *labels[(arr[k] > 0) ? 0 : 1];
    
process_again:
    arr[k] = compute_hash(arr[k], arr[(k + size/2) % size]);
    k++;
    goto loop_start;
    
loop_end:
    barrier = 1; /* Ensure side effect */
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(float* restrict a, float* restrict b, float* restrict c, int n) {
    int i;
    
    /* Simple stride-1 loop for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < n; i++) {
        a[i] = b[i] * c[i] + 1.0f;
        /* Create some dependencies */
        if (i > 0) {
            a[i] += a[i-1] * 0.1f;
        }
    }
    
    /* Another loop with reduction */
    float sum = 0.0f;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; i++) {
        sum += a[i] * b[i];
        /* Conditional inside vectorized loop */
        if (a[i] > 0.5f) {
            c[i] = sum;
        }
    }
}

/* Function with OpenMP parallel region */
void parallel_region(int* data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < size; i++) {
        /* Complex operations inside parallel region */
        int val = data[i];
        
        /* Deep if-else chain */
        if (val % 2 == 0) {
            val = compute_hash(val, i);
            if (val > 1000) {
                val >>= 3;
            } else if (val > 100) {
                val = complex_condition(val, i, size);
            } else {
                val = (val * 17 + 3) & 0xFFFF;
            }
        } else {
            for (int j = 0; j < 3; j++) {
                val = (val ^ (val << 5)) & 0xFFFFFF;
            }
        }
        
        data[i] = val;
        
        /* Memory barrier in parallel region */
        asm volatile("" ::: "memory");
    }
}

/* Function with irreducible control flow using computed gotos */
__attribute__((noinline))
void irreducible_cfg(int* arr, int n) {
    static void* jump_table[] = { &&case0, &&case1, &&case2, &&case3, &&default_case };
    
    int i = 0;
    while (i < n) {
        int selector = arr[i] % 5;
        
        /* Computed goto */
        goto *jump_table[selector];
        
    case0:
        arr[i] += i * 2;
        i++;
        continue;
        
    case1:
        arr[i] = arr[i] ^ 0xAAAAAAAA;
        i += 2;
        continue;
        
    case2:
        arr[i] = complex_condition(arr[i], arr[i+1], n);
        i++;
        continue;
        
    case3:
        for (int j = 0; j < 4; j++) {
            arr[i+j] = compute_hash(arr[i+j], j);
        }
        i += 4;
        continue;
        
    default_case:
        arr[i] = arr[i] * 3 + 7;
        i++;
        continue;
    }
}

/* Main orchestrator */
int main() {
    int i, iter;
    clock_t start, end;
    double total_time = 0.0;
    
    /* Allocate arrays with different alignments */
    int* array1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* array2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    float* farray1 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farray2 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    float* farray3 = (float*)aligned_alloc(64, ARRAY_SIZE * sizeof(float));
    
    if (!array1 || !array2 || !farray1 || !farray2 || !farray3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with varying patterns */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        farray1[i] = (float)rand() / RAND_MAX;
        farray2[i] = (float)rand() / RAND_MAX;
        farray3[i] = (float)rand() / RAND_MAX;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase (%d iterations)...\n", WARMUP_ITERATIONS);
    for (iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        process_array_complex(array1, ARRAY_SIZE / 4, 500);
        vectorizable_loop(farray1, farray2, farray3, ARRAY_SIZE / 8);
    }
    
    /* Main timed execution with multiple patterns */
    printf("Main execution phase (%d iterations)...\n", ITERATIONS);
    start = clock();
    
    uint64_t checksum = 0;
    
    for (iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different functions to stress scheduler */
        switch (iter % 5) {
            case 0:
                process_array_complex(array1, ARRAY_SIZE, 500 + (iter % 100));
                break;
            case 1:
                vectorizable_loop(farray1, farray2, farray3, ARRAY_SIZE);
                break;
            case 2:
                parallel_region(array2, ARRAY_SIZE);
                break;
            case 3:
                irreducible_cfg(array1, ARRAY_SIZE / 2);
                break;
            case 4:
                /* Mix all patterns */
                process_array_complex(array2, ARRAY_SIZE / 2, 300);
                vectorizable_loop(farray2, farray3, farray1, ARRAY_SIZE / 2);
                parallel_region(array1, ARRAY_SIZE / 4);
                break;
        }
        
        /* Update checksum to prevent dead code elimination */
        for (i = 0; i < ARRAY_SIZE; i += 64) {
            checksum += array1[i] + array2[i];
            checksum += (uint64_t)(farray1[i] * 1000);
        }
        
        /* Vary array sizes to trigger different scheduling decisions */
        if (iter % 100 == 0) {
            int size = ARRAY_SIZE / (1 + (iter / 100) % 4);
            process_array_complex(array1, size, 200);
        }
    }
    
    end = clock();
    total_time = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Final computation for verification */
    uint64_t final_checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_checksum += array1[i] + array2[i];
        final_checksum += (uint64_t)(farray1[i] * 1000);
        final_checksum += (uint64_t)(farray2[i] * 1000);
        final_checksum += (uint64_t)(farray3[i] * 1000);
    }
    
    printf("Execution time: %.2f seconds\n", total_time);
    printf("Final checksum: %llu\n", (unsigned long long)final_checksum);
    printf("Intermediate checksum: %llu\n", (unsigned long long)checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray1);
    free(farray2);
    free(farray3);
    
    return 0;
}
