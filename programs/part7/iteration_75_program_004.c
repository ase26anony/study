/* Complex scheduling test for HAIFA scheduler state save/restore */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define CHASE_SIZE 256

/* Volatile and memory barriers to create scheduling hazards */
volatile int g_volatile_barrier = 0;
#define MEMORY_BARRIER() asm volatile("" ::: "memory")

/* Packed struct with mixed alignments */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    short s;
    float f;
};

/* Non-inlineable function to create scheduling boundaries */
static __attribute__((noinline)) 
int compute_chunk(int start, int end, int *data) {
    int result = 0;
    for (int j = start; j < end; j++) {
        result += data[j] * (j % 7);
        MEMORY_BARRIER();
    }
    return result;
}

/* Another non-inlineable function */
static __attribute__((noinline))
double process_doubles(double *arr, int size, double factor) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        arr[i] = arr[i] * factor + (i % 3);
        sum += arr[i];
        if (i & 1) {
            arr[i] = sqrt(arr[i] + 1.0);
        }
    }
    return sum;
}

/* Function pointer for computed jumps */
typedef int (*compute_func_t)(int, int*);
static compute_func_t func_table[5];

static int func0(int x, int *arr) { return arr[x] * 2; }
static int func1(int x, int *arr) { return arr[x] + arr[x-1]; }
static int func2(int x, int *arr) { return arr[x] / (x % 5 + 1); }
static int func3(int x, int *arr) { return arr[x] ^ 0x55AA55AA; }
static int func4(int x, int *arr) { return arr[x] << (x % 8); }

/* Initialize function table */
static void init_func_table(void) {
    func_table[0] = func0;
    func_table[1] = func1;
    func_table[2] = func2;
    func_table[3] = func3;
    func_table[4] = func4;
}

int main(int argc, char **argv) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Initialize arrays with different types and alignments */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    float *float_array = (float*)malloc(ARRAY_SIZE * sizeof(float));
    struct MixedData *mixed_array = (struct MixedData*)malloc(CHASE_SIZE * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        double_array[i] = (double)(i * 1103515245) / 1000000.0;
        float_array[i] = (float)(i * 1103515245) / 1000000.0f;
    }
    
    for (int i = 0; i < CHASE_SIZE; i++) {
        mixed_array[i].c = (char)(i % 256);
        mixed_array[i].i = i * 3;
        mixed_array[i].d = (double)i * 2.5;
        mixed_array[i].s = (short)(i * 7);
        mixed_array[i].f = (float)i * 1.5f;
    }
    
    init_func_table();
    
    /* Create pointer-chasing indices (simulated linked list) */
    int chase_indices[CHASE_SIZE];
    for (int i = 0; i < CHASE_SIZE; i++) {
        chase_indices[i] = (i * 97) % CHASE_SIZE;  /* Pseudo-random permutation */
    }
    
    /* Main computation loop with complex control flow */
    long long total_result = 0;
    double double_result = 0.0;
    float float_result = 0.0f;
    
    for (int iter = 0; iter < N; iter++) {
        /* Pointer chasing through mixed array */
        int chase_ptr = iter % CHASE_SIZE;
        for (int step = 0; step < 50; step++) {
            chase_ptr = chase_indices[chase_ptr];
            struct MixedData *current = &mixed_array[chase_ptr];
            
            /* Dependent arithmetic chain */
            int temp1 = current->i * 3;
            int temp2 = temp1 + current->s;
            int temp3 = temp2 / (current->c + 1);
            int temp4 = temp3 ^ 0x12345678;
            int temp5 = temp4 << (iter % 8);
            
            total_result += temp5;
            MEMORY_BARRIER();
        }
        
        /* Large basic block with many independent operations */
        for (int i = 0; i < 100; i++) {
            int idx = (iter + i) % ARRAY_SIZE;
            
            /* Independent operations to fill instruction queue */
            int_array[idx] = int_array[idx] * 2 + i;
            double_array[idx] = double_array[idx] * 1.01 + sin((double)i);
            float_array[idx] = float_array[idx] * 0.99f + cosf((float)i);
            
            if (i % 7 == 0) {
                g_volatile_barrier = i;
            }
        }
        
        /* Deeply nested conditional chain */
        int branch_selector = iter % 20;
        if (branch_selector < 5) {
            /* Call non-inlineable function */
            int chunk_start = iter % (ARRAY_SIZE - 100);
            int chunk_result = compute_chunk(chunk_start, chunk_start + 100, int_array);
            total_result += chunk_result * 3;
        } else if (branch_selector < 10) {
            /* Process doubles */
            double_result += process_doubles(double_array, 50, 1.0 + iter * 0.001);
        } else if (branch_selector < 15) {
            /* Complex arithmetic with mixed types */
            for (int i = 0; i < 30; i++) {
                int idx = (iter + i * 3) % ARRAY_SIZE;
                double temp = double_array[idx] * float_array[idx];
                int_array[idx] = (int)(temp * 1000.0);
                total_result += int_array[idx];
            }
        } else {
            /* Function pointer dispatch */
            int func_idx = iter % 5;
            int fp_result = func_table[func_idx](iter % ARRAY_SIZE, int_array);
            total_result += fp_result;
        }
        
        /* Switch statement with many cases */
        switch (iter % 10) {
            case 0: {
                /* Computation kernel 0 */
                for (int i = 0; i < 20; i++) {
                    float_result += float_array[i] * 0.5f;
                }
                break;
            }
            case 1: {
                /* Computation kernel 1 */
                double sum = 0.0;
                for (int i = 0; i < 15; i++) {
                    sum += double_array[i] * int_array[i];
                }
                double_result += sum;
                break;
            }
            case 2: {
                /* Computation kernel 2 */
                int prod = 1;
                for (int i = 1; i < 10; i++) {
                    prod *= (int_array[i] % 100) + 1;
                }
                total_result += prod;
                break;
            }
            case 3: {
                /* Computation kernel 3 - memory intensive */
                memcpy(&int_array[100], &int_array[0], 100 * sizeof(int));
                break;
            }
            case 4: {
                /* Computation kernel 4 */
                for (int i = 0; i < 25; i++) {
                    int_array[i] = (int_array[i] << 3) | (int_array[i] >> 29);
                }
                break;
            }
            case 5: {
                /* Computation kernel 5 */
                for (int i = 0; i < 30; i++) {
                    double_array[i] = pow(double_array[i], 1.001);
                }
                break;
            }
            case 6: {
                /* Computation kernel 6 */
                for (int i = 0; i < 40; i++) {
                    float_array[i] = float_array[i] + float_array[i+1] + float_array[i+2];
                }
                break;
            }
            case 7: {
                /* Computation kernel 7 */
                int xor_sum = 0;
                for (int i = 0; i < 35; i++) {
                    xor_sum ^= int_array[i];
                }
                total_result += xor_sum;
                break;
            }
            case 8: {
                /* Computation kernel 8 */
                for (int i = 0; i < 45; i++) {
                    if (int_array[i] > 1000000) {
                        int_array[i] /= 2;
                    }
                }
                break;
            }
            case 9: {
                /* Computation kernel 9 - mixed operations */
                for (int i = 0; i < 20; i++) {
                    double_array[i] = (double)int_array[i] * float_array[i];
                    float_result += (float)double_array[i];
                }
                break;
            }
        }
        
        /* Loop-carried dependency */
        static int loop_carried = 0;
        for (int i = 0; i < 25; i++) {
            loop_carried = loop_carried * 3 + int_array[i % ARRAY_SIZE];
        }
        total_result += loop_carried;
        
        /* Conditional goto to create non-linear control flow */
        if (iter % 37 == 0) {
            /* Small block with goto */
            int local_counter = 0;
        restart_point:
            local_counter++;
            total_result += local_counter;
            if (local_counter < 5) {
                goto restart_point;
            }
        }
    }
    
    /* Final reduction across all results */
    long long final_sum = total_result;
    final_sum += (long long)double_result;
    final_sum += (long long)float_result;
    
    /* Use arrays to prevent elimination */
    for (int i = 0; i < ARRAY_SIZE; i += 64) {
        final_sum += int_array[i];
        final_sum += (long long)double_array[i];
        final_sum += (long long)float_array[i];
    }
    
    for (int i = 0; i < CHASE_SIZE; i += 16) {
        final_sum += mixed_array[i].i;
        final_sum += (long long)mixed_array[i].d;
    }
    
    printf("Result: %lld\n", final_sum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return 0;
}
