/* haifa_sched_trigger.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and exercise the free_state function uncovered lines.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_barrier = 0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    char trailing;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Different computation kernels for switch statement */
static int kernel_add(int a, int b) { 
    g_volatile_barrier++;
    return a + b + g_volatile_counter; 
}

static int kernel_mul(int a, int b) { 
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return a * b - g_volatile_counter; 
}

static int kernel_xor(int a, int b) { 
    g_volatile_counter ^= a;
    return a ^ b ^ g_volatile_counter; 
}

static int kernel_shift(int a, int b) { 
    return (a << (b & 7)) | (a >> (8 - (b & 7))); 
}

/* Helper function with complex dependencies */
static double helper_complex_calc(double *arr, int idx, int n) {
    double result = 0.0;
    volatile double temp = 0.0;  /* Volatile to prevent optimization */
    
    /* Loop with carried dependency */
    for (int i = 0; i < n; i++) {
        result = result * 1.01 + arr[(idx + i) % n];
        temp = result;  /* Force memory write */
    }
    
    /* Mixed operations */
    result = result / (n + 1.0);
    asm volatile("" ::: "memory");  /* Barrier */
    
    return result;
}

/* Another helper with pointer chasing */
static int pointer_chase(int *array, int size, int start) {
    int idx = start;
    int sum = 0;
    
    /* Pointer chasing through array (simulated linked list) */
    for (int i = 0; i < size / 2; i++) {
        idx = array[idx] % size;
        sum += array[idx];
        g_volatile_counter++;  /* Volatile access creates scheduling hazard */
    }
    
    return sum;
}

/* Main computation with complex control flow */
static uint64_t complex_computation(int iterations, int array_size) {
    /* Allocate arrays with different types and alignments */
    int *int_array = (int*)malloc(array_size * sizeof(int));
    double *double_array = (double*)malloc(array_size * sizeof(double));
    struct PackedData *packed_array = (struct PackedData*)malloc(array_size * sizeof(struct PackedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = (i * 1103515245) & (array_size - 1);
        double_array[i] = (i * 0.123456789) + 1.0;
        packed_array[i].c = i & 0xFF;
        packed_array[i].i = i * 3;
        packed_array[i].d = i * 0.987654321;
        packed_array[i].trailing = (i >> 8) & 0xFF;
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {kernel_add, kernel_mul, kernel_xor, kernel_shift};
    
    uint64_t accumulator = 0;
    double fp_accumulator = 0.0;
    
    /* Main computation loop - designed to create complex scheduling */
    for (int iter = 0; iter < iterations; iter++) {
        int base_idx = iter % array_size;
        
        /* Deeply nested conditional chain */
        if (iter & 1) {
            /* Branch 1: Pointer chasing with volatile accesses */
            int chase_result = pointer_chase(int_array, array_size, base_idx);
            accumulator += chase_result;
            
            /* Call helper function */
            fp_accumulator += helper_complex_calc(double_array, base_idx, 16);
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
        } 
        else if (iter & 2) {
            /* Branch 2: Mixed integer/FP operations */
            for (int j = 0; j < 8; j++) {
                double temp = double_array[(base_idx + j) % array_size];
                int_array[(base_idx + j) % array_size] += (int)(temp * 100.0);
                fp_accumulator = fp_accumulator * 0.99 + temp;
            }
        } 
        else if (iter & 4) {
            /* Branch 3: Packed struct operations */
            for (int j = 0; j < 4; j++) {
                struct PackedData *p = &packed_array[(base_idx + j) % array_size];
                accumulator += p->i + p->c;
                fp_accumulator += p->d;
            }
        }
        
        /* Switch statement with many cases - creates control flow complexity */
        switch (iter % 10) {
            case 0: {
                /* Large basic block with independent operations */
                int t0 = int_array[base_idx];
                int t1 = int_array[(base_idx + 1) % array_size];
                int t2 = int_array[(base_idx + 2) % array_size];
                int t3 = int_array[(base_idx + 3) % array_size];
                int t4 = int_array[(base_idx + 4) % array_size];
                int t5 = int_array[(base_idx + 5) % array_size];
                int t6 = int_array[(base_idx + 6) % array_size];
                int t7 = int_array[(base_idx + 7) % array_size];
                
                /* Chain of dependent operations */
                t0 = t0 * t1 + t2;
                t1 = t1 ^ t3 | t4;
                t2 = t2 + t5 - t6;
                t3 = t3 * t7 / (t0 + 1);
                t4 = (t4 << 3) | (t5 >> 5);
                t5 = t5 + t6 * t7;
                t6 = t6 ^ t0 & t1;
                t7 = t7 | t2 ^ t3;
                
                accumulator += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
                break;
            }
            case 1:
                accumulator += funcs[0](int_array[base_idx], int_array[(base_idx + 1) % array_size]);
                break;
            case 2:
                accumulator += funcs[1](int_array[base_idx], int_array[(base_idx + 2) % array_size]);
                break;
            case 3:
                accumulator += funcs[2](int_array[base_idx], int_array[(base_idx + 3) % array_size]);
                break;
            case 4:
                accumulator += funcs[3](int_array[base_idx], int_array[(base_idx + 4) % array_size]);
                break;
            case 5:
                /* Loop with carried dependency */
                {
                    int sum = 0;
                    for (int k = 0; k < 16; k++) {
                        sum += int_array[(base_idx + k) % array_size] * 
                               int_array[(base_idx + k - 1 + array_size) % array_size];
                    }
                    accumulator += sum;
                }
                break;
            case 6:
                /* Mixed type operations */
                fp_accumulator += double_array[base_idx] * 1.5;
                accumulator += (int)(fp_accumulator);
                break;
            case 7:
                /* Another volatile access */
                g_volatile_barrier = iter;
                accumulator += g_volatile_counter;
                break;
            case 8:
                /* Memory intensive */
                memcpy(&int_array[base_idx % (array_size/2)], 
                       &int_array[(base_idx + array_size/2) % array_size], 
                       sizeof(int) * 8);
                break;
            case 9:
                /* Computed goto simulation */
                {
                    int idx = iter % 4;
                    int result = funcs[idx](int_array[base_idx], iter);
                    accumulator += result;
                }
                break;
        }
        
        /* Additional conditional with inline asm barrier */
        if ((iter % 17) == 0) {
            asm volatile("" ::: "memory");
            g_volatile_counter ^= accumulator;
        }
    }
    
    /* Final reduction across arrays */
    for (int i = 0; i < array_size; i += 4) {
        accumulator += int_array[i];
        fp_accumulator += double_array[i];
        accumulator += packed_array[i].i;
    }
    
    /* Combine results */
    uint64_t final_result = accumulator + (uint64_t)fp_accumulator;
    final_result ^= g_volatile_counter;
    final_result ^= g_volatile_barrier;
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(packed_array);
    
    return final_result;
}

int main(int argc, char *argv[]) {
    /* Parse iteration count from command line */
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Use power-of-two array size for modulo optimization */
    int array_size = 1024;
    
    printf("Starting complex computation with %d iterations...\n", iterations);
    
    uint64_t result = complex_computation(iterations, array_size);
    
    printf("Result: %llu\n", (unsigned long long)result);
    
    /* Prevent dead code elimination */
    volatile uint64_t volatile_result = result;
    if (volatile_result == 0x12345678) {  /* Always false */
        printf("Impossible branch\n");
    }
    
    return 0;
}
