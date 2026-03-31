/* haifa_scheduler_stress_test.c
 * A comprehensive test to stress GCC's Haifa scheduler and trigger
 * free_sched_context cleanup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    /* Mix of arithmetic and bitwise operations */
    return ((x << 5) ^ y) + (x >> 3) * 7;
}

static inline int __attribute__((always_inline))
data_transform(int val, int mask) {
    /* Complex dependency chain */
    int t1 = val & mask;
    int t2 = (val ^ mask) << 1;
    int t3 = t1 * 3 + t2;
    return (t3 >> 4) | (t3 << 28);
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int acc = data[i];
        
        /* Deep if-else chain */
        if (i % 3 == 0) {
            acc = compute_hash(acc, i);
            if (i % 7 == 0) {
                acc += data[(i * 2) % size];
                if (i % 11 == 0) {
                    acc ^= 0xDEADBEEF;
                } else {
                    acc |= 0xCAFEBABE;
                }
            } else {
                acc -= data[(i * 3) % size];
            }
        } else if (i % 5 == 0) {
            acc = data_transform(acc, 0xFF00FF);
            switch (i % 8) {
                case 0: acc <<= 1; break;
                case 1: acc >>= 2; break;
                case 2: acc *= 3; break;
                case 3: acc /= 2; break;
                case 4: acc ^= acc >> 16; break;
                case 5: acc = ~acc; break;
                case 6: acc = acc * acc; break;
                case 7: acc = acc + (acc << 1); break;
            }
        } else {
            acc = (acc * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Irreducible control flow using computed goto */
        static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
        goto *labels[i % 5];
        
        L0:
            acc += barrier;
            barrier = acc;
            goto cont;
        L1:
            acc -= barrier;
            barrier = acc;
            goto cont;
        L2:
            acc ^= barrier;
            barrier = acc;
            goto cont;
        L3:
            acc |= barrier;
            barrier = acc;
            goto cont;
        L4:
            acc &= barrier;
            barrier = acc;
            goto cont;
        
        cont:
        /* Memory operations with pointer arithmetic */
        int *ptr = &data[i];
        for (j = 0; j < 3; j++) {
            ptr = &ptr[(i + j) % size];
            acc += *ptr;
        }
        
        result[i] = acc;
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Simple stride-1 loop for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * 3 + c[i] * 7;
    }
    
    /* Loop with carried dependency */
    for (i = 1; i < size; i++) {
        a[i] += a[i-1] * 2;
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Inline function calls from multiple sites */
        val = compute_hash(val, i);
        
        /* Artificial scheduling barrier */
        asm volatile("" : "+r" (val) : : "memory");
        
        val = data_transform(val, 0x00FF00FF);
        
        /* More inline calls */
        val = compute_hash(val, size - i);
        
        data[i] = val;
    }
}

/* Complex function with mixed operations */
__attribute__((hot))
void mixed_operations(int *arr, int n, int iterations) {
    int i, j, k;
    
    for (k = 0; k < iterations; k++) {
        /* Triple nested loop */
        for (i = 0; i < n; i++) {
            int sum = 0;
            for (j = 0; j < n; j++) {
                /* Memory access pattern with dependencies */
                int idx = (i * j) % n;
                sum += arr[idx] * (i + j + k);
                
                /* Conditional with side effects */
                if ((i ^ j) & 1) {
                    sum -= arr[(idx + 1) % n];
                } else {
                    sum += arr[(idx + n - 1) % n];
                }
                
                /* Function call that should be inlined */
                sum = compute_hash(sum, idx);
            }
            
            /* Do-while loop */
            int counter = i % 8;
            do {
                sum = (sum << 1) | (sum >> 31);
                sum ^= 0xAAAAAAAA;
                counter--;
            } while (counter > 0);
            
            arr[i] = sum;
        }
    }
}

/* Main orchestrator */
int main(int argc, char **argv) {
    const int sizes[] = {256, 512, 1024, 2048, 4096};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int i, j;
    long total_checksum = 0;
    clock_t start, end;
    
    printf("Starting Haifa scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (i = 0; i < 2; i++) {
        int warm_size = 128;
        int *warm_data = malloc(warm_size * sizeof(int));
        int *warm_result = malloc(warm_size * sizeof(int));
        
        for (j = 0; j < warm_size; j++) {
            warm_data[j] = j * 3 + 1;
        }
        
        complex_control_flow(warm_data, warm_size, warm_result);
        vectorizable_loop(warm_data, warm_result, warm_data, warm_size);
        
        free(warm_data);
        free(warm_result);
    }
    
    /* Main test with different dataset sizes */
    printf("Main test phase...\n");
    for (i = 0; i < num_sizes; i++) {
        int size = sizes[i];
        printf("Testing with size %d\n", size);
        
        int *data1 = malloc(size * sizeof(int));
        int *data2 = malloc(size * sizeof(int));
        int *data3 = malloc(size * sizeof(int));
        int *result = malloc(size * sizeof(int));
        
        /* Initialize with pattern */
        for (j = 0; j < size; j++) {
            data1[j] = j * 7 + 3;
            data2[j] = j * 11 + 5;
            data3[j] = j * 13 + 7;
        }
        
        start = clock();
        
        /* Execute all complex functions */
        complex_control_flow(data1, size, result);
        
        vectorizable_loop(data2, data3, result, size);
        
        parallel_region(data1, size);
        
        mixed_operations(data2, size > 100 ? 100 : size, 3);
        
        /* Compute checksum for verification */
        long checksum = 0;
        for (j = 0; j < size; j++) {
            checksum += data1[j] ^ data2[j] ^ data3[j] ^ result[j];
            checksum = (checksum << 13) | (checksum >> 51);
        }
        
        end = clock();
        
        total_checksum += checksum;
        printf("  Size %d: checksum = %ld, time = %.3f seconds\n", 
               size, checksum, (double)(end - start) / CLOCKS_PER_SEC);
        
        free(data1);
        free(data2);
        free(data3);
        free(result);
    }
    
    /* Additional stress with very complex control flow */
    printf("Additional stress test...\n");
    {
        int stress_size = 1024;
        int *stress_data = malloc(stress_size * sizeof(int));
        
        for (i = 0; i < stress_size; i++) {
            stress_data[i] = i;
        }
        
        /* Multiple passes with different transformations */
        for (int pass = 0; pass < 5; pass++) {
            int *temp = malloc(stress_size * sizeof(int));
            
            /* Switch between different algorithms */
            switch (pass % 3) {
                case 0:
                    complex_control_flow(stress_data, stress_size, temp);
                    break;
                case 1:
                    mixed_operations(stress_data, stress_size, 2);
                    memcpy(temp, stress_data, stress_size * sizeof(int));
                    break;
                case 2:
                    #pragma omp parallel
                    {
                        #pragma omp for nowait
                        for (i = 0; i < stress_size; i++) {
                            temp[i] = compute_hash(stress_data[i], i);
                        }
                        #pragma omp for
                        for (i = 0; i < stress_size; i++) {
                            temp[i] = data_transform(temp[i], 0x12345678);
                        }
                    }
                    break;
            }
            
            /* Barrier with inline asm */
            asm volatile("" ::: "memory");
            
            memcpy(stress_data, temp, stress_size * sizeof(int));
            free(temp);
        }
        
        /* Final checksum */
        long final_checksum = 0;
        for (i = 0; i < stress_size; i++) {
            final_checksum ^= stress_data[i];
            final_checksum = (final_checksum << 7) | (final_checksum >> 57);
        }
        total_checksum += final_checksum;
        
        free(stress_data);
    }
    
    printf("\nTotal checksum: %ld\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
