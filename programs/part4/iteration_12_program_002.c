/* haifa_scheduler_stress_test.c
 * A comprehensive test to trigger GCC's Haifa scheduler context allocation
 * and cleanup, specifically targeting free_sched_context logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Always-inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise operations */
    int hash = (a * 0x9e3779b9) ^ (b * 0x85ebca6b) ^ (c * 0xc2b2ae35);
    hash = (hash >> 16) ^ (hash & 0xFFFF);
    hash = hash * 0x5bd1e995;
    hash = hash ^ (hash >> 15);
    return hash;
}

static inline int __attribute__((always_inline))
conditional_transform(int x, int selector) {
    /* Complex conditional chain creating multiple basic blocks */
    int result = x;
    
    switch (selector & 7) {
        case 0:
            result = (result << 3) | (result >> 29);
            result = result * 1103515245 + 12345;
            break;
        case 1:
            result = result ^ 0xAAAAAAAA;
            result = (result * 0xCCCCCCCD) >> 3;
            break;
        case 2:
            result = result + (result << 2);
            result = result ^ (result >> 7);
            break;
        case 3:
            result = ~result;
            result = result * 0x1B3;
            break;
        case 4:
            result = (result & 0x55555555) << 1 | (result & 0xAAAAAAAA) >> 1;
            result = result - 0x7FFFFFFF;
            break;
        case 5:
            result = result | 1;
            result = result * result;
            break;
        case 6:
            result = result + compute_hash(result, selector, 0xDEADBEEF);
            break;
        case 7:
            result = result ^ compute_hash(selector, result, 0xBEEFDEAD);
            result = result & 0x7FFFFFFF;
            break;
    }
    
    /* Additional if-else chain */
    if (result < 0) {
        result = -result;
        if (result > 1000) {
            result = result % 1000;
        } else if (result > 100) {
            result = result * 3;
        }
    } else if (result > 1000000) {
        result = result / 1000;
    } else if (result > 10000) {
        result = result >> 2;
    }
    
    return result;
}

/* Hot function with complex control flow */
__attribute__((hot))
void process_data_complex(int* data, int size, int iterations) {
    int i, j, k;
    
    /* Outer loop with varying iteration count */
    for (k = 0; k < iterations; k++) {
        /* First nested loop - candidate for vectorization */
        #pragma omp simd
        for (i = 0; i < size - 1; i++) {
            /* Memory operations creating dependencies */
            int temp = data[i] + data[i + 1];
            temp = temp * 0x9e3779b9;
            data[i] = conditional_transform(temp, i);
        }
        
        /* Second loop with different stride */
        for (i = size - 1; i > 0; i--) {
            /* Complex dependency chain */
            int val = data[i];
            for (j = 0; j < 4; j++) {
                val = compute_hash(val, j, data[i - 1]);
                /* Artificial scheduling barrier */
                asm volatile("" : "+r" (val) : : "memory");
            }
            data[i] = val;
        }
        
        /* Loop with computed goto for irreducible control flow */
        int index = 0;
        while (index < size) {
            static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4 };
            int selector = data[index] % 5;
            
            /* Computed goto creating complex CFG */
            goto *labels[selector];
            
            L0:
                data[index] = data[index] * 2 + 1;
                index++;
                continue;
            L1:
                data[index] = data[index] ^ 0xFF;
                index += 2;
                continue;
            L2:
                data[index] = ~data[index];
                index += 3;
                continue;
            L3:
                data[index] = data[index] + compute_hash(index, k, 0x12345678);
                index += 1;
                continue;
            L4:
                data[index] = conditional_transform(data[index], k);
                index += (k % 3) + 1;
                continue;
        }
    }
}

/* Function with deeply nested conditionals */
__attribute__((hot))
int nested_decision_tree(int x, int depth) {
    if (depth <= 0) return x;
    
    /* Deeply nested if-else chain */
    if (x & 1) {
        if (x & 2) {
            if (x & 4) {
                if (x & 8) {
                    x = x * 3 + 1;
                } else {
                    x = x ^ 0x5555;
                }
            } else {
                if (x & 16) {
                    x = x >> 1;
                } else {
                    x = x << 1;
                }
            }
        } else {
            if (x & 32) {
                x = x + compute_hash(x, depth, 0xABCD);
            } else {
                x = x - conditional_transform(x, depth);
            }
        }
    } else {
        switch (x % 6) {
            case 0:
                x = nested_decision_tree(x / 2, depth - 1);
                break;
            case 1:
                x = nested_decision_tree(x * 3, depth - 1);
                break;
            case 2:
                x = nested_decision_tree(x ^ 0xAAAA, depth - 1);
                break;
            case 3:
                x = nested_decision_tree(x & 0xFFFF, depth - 1);
                break;
            case 4:
                x = nested_decision_tree(x | 0x1111, depth - 1);
                break;
            case 5:
                x = nested_decision_tree(~x, depth - 1);
                break;
        }
    }
    
    /* Recursive call with different parameters */
    return nested_decision_tree(x, depth - 1);
}

/* Function with mixed operations and memory access patterns */
void mixed_operations(int* arr1, int* arr2, int* result, int n) {
    int i, j;
    
    /* Triple nested loop with complex indexing */
    for (i = 0; i < n; i++) {
        int sum = 0;
        for (j = 0; j < n; j++) {
            /* Memory operations with pointer arithmetic */
            int* ptr1 = arr1 + (i * n + j) % (n * n);
            int* ptr2 = arr2 + (j * n + i) % (n * n);
            
            /* Mix of operations */
            int val = *ptr1 * *ptr2;
            val = val + compute_hash(i, j, val);
            val = conditional_transform(val, i ^ j);
            
            /* Volatile asm to prevent optimization and create scheduling barriers */
            asm volatile("" : "+r" (val) : : "memory");
            
            sum += val;
        }
        
        /* Store with dependency on previous computation */
        result[i] = sum;
        
        /* Additional operation on result */
        if (i > 0) {
            result[i] = result[i] ^ result[i - 1];
            result[i] = conditional_transform(result[i], i);
        }
    }
    
    /* Post-processing loop with OpenMP parallelization */
    #pragma omp parallel for schedule(dynamic)
    for (i = 0; i < n; i++) {
        int temp = result[i];
        for (j = 0; j < 100; j++) {
            temp = compute_hash(temp, j, 0xDEADBEEF);
            /* Create artificial RAW dependencies */
            asm volatile("" : "+r" (temp) : : "memory");
        }
        result[i] = temp;
    }
}

/* Main orchestrator function */
int main() {
    const int sizes[] = {64, 128, 256, 512, 1024};
    const int iterations[] = {10, 20, 30, 40, 50};
    int num_tests = sizeof(sizes) / sizeof(sizes[0]);
    
    long long total_checksum = 0;
    clock_t total_time = 0;
    
    printf("Starting Haifa scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    printf("Warm-up phase...\n");
    for (int warmup = 0; warmup < 3; warmup++) {
        int size = sizes[warmup % num_tests];
        int* data = (int*)malloc(size * sizeof(int));
        
        for (int i = 0; i < size; i++) {
            data[i] = i * 0x9e3779b9 + warmup;
        }
        
        process_data_complex(data, size, 5);
        
        int checksum = 0;
        for (int i = 0; i < size; i++) {
            checksum ^= data[i];
        }
        
        free(data);
    }
    
    /* Main test phase */
    printf("Main test phase...\n");
    for (int test = 0; test < num_tests; test++) {
        int size = sizes[test];
        int iter = iterations[test % (sizeof(iterations)/sizeof(iterations[0]))];
        
        printf("Test %d: size=%d, iterations=%d\n", test+1, size, iter);
        
        /* Allocate and initialize test data */
        int* data1 = (int*)malloc(size * size * sizeof(int));
        int* data2 = (int*)malloc(size * size * sizeof(int));
        int* result = (int*)malloc(size * sizeof(int));
        
        for (int i = 0; i < size * size; i++) {
            data1[i] = compute_hash(i, test, 0x12345678);
            data2[i] = conditional_transform(i, test);
        }
        
        clock_t start = clock();
        
        /* Execute complex functions */
        process_data_complex(data1, size * size, iter / 2);
        
        for (int i = 0; i < size; i++) {
            result[i] = nested_decision_tree(data1[i], 8);
        }
        
        mixed_operations(data1, data2, result, size);
        
        clock_t end = clock();
        total_time += (end - start);
        
        /* Compute final checksum for verification */
        long long checksum = 0;
        for (int i = 0; i < size; i++) {
            checksum += result[i];
            checksum = (checksum << 13) | (checksum >> 51);
        }
        
        total_checksum ^= checksum;
        
        printf("  Checksum: 0x%016llX, Time: %.3f ms\n", 
               checksum, (double)(end - start) * 1000.0 / CLOCKS_PER_SEC);
        
        free(data1);
        free(data2);
        free(result);
    }
    
    printf("\nTotal checksum: 0x%016llX\n", total_checksum);
    printf("Total execution time: %.3f seconds\n", 
           (double)total_time / CLOCKS_PER_SEC);
    
    /* Final verification */
    if (total_checksum != 0) {
        printf("Test completed successfully.\n");
        return 0;
    } else {
        printf("ERROR: Zero checksum detected!\n");
        return 1;
    }
}
