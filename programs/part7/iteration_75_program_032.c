/* haifa_sched_coverage.c
 * Program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa_sched_coverage.c -o haifa_test
 * Run with: ./haifa_test [iterations]
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int* g_volatile_ptr = NULL;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) mixed_data {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Helper functions with different computation patterns */
static int compute_a(int x, int y) {
    volatile int barrier;
    barrier = x * y;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return barrier + (x ^ y);
}

static int compute_b(int x, int y) {
    int t = x;
    for (int i = 0; i < 3; i++) {
        t = (t * 17 + y) >> 1;
    }
    return t;
}

static int compute_c(int x, int y) {
    return (x * x - y * y) / (x - y + 1);
}

/* Non-inlineable function (due to complexity) to create scheduling boundaries */
__attribute__((noinline)) 
static int complex_transform(int val) {
    struct mixed_data md;
    md.c = (char)(val & 0xFF);
    md.i = val * 1103515245;
    md.d = (double)val * 3.14159;
    md.s = (short)(val >> 16);
    
    /* Chain of dependent operations */
    int a = md.i * md.c;
    int b = a + (int)md.d;
    int c = b ^ md.s;
    int d = c * 31;
    int e = d - (md.i >> 8);
    
    /* Memory barrier */
    asm volatile("" ::: "memory");
    
    return e;
}

/* Function with switch statement creating multiple basic blocks */
static int switch_computation(int idx, int value) {
    int result = value;
    
    switch (idx % 10) {
        case 0:
            result = result * 3 + 1;
            result = result ^ 0x55AA55AA;
            break;
        case 1:
            result = (result << 4) | (result >> 28);
            result += 0x12345678;
            break;
        case 2:
            result = result * result;
            result = result % 9973;
            break;
        case 3:
            result = complex_transform(result);
            break;
        case 4:
            for (int i = 0; i < 5; i++) {
                result = (result * 13 + i) & 0xFFFF;
            }
            break;
        case 5:
            result = result ^ (result >> 16);
            result = result * 0x5BD1E995;
            break;
        case 6:
            result = compute_a(result, idx);
            break;
        case 7:
            result = compute_b(result, idx);
            break;
        case 8:
            result = compute_c(result, idx);
            break;
        case 9:
            result = ~result;
            result = result * 2 - idx;
            break;
    }
    
    return result;
}

/* Pointer chasing through array */
static int pointer_chase(int* array, int size, int start) {
    int idx = start % size;
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        sum += array[idx];
        idx = array[idx] % size;
        if (idx == 0) idx = 1;
        
        /* Memory barrier every 10 iterations */
        if (i % 10 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    return sum;
}

/* Main computation with complex control flow */
static uint64_t run_computation(int iterations, int array_size) {
    /* Allocate arrays with different types and alignments */
    int* int_array = (int*)malloc(array_size * sizeof(int));
    double* double_array = (double*)malloc(array_size * sizeof(double));
    struct mixed_data* struct_array = (struct mixed_data*)malloc(array_size * sizeof(struct mixed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        double_array[i] = (double)(i * 1103515245) / 1000000.0;
        struct_array[i].c = (char)(i & 0xFF);
        struct_array[i].i = i * 1103515245;
        struct_array[i].d = (double)i * 3.14159;
        struct_array[i].s = (short)(i & 0xFFFF);
    }
    
    /* Array of function pointers for computed jumps */
    compute_func_t funcs[] = {compute_a, compute_b, compute_c};
    
    uint64_t accumulator = 0;
    int volatile_counter_local = 0;
    
    /* Main computation loop with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        int base_val = iter;
        
        /* Deeply nested conditional chain */
        if (iter & 1) {
            base_val = complex_transform(base_val);
            if (iter & 2) {
                base_val = pointer_chase(int_array, array_size, base_val);
                if (iter & 4) {
                    base_val = compute_a(base_val, iter);
                    if (iter & 8) {
                        base_val = compute_b(base_val, iter);
                        if (iter & 16) {
                            base_val = compute_c(base_val, iter);
                        }
                    }
                }
            }
        } else {
            /* Alternative path with different operations */
            base_val = base_val * 3 + 1;
            for (int j = 0; j < 10; j++) {
                base_val = (base_val << 1) | (base_val >> 31);
            }
        }
        
        /* Switch statement creating multiple basic blocks */
        base_val = switch_computation(iter, base_val);
        
        /* Computed jump via function pointer */
        int func_idx = (iter * 1103515245) % 3;
        base_val = funcs[func_idx](base_val, iter);
        
        /* Mixed data type operations */
        double temp_d = double_array[iter % array_size];
        temp_d = temp_d * 1.23456 + (double)base_val;
        int_array[iter % array_size] = (int)temp_d;
        
        /* Update volatile variable (creates scheduling hazard) */
        g_volatile_counter++;
        volatile_counter_local = g_volatile_counter;
        
        /* Large basic block with independent operations */
        int temp = base_val;
        for (int k = 0; k < 20; k++) {
            /* Independent operations that can fill instruction queue */
            int_array[(iter + k) % array_size] += temp;
            double_array[(iter + k) % array_size] *= 0.999;
            temp = (temp * 13 + k) & 0xFF;
            
            /* Access packed struct with potential misalignment */
            struct_array[(iter + k) % array_size].i ^= temp;
        }
        
        /* Reduction */
        accumulator ^= (uint64_t)base_val;
        accumulator += (uint64_t)volatile_counter_local;
        accumulator = (accumulator << 1) | (accumulator >> 63);
    }
    
    /* Final reduction across arrays */
    for (int i = 0; i < array_size; i++) {
        accumulator ^= (uint64_t)int_array[i];
        accumulator += (uint64_t)(double_array[i] * 1000);
        accumulator ^= (uint64_t)struct_array[i].i;
    }
    
    free(int_array);
    free(double_array);
    free(struct_array);
    
    return accumulator;
}

int main(int argc, char** argv) {
    int iterations = 1000;
    int array_size = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size <= 0) array_size = 1000;
    }
    
    printf("Running HAIFA scheduler coverage test...\n");
    printf("Iterations: %d, Array size: %d\n", iterations, array_size);
    
    clock_t start = clock();
    uint64_t result = run_computation(iterations, array_size);
    clock_t end = clock();
    
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Result: 0x%016llX\n", (unsigned long long)result);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    
    return 0;
}
