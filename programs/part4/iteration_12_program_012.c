/* Test program to trigger free_sched_context in GCC's Haifa scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Always inline helper functions to create complex dataflow */
static inline int __attribute__((always_inline)) 
compute_hash(int x, int y) {
    return (x * 31 + y) ^ ((x << 5) | (y >> 3));
}

static inline int __attribute__((always_inline))
bit_reverse(int x) {
    x = ((x >> 1) & 0x55555555) | ((x & 0x55555555) << 1);
    x = ((x >> 2) & 0x33333333) | ((x & 0x33333333) << 2);
    x = ((x >> 4) & 0x0F0F0F0F) | ((x & 0x0F0F0F0F) << 4);
    x = ((x >> 8) & 0x00FF00FF) | ((x & 0x00FF00FF) << 8);
    return (x >> 16) | (x << 16);
}

/* Hot function with complex control flow */
__attribute__((hot))
void process_data_complex(int *data, int size, int threshold) {
    int i, j, k;
    volatile int barrier = 0; /* Force memory dependencies */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        /* Deep if-else chain */
        if (data[i] < threshold) {
            data[i] = compute_hash(data[i], i);
            /* Memory operation creating dependencies */
            barrier = data[i];
        } else if (data[i] < threshold * 2) {
            data[i] = bit_reverse(data[i]);
            /* Inline asm to create scheduling barriers */
            asm volatile("" : : "r"(data[i]) : "memory");
        } else if (data[i] < threshold * 3) {
            /* Switch statement with multiple cases */
            switch (data[i] % 7) {
                case 0: data[i] = data[i] * 2; break;
                case 1: data[i] = data[i] / 3; break;
                case 2: data[i] = data[i] ^ 0xAAAAAAAA; break;
                case 3: data[i] = data[i] | 0x55555555; break;
                case 4: data[i] = data[i] & 0x33333333; break;
                case 5: data[i] = data[i] << (i % 8); break;
                case 6: data[i] = data[i] >> (i % 8); break;
            }
        } else {
            /* Complex computation with multiple dependencies */
            int temp = data[i];
            for (j = 0; j < 4; j++) {
                temp = compute_hash(temp, j);
                temp = bit_reverse(temp);
            }
            data[i] = temp;
        }
        
        /* Inner loop with carried dependency */
        for (k = 0; k < (i % 8); k++) {
            data[i] = compute_hash(data[i], k);
        }
    }
}

/* Function with irreducible control flow using computed goto */
__attribute__((noinline))
void irreducible_cfg(int *data, int size) {
    static void *labels[] = { &&L0, &&L1, &&L2, &&L3, &&L4, &&L5 };
    int i = 0;
    
    L0:
        data[i] += 1;
        goto *labels[(i++) % 6];
    L1:
        data[i] *= 3;
        goto *labels[(i++) % 6];
    L2:
        data[i] ^= 0xFF;
        goto *labels[(i++) % 6];
    L3:
        data[i] |= 0xAA;
        goto *labels[(i++) % 6];
    L4:
        data[i] &= 0x55;
        goto *labels[(i++) % 6];
    L5:
        if (i < size) goto L0;
}

/* Vectorization candidate with OpenMP pragmas */
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Loop with stride-1 array operations */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * c[i] + compute_hash(i, b[i]);
    }
    
    /* Another loop with memory dependencies */
    #pragma omp parallel for
    for (i = 1; i < size; i++) {
        a[i] += a[i-1]; /* Create dependency */
        asm volatile("" : : "r"(a[i]) : "memory");
    }
}

/* Function with mixed operations and deep nesting */
__attribute__((hot))
void mixed_operations(int *arr, int n) {
    int i, j;
    
    for (i = 0; i < n; i++) {
        /* Multiple independent operations to create ILP */
        int x = arr[i];
        int y = compute_hash(x, i);
        int z = bit_reverse(y);
        int w = x * y + z;
        
        /* Complex conditional with side effects */
        if (w % 2 == 0) {
            for (j = 0; j < 3; j++) {
                w = compute_hash(w, j);
                /* Memory clobber to force scheduling constraints */
                asm volatile("" : : "r"(w) : "memory");
            }
        } else {
            do {
                w = bit_reverse(w);
                j++;
            } while (j < 2);
        }
        
        arr[i] = w;
        
        /* While loop with break/continue */
        j = 0;
        while (j < 5) {
            if (arr[i] % (j + 2) == 0) {
                arr[i] /= 2;
                continue;
            }
            arr[i] += j;
            j++;
            if (arr[i] > 1000000) break;
        }
    }
}

/* Main orchestrator */
int main() {
    const int sizes[] = {128, 256, 512, 1024, 2048};
    const int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    int checksum = 0;
    int iter;
    
    /* Warm-up loop to trigger optimization heuristics */
    printf("Starting warm-up...\n");
    for (iter = 0; iter < 3; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s];
            int *data1 = malloc(size * sizeof(int));
            int *data2 = malloc(size * sizeof(int));
            int *data3 = malloc(size * sizeof(int));
            
            /* Initialize with pattern */
            for (int i = 0; i < size; i++) {
                data1[i] = (i * 17 + 13) % 1000;
                data2[i] = (i * 23 + 7) % 1000;
                data3[i] = (i * 31 + 11) % 1000;
            }
            
            /* Call complex functions */
            process_data_complex(data1, size, 500);
            irreducible_cfg(data2, size);
            vectorizable_loop(data3, data1, data2, size);
            mixed_operations(data1, size);
            
            /* Compute checksum */
            for (int i = 0; i < size; i++) {
                checksum ^= data1[i] + data2[i] * 3 - data3[i];
            }
            
            free(data1);
            free(data2);
            free(data3);
        }
    }
    
    /* Timed section with different patterns */
    printf("Starting timed execution...\n");
    clock_t start = clock();
    
    for (iter = 0; iter < 10; iter++) {
        for (int s = 0; s < num_sizes; s++) {
            int size = sizes[s];
            int *arrays[5];
            
            /* Allocate and initialize multiple arrays */
            for (int a = 0; a < 5; a++) {
                arrays[a] = malloc(size * sizeof(int));
                for (int i = 0; i < size; i++) {
                    arrays[a][i] = (i * (a + 1) * 19 + iter) % 2000;
                }
            }
            
            /* Interleave different operations */
            #pragma omp parallel sections
            {
                #pragma omp section
                process_data_complex(arrays[0], size, 300 + iter * 50);
                
                #pragma omp section
                irreducible_cfg(arrays[1], size);
                
                #pragma omp section
                {
                    vectorizable_loop(arrays[2], arrays[0], arrays[1], size);
                    mixed_operations(arrays[2], size);
                }
            }
            
            /* More complex nested operations */
            for (int i = 0; i < size; i += 16) {
                int block_size = (size - i) < 16 ? (size - i) : 16;
                process_data_complex(&arrays[3][i], block_size, 400);
                irreducible_cfg(&arrays[4][i], block_size);
            }
            
            /* Final checksum computation */
            for (int a = 0; a < 5; a++) {
                for (int i = 0; i < size; i++) {
                    checksum = compute_hash(checksum, arrays[a][i]);
                    checksum = bit_reverse(checksum);
                }
                free(arrays[a]);
            }
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Final checksum: %d\n", checksum);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    return 0;
}
