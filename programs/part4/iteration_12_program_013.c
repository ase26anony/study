/* haifa_scheduler_stress_test.c
 * A comprehensive test to stress GCC's Haifa scheduler and trigger
 * free_sched_context cleanup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Helper functions designed for inlining */
static inline unsigned int __attribute__((always_inline)) 
hash_mix(unsigned int a, unsigned int b, unsigned int c) {
    /* MurmurHash3 mixer */
    a ^= c; a -= (c << 14) | (c >> 18);
    b ^= a; b -= (a << 11) | (a >> 21);
    c ^= b; c -= (b << 25) | (b >> 7);
    a ^= c; a -= (c << 16) | (c >> 16);
    b ^= a; b -= (a << 4) | (a >> 28);
    c ^= b; c -= (b << 14) | (b >> 18);
    a ^= c; a -= (c << 24) | (c >> 8);
    return a + b + c;
}

static inline int __attribute__((always_inline))
complex_condition(int x, int y, int z) {
    /* Complex conditional with many dependencies */
    int t = (x * y) ^ (z << 3);
    t = (t & 0x55555555) + ((t >> 1) & 0x55555555);
    t = (t & 0x33333333) + ((t >> 2) & 0x33333333);
    t = (t & 0x0F0F0F0F) + ((t >> 4) & 0x0F0F0F0F);
    t = (t & 0x00FF00FF) + ((t >> 8) & 0x00FF00FF);
    t = (t & 0x0000FFFF) + ((t >> 16) & 0x0000FFFF);
    return t & 1;
}

/* Hot function with complex control flow */
__attribute__((hot))
unsigned long long process_data(int* data, int size, int iterations) {
    unsigned long long checksum = 0;
    int i, j, k;
    
    /* Outer loop with varying iteration counts */
    for (i = 0; i < iterations; i++) {
        /* Nested loops creating ILP opportunities */
        #pragma omp simd reduction(+:checksum)
        for (j = 0; j < size - 7; j += 8) {
            /* SIMD-friendly operations */
            int v0 = data[j] * data[j + 1];
            int v1 = data[j + 2] ^ data[j + 3];
            int v2 = data[j + 4] + data[j + 5];
            int v3 = data[j + 6] - data[j + 7];
            
            checksum += v0 + v1 + v2 + v3;
            
            /* Artificial scheduling barrier */
            asm volatile("" ::: "memory");
        }
        
        /* Complex switch statement with computed gotos */
        switch (i % 7) {
            case 0: {
                /* Deep if-else chain */
                if (data[0] > 100) {
                    for (k = 0; k < size; k++) {
                        data[k] = hash_mix(data[k], i, k);
                    }
                } else if (data[0] > 50) {
                    for (k = 0; k < size; k += 2) {
                        data[k] = (data[k] << 1) | (data[k] >> 31);
                    }
                } else {
                    for (k = 1; k < size; k += 2) {
                        data[k] = data[k] ^ 0xDEADBEEF;
                    }
                }
                break;
            }
            case 1:
            case 2: {
                /* Loop with carried dependency */
                int prev = data[size - 1];
                for (k = 0; k < size; k++) {
                    int curr = data[k];
                    data[k] = prev + curr * 3;
                    prev = curr;
                }
                break;
            }
            case 3: {
                /* Irreducible control flow using labels as values */
                static void* jump_table[] = {
                    &&label_a, &&label_b, &&label_c, 
                    &&label_d, &&label_e, &&label_f
                };
                
                int idx = data[0] % 6;
                goto *jump_table[idx];
                
                label_a:
                    data[0] *= 2;
                    goto end_switch;
                label_b:
                    data[0] += 0x12345678;
                    goto end_switch;
                label_c:
                    data[0] ^= 0x87654321;
                    goto end_switch;
                label_d:
                    data[0] = (data[0] << 5) | (data[0] >> 27);
                    goto end_switch;
                label_e:
                    data[0] = ~data[0];
                    goto end_switch;
                label_f:
                    data[0] = data[0] * 0x9E3779B9;
                    goto end_switch;
                end_switch:
                break;
            }
            case 4: {
                /* Memory-intensive operations */
                int* temp = malloc(size * sizeof(int));
                if (temp) {
                    for (k = 0; k < size; k++) {
                        temp[k] = data[(k + 1) % size];
                    }
                    for (k = 0; k < size; k++) {
                        data[k] = data[k] + temp[k] - (k % 256);
                    }
                    free(temp);
                }
                break;
            }
            case 5: {
                /* Mixed integer operations */
                for (k = 0; k < size; k++) {
                    data[k] = ((data[k] * 1103515245) + 12345) & 0x7FFFFFFF;
                    if (complex_condition(data[k], k, i)) {
                        data[k] = data[k] >> 4;
                    } else {
                        data[k] = data[k] << 4;
                    }
                }
                break;
            }
            case 6: {
                /* Do-while with early exit */
                k = 0;
                do {
                    data[k] = data[k] * 3 + 1;
                    while (data[k] > 1000000) {
                        data[k] /= 2;
                    }
                    k++;
                } while (k < size && data[k-1] % 7 != 0);
                break;
            }
        }
    }
    
    return checksum;
}

/* Another hot function with different patterns */
__attribute__((hot))
void transform_matrix(int** matrix, int rows, int cols) {
    int i, j;
    
    /* OpenMP parallel region - scheduler interacts with parallelization */
    #pragma omp parallel for private(i, j) collapse(2) schedule(dynamic)
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            /* Complex expression with many operations */
            int val = matrix[i][j];
            val = ((val & 0xFF) << 24) | 
                  ((val & 0xFF00) << 8) |
                  ((val & 0xFF0000) >> 8) |
                  ((val & 0xFF000000) >> 24);
            
            val = val * 0xCC9E2D51;
            val = (val << 15) | (val >> 17);
            val = val * 0x1B873593;
            
            /* Conditional store */
            if ((i + j) % 3 == 0) {
                matrix[i][j] = val ^ 0xFFFFFFFF;
            } else if ((i + j) % 3 == 1) {
                matrix[i][j] = val + i - j;
            } else {
                matrix[i][j] = val * 2 - (i * j);
            }
        }
    }
    
    /* Sequential section with pointer chasing */
    int* ptr = &matrix[0][0];
    for (i = 0; i < rows * cols - 1; i++) {
        int next_idx = (*ptr) % (rows * cols - 1);
        ptr = &matrix[0][0] + next_idx;
        *ptr = (*ptr + i) & 0xFF;
    }
}

/* Function with recursive pattern */
__attribute__((noinline))
unsigned long long recursive_pattern(int depth, int* arr, int size) {
    if (depth <= 0) {
        return arr[0];
    }
    
    unsigned long long sum = 0;
    for (int i = 0; i < size; i += 4) {
        /* Create instruction-level parallelism */
        int a = arr[i] * depth;
        int b = arr[i + 1] ^ depth;
        int c = arr[i + 2] + depth;
        int d = arr[i + 3] - depth;
        
        /* Memory barrier to force scheduling constraints */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d) :: "memory");
        
        sum += a + b + c + d;
        
        /* Recursive call with reduced depth */
        if ((i % 16) == 0) {
            arr[i] = recursive_pattern(depth - 1, arr, size / 2);
        }
    }
    
    return sum;
}

/* Main test driver */
int main() {
    const int NUM_TESTS = 5;
    const int BASE_SIZE = 1024;
    unsigned long long total_checksum = 0;
    
    printf("Starting Haifa scheduler stress test...\n");
    
    /* Warm-up phase */
    printf("Phase 1: Warm-up\n");
    for (int test = 0; test < 3; test++) {
        int size = BASE_SIZE >> test;
        int* data = malloc(size * sizeof(int));
        
        if (!data) continue;
        
        /* Initialize with pseudo-random data */
        for (int i = 0; i < size; i++) {
            data[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        }
        
        /* Call hot function */
        unsigned long long checksum = process_data(data, size, 10);
        total_checksum ^= checksum;
        
        free(data);
    }
    
    /* Main stress phase */
    printf("Phase 2: Main stress tests\n");
    for (int test = 0; test < NUM_TESTS; test++) {
        int size = BASE_SIZE * (1 << test);
        int* data = malloc(size * sizeof(int));
        
        if (!data) {
            printf("Failed to allocate %d elements\n", size);
            continue;
        }
        
        /* Different initialization patterns */
        for (int i = 0; i < size; i++) {
            switch (test % 4) {
                case 0: data[i] = i; break;
                case 1: data[i] = size - i; break;
                case 2: data[i] = i * i; break;
                case 3: data[i] = (i << 16) | (i >> 16); break;
            }
        }
        
        clock_t start = clock();
        
        /* Test 1: Complex processing */
        unsigned long long checksum1 = process_data(data, size, 5 + test);
        
        /* Test 2: Matrix transformation */
        int rows = 32 << (test % 3);
        int cols = 32 << (test % 2);
        int** matrix = malloc(rows * sizeof(int*));
        for (int i = 0; i < rows; i++) {
            matrix[i] = malloc(cols * sizeof(int));
            for (int j = 0; j < cols; j++) {
                matrix[i][j] = (i * cols + j) * 0x9E3779B9;
            }
        }
        
        transform_matrix(matrix, rows, cols);
        
        /* Test 3: Recursive pattern */
        unsigned long long checksum2 = recursive_pattern(3 + test, data, size);
        
        clock_t end = clock();
        
        /* Combine results */
        total_checksum ^= checksum1;
        total_checksum ^= checksum2;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                total_checksum += matrix[i][j];
            }
            free(matrix[i]);
        }
        free(matrix);
        
        printf("  Test %d (size=%d): %.3f ms, checksum=0x%016llX\n",
               test, size, 
               (double)(end - start) * 1000 / CLOCKS_PER_SEC,
               total_checksum);
        
        free(data);
    }
    
    /* Final verification phase */
    printf("Phase 3: Verification\n");
    {
        int final_size = 256;
        int* final_data = malloc(final_size * sizeof(int));
        
        for (int i = 0; i < final_size; i++) {
            final_data[i] = i * 0x5A827999;
        }
        
        /* Mix all patterns in one function call */
        unsigned long long final_checksum = 0;
        for (int i = 0; i < 100; i++) {
            final_checksum ^= process_data(final_data, final_size, 1);
            
            /* Every 10 iterations, transform as matrix */
            if (i % 10 == 0) {
                int** mini_matrix = malloc(16 * sizeof(int*));
                for (int r = 0; r < 16; r++) {
                    mini_matrix[r] = &final_data[r * 16];
                }
                transform_matrix(mini_matrix, 16, 16);
                free(mini_matrix);
            }
            
            /* Every 25 iterations, do recursive pattern */
            if (i % 25 == 0) {
                final_checksum += recursive_pattern(2, final_data, final_size);
            }
        }
        
        total_checksum ^= final_checksum;
        free(final_data);
    }
    
    printf("\nFinal checksum: 0x%016llX\n", total_checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
