/* haifa_scheduler_stress_test.c
 * A comprehensive test to stress GCC's Haifa scheduler and trigger
 * free_sched_context cleanup logic.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

/* Helper functions marked for inlining */
static inline unsigned int __attribute__((always_inline)) 
compute_hash(unsigned int x) {
    /* Complex bitwise operations to create ILP opportunities */
    x = (x ^ (x >> 16)) * 0x45d9f3b;
    x = (x ^ (x >> 16)) * 0x45d9f3b;
    x = x ^ (x >> 16);
    return x;
}

static inline int __attribute__((always_inline))
branch_heavy_transform(int x, int y) {
    /* Deeply nested if-else chain */
    if (x < 0) {
        if (y < 0) return x * y;
        else if (y == 0) return x + 1;
        else return x - y;
    } else if (x == 0) {
        if (y < 0) return y * 2;
        else if (y == 0) return 1;
        else return y * 3;
    } else {
        if (y < 0) return x + y * 2;
        else if (y == 0) return x - 1;
        else {
            if (x > y) return x / (y + 1);
            else return y / (x + 1);
        }
    }
}

/* Hot function with complex control flow */
__attribute__((hot))
void complex_control_flow(int *data, int size, int *result) {
    int i, j, k;
    volatile int barrier = 0; /* Scheduling barrier */
    
    /* Nested loops with varying iteration counts */
    for (i = 0; i < size; i++) {
        int temp = data[i];
        
        /* Switch statement with multiple cases */
        switch (i % 7) {
            case 0:
                temp = compute_hash(temp);
                /* Artificial asm barrier */
                asm volatile("" ::: "memory");
                break;
            case 1:
                temp = temp * 3 + 1;
                while (temp > 1) {
                    if (temp % 2 == 0) temp /= 2;
                    else temp = temp * 3 + 1;
                }
                break;
            case 2:
                for (j = 0; j < (i % 5) + 1; j++) {
                    temp = branch_heavy_transform(temp, j);
                }
                break;
            case 3:
                /* Memory operations with pointer arithmetic */
                for (k = 0; k < 4; k++) {
                    int *ptr = &data[(i + k) % size];
                    temp ^= *ptr;
                    asm volatile("" : "+r" (temp) : : "memory");
                }
                break;
            case 4:
                /* Computed goto for irreducible CFG */
                {
                    static void *labels[] = { &&L0, &&L1, &&L2, &&L3 };
                    goto *labels[i % 4];
                L0:
                    temp += 1;
                    goto end_switch;
                L1:
                    temp *= 2;
                    goto end_switch;
                L2:
                    temp = temp >> 1;
                    goto end_switch;
                L3:
                    temp = ~temp;
                    goto end_switch;
                end_switch:;
                }
                break;
            case 5:
                /* Do-while loop */
                j = 0;
                do {
                    temp = (temp << 1) | (temp >> 31);
                    j++;
                } while (j < 3);
                break;
            case 6:
                /* Mixed operations */
                temp = ((temp & 0xAAAAAAAA) >> 1) | ((temp & 0x55555555) << 1);
                temp = compute_hash(temp);
                barrier = temp; /* Force memory dependency */
                temp = barrier + i;
                break;
        }
        
        result[i] = temp;
    }
}

/* Function with vectorization candidate */
__attribute__((hot))
void vectorizable_loop(int *a, int *b, int *c, int size) {
    int i;
    
    /* Simple stride-1 loop for auto-vectorization */
    #pragma omp simd
    for (i = 0; i < size; i++) {
        a[i] = b[i] * c[i] + i;
    }
    
    /* Second loop with carried dependency */
    for (i = 1; i < size; i++) {
        a[i] = a[i] + a[i-1] * 0.5;
    }
}

/* Function with OpenMP parallelization */
void parallel_region(int *data, int size) {
    int i;
    
    #pragma omp parallel for schedule(dynamic, 16)
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Complex operation inside parallel region */
        val = compute_hash(val);
        val = branch_heavy_transform(val, i);
        
        /* Memory barrier */
        #pragma omp atomic
        data[i] ^= val;
    }
}

/* Irreducible control flow using computed gotos */
__attribute__((noinline))
int irreducible_cfg(int x, int iterations) {
    void *labels[] = { &&start, &&loop, &&branch1, &&branch2, &&end };
    int counter = 0;
    
    goto start;
    
start:
    if (iterations-- <= 0) goto end;
    
loop:
    /* Mix of operations */
    x = (x * 1103515245 + 12345) & 0x7fffffff;
    counter++;
    
    /* Computed goto based on x */
    goto *labels[x % 5];
    
branch1:
    x = x ^ (x >> 16);
    goto loop;
    
branch2:
    x = x * 0x5bd1e995;
    goto loop;
    
end:
    return x + counter;
}

/* Main stress test function */
__attribute__((hot))
unsigned long long stress_test(int size) {
    int *data1 = malloc(size * sizeof(int));
    int *data2 = malloc(size * sizeof(int));
    int *data3 = malloc(size * sizeof(int));
    int *result = malloc(size * sizeof(int));
    unsigned long long checksum = 0;
    int i, j;
    
    if (!data1 || !data2 || !data3 || !result) {
        free(data1); free(data2); free(data3); free(result);
        return 0;
    }
    
    /* Initialize data */
    srand(42);
    for (i = 0; i < size; i++) {
        data1[i] = rand();
        data2[i] = rand();
        data3[i] = rand();
    }
    
    /* Warm-up phase - trigger optimization heuristics */
    for (j = 0; j < 3; j++) {
        complex_control_flow(data1, size > 1000 ? 1000 : size, result);
        vectorizable_loop(data2, data3, result, size > 500 ? 500 : size);
    }
    
    /* Main test phase */
    for (j = 0; j < 5; j++) {
        /* Complex control flow */
        complex_control_flow(data1, size, result);
        
        /* Vectorization attempts */
        vectorizable_loop(data2, data3, result, size);
        
        /* Parallel region */
        parallel_region(data3, size);
        
        /* Irreducible CFG */
        for (i = 0; i < size; i += 100) {
            result[i % size] = irreducible_cfg(data1[i % size], 10);
        }
        
        /* Update checksum */
        for (i = 0; i < size; i++) {
            checksum += result[i] + data1[i] + data2[i] + data3[i];
            checksum = (checksum << 13) | (checksum >> 51); /* Rotate */
        }
    }
    
    free(data1);
    free(data2);
    free(data3);
    free(result);
    
    return checksum;
}

/* Multiple entry points with different characteristics */
__attribute__((hot))
void nested_loop_test(int depth) {
    int i, j, k, l;
    volatile int sink = 0;
    
    for (i = 0; i < depth; i++) {
        for (j = 0; j < i + 1; j++) {
            for (k = 0; k < j + 1; k++) {
                for (l = 0; l < k + 1; l++) {
                    /* Mix of operations */
                    int val = i ^ j ^ k ^ l;
                    val = compute_hash(val);
                    val = branch_heavy_transform(val, i + j + k + l);
                    
                    /* Memory operation */
                    sink += val;
                    
                    /* Conditional with many branches */
                    switch (val % 8) {
                        case 0: sink += 1; break;
                        case 1: sink += 2; break;
                        case 2: sink += 3; break;
                        case 3: sink += 4; break;
                        case 4: sink += 5; break;
                        case 5: sink += 6; break;
                        case 6: sink += 7; break;
                        case 7: sink += 8; break;
                    }
                }
            }
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r" (sink) : : "memory");
}

int main(int argc, char *argv[]) {
    clock_t start, end;
    double cpu_time_used;
    unsigned long long final_checksum = 0;
    
    printf("Starting Haifa scheduler stress test...\n");
    
    /* Test with different sizes to trigger different scheduling paths */
    int sizes[] = {100, 500, 1000, 5000, 10000};
    int num_sizes = sizeof(sizes) / sizeof(sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        printf("\nTesting with size %d...\n", sizes[i]);
        
        start = clock();
        unsigned long long checksum = stress_test(sizes[i]);
        end = clock();
        
        cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
        printf("Size %d: checksum = 0x%016llx, time = %.3f seconds\n",
               sizes[i], checksum, cpu_time_used);
        
        final_checksum ^= checksum;
        
        /* Also run nested loop test */
        if (sizes[i] > 1000) {
            start = clock();
            nested_loop_test(50);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
            printf("Nested loop test: %.3f seconds\n", cpu_time_used);
        }
    }
    
    printf("\nFinal XOR checksum: 0x%016llx\n", final_checksum);
    printf("Test completed.\n");
    
    return 0;
}
