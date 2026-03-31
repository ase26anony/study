/* haifa_scheduler_test.c
 * Designed to trigger free_sched_context logic in GCC's Haifa scheduler
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -frandom-seed=1 haifa_scheduler_test.c -o haifa_test -fopenmp
 * For FDO: gcc -O2 -fprofile-generate haifa_scheduler_test.c -o haifa_test_gen -fopenmp
 * Then: ./haifa_test_gen
 * Then: gcc -O3 -fschedule-insns2 -fprofile-use haifa_scheduler_test.c -o haifa_test_fdo -fopenmp
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

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
            return a * c;
        } else if (a == c) {
            return b + c;
        } else {
            return a - b;
        }
    } else if (a == b) {
        if (c > 0) {
            return c * 2;
        } else {
            return -c;
        }
    } else {
        if (b > c * 2) {
            return b / (c + 1);
        } else {
            return a * b * c;
        }
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *arr, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Switch statement inside loop */
        switch (i % 7) {
            case 0:
                arr[i] = compute_hash(i, arr[i]);
                break;
            case 1:
                arr[i] = arr[i] * 3 + 1;
                break;
            case 2:
                arr[i] = arr[i] ^ 0xAAAAAAAA;
                break;
            case 3:
                arr[i] = complex_condition(arr[i], i, size);
                break;
            case 4:
                /* Memory operation with pointer arithmetic */
                *(arr + i) = *(arr + ((i + 1) % size)) + 1;
                break;
            case 5:
                /* Inline asm to create scheduling barrier */
                asm volatile("" ::: "memory");
                arr[i] = arr[i] << 2;
                break;
            case 6:
                /* Another asm with clobber */
                asm volatile("nop" ::: "cc");
                arr[i] = ~arr[i];
                break;
        }
        
        /* Inner loop with carried dependency */
        for (j = 0; j < (i % 8); j++) {
            arr[i] += j * arr[(i + j) % size];
        }
        
        /* Do-while loop */
        k = 0;
        do {
            arr[i] ^= (1 << (k % 16));
            k++;
        } while (k < 4);
    }
    
    /* Compute result with more complex flow */
    *result = 0;
    for (i = 0; i < size; i++) {
        if (arr[i] > 0) {
            for (j = 0; j < 3; j++) {
                *result += complex_condition(arr[i], j, i);
            }
        } else {
            *result -= arr[i];
        }
    }
    
    /* Artificial memory clobber */
    asm volatile("" ::: "memory");
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *arr, int size) {
    static void *labels[] = { &&label0, &&label1, &&label2, &&label3, &&label4 };
    int i = 0;
    int state = 0;
    
    /* Computed goto creates irreducible control flow */
    goto *labels[state];
    
label0:
    arr[i] = compute_hash(arr[i], i);
    state = (arr[i] % 4) + 1;
    if (++i >= size) goto end;
    goto *labels[state];
    
label1:
    arr[i] = arr[i] * 2 + 1;
    state = (arr[i] % 3) + 1;
    if (++i >= size) goto end;
    goto *labels[state];
    
label2:
    arr[i] = arr[i] ^ 0x12345678;
    state = (arr[i] % 5);
    if (++i >= size) goto end;
    goto *labels[state];
    
label3:
    arr[i] = complex_condition(arr[i], i, size);
    state = (arr[i] % 4);
    if (++i >= size) goto end;
    goto *labels[state];
    
label4:
    arr[i] = arr[i] << 1;
    state = (arr[i] % 5);
    if (++i >= size) goto end;
    goto *labels[state];
    
end:
    return;
}

/* Function with vectorization candidate */
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Simple stride-1 operations - good for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] + c[i];
        b[i] = a[i] * 2 - c[i];
        c[i] = (b[i] > a[i]) ? b[i] : a[i];
    }
    
    /* Another loop with dependencies */
    for (i = 1; i < size; i++) {
        a[i] += a[i-1];  /* Carried dependency */
    }
}

/* OpenMP parallel region */
void parallel_computation(int *arr, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < size; i++) {
        int j;
        int temp = arr[i];
        
        /* Inner computation with function calls */
        for (j = 0; j < 16; j++) {
            temp = compute_hash(temp, j);
            if (j % 3 == 0) {
                temp = complex_condition(temp, j, i);
            }
        }
        
        arr[i] = temp;
    }
}

/* Main orchestrator */
int main() {
    int i, j;
    int result = 0;
    clock_t start, end;
    double cpu_time_used;
    
    /* Allocate arrays with different sizes to test different scheduling paths */
    int sizes[] = {64, 128, 256, 512, 1024, 2048};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (i = 0; i < WARMUP_ITERATIONS; i++) {
        int warmup_arr[16];
        int warmup_result;
        
        for (j = 0; j < 16; j++) {
            warmup_arr[j] = j * i;
        }
        
        complex_control_flow(warmup_arr, 16, &warmup_result);
        result ^= warmup_result;
    }
    
    /* Main computation with different array sizes */
    printf("Main computation phase...\n");
    start = clock();
    
    for (i = 0; i < num_sizes; i++) {
        int size = sizes[i];
        int *arr1 = malloc(size * sizeof(int));
        int *arr2 = malloc(size * sizeof(int));
        int *arr3 = malloc(size * sizeof(int));
        int local_result;
        
        if (!arr1 || !arr2 || !arr3) {
            printf("Memory allocation failed!\n");
            return 1;
        }
        
        /* Initialize arrays */
        for (j = 0; j < size; j++) {
            arr1[j] = j ^ 0x55;
            arr2[j] = j * 3;
            arr3[j] = j + 1;
        }
        
        /* Call different functions to stress scheduler */
        complex_control_flow(arr1, size, &local_result);
        result += local_result;
        
        irreducible_cfg(arr2, size);
        for (j = 0; j < size; j++) {
            result ^= arr2[j];
        }
        
        vectorizable_loop(arr1, arr2, arr3, size);
        for (j = 0; j < size; j++) {
            result += arr1[j] - arr2[j] + arr3[j];
        }
        
        parallel_computation(arr3, size);
        for (j = 0; j < size; j++) {
            result = compute_hash(result, arr3[j]);
        }
        
        free(arr1);
        free(arr2);
        free(arr3);
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    /* Final verification computation */
    printf("Final verification...\n");
    int final_arr[ARRAY_SIZE];
    int final_result;
    
    for (i = 0; i < ARRAY_SIZE; i++) {
        final_arr[i] = i;
    }
    
    /* Mix of all patterns in one final call */
    for (i = 0; i < 10; i++) {
        complex_control_flow(final_arr, ARRAY_SIZE, &final_result);
        result += final_result;
        
        irreducible_cfg(final_arr, ARRAY_SIZE);
        
        #pragma omp parallel for
        for (j = 0; j < ARRAY_SIZE; j++) {
            final_arr[j] = compute_hash(final_arr[j], j);
        }
    }
    
    printf("Final checksum: %d\n", result);
    printf("Total time: %f seconds\n", cpu_time_used);
    
    return 0;
}
