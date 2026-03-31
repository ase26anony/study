/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000
#define MAX_DEPTH 8

/* Helper functions marked for inlining */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise ops */
    return ((a << 3) ^ (b >> 2)) + (c * 0x5bd1e995);
}

static inline int __attribute__((always_inline))
conditional_transform(int x, int selector) {
    /* Complex conditional chain */
    switch (selector & 7) {
        case 0: return x + (x << 2);
        case 1: return x ^ 0xDEADBEEF;
        case 2: return x * 3 + 1;
        case 3: return (x >> 4) | (x << 28);
        case 4: return x + compute_hash(x, selector, x >> 8);
        case 5: return x - (x / 3);
        case 6: return x & 0x55555555;
        case 7: return x | 0xAAAAAAAA;
        default: return x; /* Never reached */
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void process_array_complex(int* restrict src, int* restrict dst, int size, int depth) {
    int i, j, k;
    
    /* Outer loop with varying iteration count */
    for (i = 0; i < size; i += (depth + 1)) {
        /* Nested loop with carried dependency */
        int acc = src[i];
        for (j = 0; j < depth; j++) {
            /* Mix of memory ops and computation */
            acc = conditional_transform(acc, i + j);
            
            /* Inner loop with stride-1 access (vectorization candidate) */
            for (k = 0; k < 4; k++) {
                /* Artificial scheduling barrier */
                asm volatile("" : "+r"(acc) : : "memory");
                acc += src[(i + k) % size] * (j + 1);
            }
        }
        
        /* Store result with pointer arithmetic */
        *(dst + i) = acc;
        
        /* Another scheduling barrier */
        asm volatile("" : : : "memory");
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
int irreducible_cfg(int x, int y) {
    static void* labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int result = x;
    int counter = 0;
    
    /* Deeply nested if-else chain */
    if (x > 0) {
        if (y < 0) {
            result = x * y;
        } else if (y == 0) {
            result = x >> 2;
        } else {
            result = x + y;
        }
    } else {
        if (y > 100) {
            result = y - x;
        } else {
            result = x ^ y;
        }
    }
    
    /* Loop with computed goto */
    int idx = result % 6;
    goto *labels[idx];
    
L0:
    result += compute_hash(x, y, 1);
    if (counter++ < 10) goto *labels[(result + 1) % 6];
    /* Fall through */
    
L1:
    result ^= 0x12345678;
    if (counter++ < 10) goto *labels[(result + 2) % 6];
    
L2:
    result *= 3;
    if (counter++ < 10) goto *labels[(result + 3) % 6];
    
L3:
    result = (result << 4) | (result >> 28);
    if (counter++ < 10) goto *labels[(result + 4) % 6];
    
L4:
    result -= y;
    if (counter++ < 10) goto *labels[(result + 5) % 6];
    
L5:
    result = conditional_transform(result, x);
    if (counter++ < 10) goto *labels[result % 6];
    
    return result;
}

/* Vectorization candidate with OpenMP */
__attribute__((hot))
void vectorizable_loop(int* a, int* b, int* c, int n) {
    int i;
    
    #pragma omp simd safelen(16)
    for (i = 0; i < n; i++) {
        /* Independent operations with stride-1 access */
        int temp = a[i] * 3 + b[i];
        temp = (temp << 2) | (temp >> 30);
        c[i] = temp ^ compute_hash(a[i], b[i], i);
    }
    
    /* Additional loop with carried dependency */
    for (i = 1; i < n; i++) {
        c[i] += c[i-1] * 0x9e3779b9;
    }
}

/* Function with mixed instruction types */
__attribute__((hot))
int mixed_operations(int* arr, int size) {
    int sum = 0;
    int prod = 1;
    int i, j;
    
    /* Triple nested loop */
    for (i = 0; i < size; i++) {
        int val = arr[i];
        
        /* Function call from multiple sites */
        val = conditional_transform(val, i);
        
        for (j = 0; j < 4; j++) {
            /* Memory operations with addressing modes */
            int idx = (i * 4 + j) % size;
            val += arr[idx] * (j + 1);
            
            /* Bitwise operations */
            val = (val & 0xFF00FF) | ((val & 0x00FF00) << 8);
        }
        
        /* Conditional update */
        if (val > 0) {
            sum += val;
            prod *= (val & 0xFF) + 1;
        } else {
            sum -= val;
            prod /= 2;
        }
        
        /* Another scheduling barrier */
        asm volatile("" : "+r"(sum), "+r"(prod) : : "memory");
    }
    
    return sum ^ prod;
}

/* Main orchestrator */
int main() {
    int i, iter;
    int checksum = 0;
    clock_t start, end;
    
    /* Allocate arrays with different alignments */
    int* array1 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* array2 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* array3 = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    int* temp = (int*)aligned_alloc(64, ARRAY_SIZE * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !temp) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    srand(42);
    for (i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand() % 1000;
        array2[i] = rand() % 1000;
        array3[i] = rand() % 1000;
    }
    
    printf("Starting scheduler stress test...\n");
    start = clock();
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (iter = 0; iter < ITERATIONS / 10; iter++) {
        process_array_complex(array1, temp, ARRAY_SIZE, iter % MAX_DEPTH);
        checksum ^= temp[iter % ARRAY_SIZE];
    }
    
    /* Main test phase with different patterns */
    printf("Main test phase...\n");
    for (iter = 0; iter < ITERATIONS; iter++) {
        int depth = iter % MAX_DEPTH;
        
        /* Alternate between different functions */
        switch (iter % 4) {
            case 0:
                process_array_complex(array1, temp, ARRAY_SIZE, depth);
                checksum ^= mixed_operations(temp, ARRAY_SIZE / 2);
                break;
                
            case 1:
                vectorizable_loop(array1, array2, temp, ARRAY_SIZE);
                checksum ^= irreducible_cfg(temp[0], temp[ARRAY_SIZE-1]);
                break;
                
            case 2:
                #pragma omp parallel for
                for (i = 0; i < ARRAY_SIZE; i += 64) {
                    int end = (i + 64 < ARRAY_SIZE) ? i + 64 : ARRAY_SIZE;
                    for (int j = i; j < end; j++) {
                        temp[j] = conditional_transform(array1[j], array2[j]);
                    }
                }
                checksum ^= mixed_operations(temp, ARRAY_SIZE);
                break;
                
            case 3:
                /* Complex nested loops */
                for (i = 0; i < ARRAY_SIZE; i++) {
                    int val = array1[i];
                    for (int j = 0; j < depth; j++) {
                        val = compute_hash(val, array2[(i+j)%ARRAY_SIZE], j);
                        asm volatile("" : "+r"(val) : : "memory");
                    }
                    temp[i] = val;
                }
                checksum ^= irreducible_cfg(temp[0], temp[ARRAY_SIZE/2]);
                break;
        }
        
        /* Rotate arrays to create varying data patterns */
        if (iter % 100 == 0) {
            int* swap = array1;
            array1 = array2;
            array2 = array3;
            array3 = swap;
        }
    }
    
    end = clock();
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("Execution time: %.2f seconds\n", 
           (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(temp);
    
    return 0;
}
