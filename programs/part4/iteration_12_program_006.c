/* Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-generate -o scheduler_test scheduler_test.c
 * Run: ./scheduler_test
 * Recompile: gcc -O3 -fschedule-insns -fschedule-insns2 -finline-functions -funswitch-loops -fprofile-use -o scheduler_test_opt scheduler_test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000
#define WARMUP_ITERATIONS 1000

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int a, int b, int c) {
    /* Mix of arithmetic and bitwise ops for ILP */
    int t = a ^ (b << 3);
    t = t + (c * 17);
    t = (t >> 5) | (t << 27);
    t = t * 0x9e3779b9;
    return t;
}

static inline int __attribute__((always_inline))
process_element(int x, int y, int selector) {
    /* Complex conditional chain */
    int result;
    switch (selector & 7) {
        case 0:
            result = x + y;
            asm volatile("" : "+r"(result) : : "memory"); /* Scheduling barrier */
            break;
        case 1:
            result = x - y;
            asm volatile("" : "+r"(result) : : "memory");
            break;
        case 2:
            result = x * y;
            asm volatile("" : "+r"(result) : : "memory");
            break;
        case 3:
            result = x ^ y;
            asm volatile("" : "+r"(result) : : "memory");
            break;
        case 4:
            result = (x << 3) | (y >> 2);
            asm volatile("" : "+r"(result) : : "memory");
            break;
        case 5:
            result = (x & 0xFF) + (y & 0xFF00);
            asm volatile("" : "+r"(result) : : "memory");
            break;
        case 6:
            result = x % (y | 1);
            asm volatile("" : "+r"(result) : : "memory");
            break;
        default:
            result = ~(x + y);
            asm volatile("" : "+r"(result) : : "memory");
            break;
    }
    return result;
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *input, int *output, int size, int seed) {
    int i, j, k;
    volatile int state = seed; /* Prevent optimization */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int acc = input[i];
        
        /* Inner loop with carried dependency */
        for (j = 0; j < (i & 15); j++) {
            acc = compute_hash(acc, j, state);
            
            /* Deep if-else chain inside loop */
            if (acc & 1) {
                if (acc & 2) {
                    acc = process_element(acc, input[(i + j) % size], 0);
                } else if (acc & 4) {
                    acc = process_element(acc, state, 1);
                } else {
                    acc = process_element(acc, j, 2);
                }
            } else {
                if (acc & 8) {
                    acc = process_element(acc, i, 3);
                } else {
                    acc = process_element(acc, size, 4);
                }
            }
            
            /* Small switch inside inner loop */
            switch (acc & 3) {
                case 0: acc += 1; break;
                case 1: acc *= 2; break;
                case 2: acc ^= 0x55; break;
                case 3: acc = (acc >> 1) | (acc << 31); break;
            }
        }
        
        /* Another loop with pointer arithmetic */
        int *ptr = &input[i];
        for (k = 0; k < 8; k++) {
            if (k & 1) {
                acc += *ptr;
                ptr = &input[(ptr - input + 1) % size];
            } else {
                acc -= *ptr;
                ptr = &input[(ptr - input - 1 + size) % size];
            }
        }
        
        output[i] = acc;
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((hot))
void irreducible_cfg(int *data, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int i = 0;
    int state = 0;
    
    L0:
        data[i] = compute_hash(data[i], state, i);
        state = (state + 1) % 6;
        goto *labels[state];
    
    L1:
        if (i < size - 1) {
            data[i] += data[i + 1];
            i++;
            state = (state * 3) % 6;
            goto *labels[state];
        }
        goto L2;
    
    L2:
        data[i] = process_element(data[i], size, state);
        if (i > 0) {
            i--;
            state = (state + 2) % 6;
            goto *labels[state];
        }
        goto L3;
    
    L3:
        for (int j = 0; j < 4; j++) {
            data[i] ^= (data[i] << j);
        }
        state = (state + 5) % 6;
        if (i < size / 2) goto *labels[state];
        goto L4;
    
    L4:
        data[i] = data[i] * 0x9e3779b9;
        if (data[i] & 1) {
            state = 0;
            goto *labels[state];
        }
        goto L5;
    
    L5:
        if (++i < size) {
            state = (data[i] & 5) % 6;
            goto *labels[state];
        }
}

/* Vectorization candidate with OpenMP pragmas */
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    #pragma omp simd safelen(16)
    for (i = 0; i < size; i++) {
        /* Simple stride-1 operations for vectorization */
        int t = a[i] + b[i];
        t = t * 3 - 7;
        t = t ^ (t >> 3);
        c[i] = t;
    }
    
    /* Another loop with carried dependency */
    for (i = 1; i < size; i++) {
        c[i] = c[i] + c[i - 1] * 2;
    }
}

/* Function with mixed operations and memory access patterns */
__attribute__((hot))
void mixed_operations(int *arr, int size) {
    int i, j;
    
    /* Do-while loop */
    i = 0;
    do {
        int temp = arr[i];
        
        /* While loop inside do-while */
        j = 0;
        while (j < 8) {
            temp = compute_hash(temp, j, i);
            j++;
        }
        
        /* Memory operations with different patterns */
        arr[(i * 17) % size] = temp;
        arr[(i * 31) % size] = process_element(temp, arr[i], i & 7);
        
        i = (i + 1) % size;
    } while (i != 0);
}

/* Main orchestrator */
int main() {
    int *array1 = malloc(ARRAY_SIZE * sizeof(int));
    int *array2 = malloc(ARRAY_SIZE * sizeof(int));
    int *array3 = malloc(ARRAY_SIZE * sizeof(int));
    int *array4 = malloc(ARRAY_SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = rand();
        array2[i] = rand();
        array3[i] = i;
        array4[i] = 0;
    }
    
    printf("Starting scheduler stress test...\n");
    
    /* Warm-up phase to trigger optimization heuristics */
    clock_t start_warmup = clock();
    for (int iter = 0; iter < WARMUP_ITERATIONS; iter++) {
        complex_control_flow(array1, array4, ARRAY_SIZE / 4, iter);
        irreducible_cfg(array2, ARRAY_SIZE / 8);
    }
    clock_t end_warmup = clock();
    printf("Warm-up completed in %.2f seconds\n", 
           (double)(end_warmup - start_warmup) / CLOCKS_PER_SEC);
    
    /* Main test phase with timing */
    clock_t start_main = clock();
    long long checksum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Alternate between different functions to stress different scheduling paths */
        switch (iter & 3) {
            case 0:
                complex_control_flow(array1, array4, ARRAY_SIZE, iter);
                break;
            case 1:
                irreducible_cfg(array3, ARRAY_SIZE);
                break;
            case 2:
                vectorizable_loop(array1, array2, array4, ARRAY_SIZE);
                break;
            case 3:
                mixed_operations(array4, ARRAY_SIZE);
                break;
        }
        
        /* Compute checksum for verification */
        for (int i = 0; i < ARRAY_SIZE; i += 64) {
            checksum += array4[i];
        }
        
        /* Vary array sizes to trigger different optimization decisions */
        if (iter % 100 == 0) {
            complex_control_flow(array1, array4, ARRAY_SIZE / 2, iter);
            complex_control_flow(array1, array4, ARRAY_SIZE / 8, iter);
        }
    }
    
    clock_t end_main = clock();
    
    printf("Main test completed in %.2f seconds\n", 
           (double)(end_main - start_main) / CLOCKS_PER_SEC);
    printf("Final checksum: %lld\n", checksum);
    
    /* Verify computation */
    int verify = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        verify ^= array4[i];
    }
    printf("Verification XOR: %d\n", verify);
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
