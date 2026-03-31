/* haifa-sched-test.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O2 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa-sched-test.c -o haifa-test
 * Or more aggressively: gcc -O3 -fsched-pressure -fschedule-insns -fschedule-insns2 -funroll-loops haifa-sched-test.c -o haifa-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_barrier = 0;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) packed_data {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*compute_func_t)(int, int);

/* Small helper functions that may be inlined or not */
static int helper_mul(int a, int b) {
    volatile int barrier = g_volatile_barrier;
    int result = a * b + barrier;
    asm volatile("" ::: "memory");  /* Compiler barrier */
    return result;
}

static int helper_add(int a, int b) {
    volatile int barrier = g_volatile_barrier;
    int result = a + b - barrier;
    asm volatile("" ::: "memory");
    return result;
}

static int helper_xor(int a, int b) {
    volatile int barrier = g_volatile_barrier;
    int result = (a ^ b) | barrier;
    asm volatile("" ::: "memory");
    return result;
}

/* Non-inlineable function (due to complexity) to create scheduling boundaries */
__attribute__((noinline)) static int complex_transform(int x) {
    struct packed_data pd;
    pd.c = (char)(x & 0xFF);
    pd.i = x * 1103515245;
    pd.d = (double)x / 3.14159;
    pd.s = (short)(x >> 8);
    
    /* Chain of dependent operations */
    int a = pd.i + (int)pd.d;
    int b = helper_mul(a, pd.s);
    int c = helper_add(b, pd.c);
    int d = helper_xor(c, a);
    
    /* Switch with multiple cases */
    switch (d & 7) {
        case 0: return d * 2;
        case 1: return d + pd.i;
        case 2: return d ^ pd.s;
        case 3: return helper_mul(d, pd.c);
        case 4: return d >> 2;
        case 5: return d | 0x5555;
        case 6: return helper_add(d, pd.i);
        case 7: return helper_xor(d, pd.s);
        default: return d;
    }
}

/* Another non-inlineable function with deep conditional chain */
__attribute__((noinline)) static int conditional_chain(int x, int y) {
    int result = x;
    
    /* Deep if-else chain */
    if (x < 0) {
        result = helper_mul(x, y);
    } else if (x < 10) {
        result = helper_add(x, y);
    } else if (x < 100) {
        result = helper_xor(x, y);
    } else if (x < 1000) {
        result = x * y + (x >> 3);
    } else if (x < 10000) {
        result = (x ^ y) | (x & y);
    } else {
        /* Nested conditionals */
        if (y & 1) {
            result = complex_transform(x);
        } else {
            result = complex_transform(y);
        }
    }
    
    /* More conditionals */
    switch (result & 3) {
        case 0: result += g_volatile_counter; break;
        case 1: result -= g_volatile_counter; break;
        case 2: result ^= g_volatile_counter; break;
        case 3: result |= g_volatile_counter; break;
    }
    
    return result;
}

/* Function pointer array for computed jumps */
static compute_func_t func_table[] = {
    helper_mul,
    helper_add,
    helper_xor,
    complex_transform,
    conditional_chain
};

int main(int argc, char *argv[]) {
    int N = 1000;  /* Default iteration count */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays of different types and alignments */
    int *int_array = (int*)malloc(N * sizeof(int));
    double *double_array = (double*)malloc(N * sizeof(double));
    float *float_array = (float*)malloc(N * sizeof(float));
    struct packed_data *packed_array = (struct packed_data*)malloc(N * sizeof(struct packed_data));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        int_array[i] = i * 1103515245;
        double_array[i] = (double)i / 1.234567;
        float_array[i] = (float)i * 3.14159f;
        packed_array[i].c = (char)(i & 0xFF);
        packed_array[i].i = i * 123456789;
        packed_array[i].d = (double)i * 2.71828;
        packed_array[i].s = (short)(i >> 4);
    }
    
    /* Pointer chasing variable */
    int *chase_ptr = int_array;
    int chase_index = 0;
    
    /* Accumulator for final result */
    uint64_t final_result = 0;
    
    /* Main computation loop - designed to create complex scheduling */
    for (int i = 0; i < N; i++) {
        /* Update volatile to create scheduling hazards */
        g_volatile_counter++;
        asm volatile("" ::: "memory");
        
        /* Pointer chasing through array (simulates linked list) */
        chase_index = (chase_index + *chase_ptr) % N;
        chase_ptr = &int_array[chase_index];
        
        /* Chain of dependent arithmetic operations */
        int a = int_array[i];
        int b = helper_mul(a, (int)double_array[i]);
        int c = helper_add(b, (int)float_array[i]);
        int d = helper_xor(c, packed_array[i].i);
        int e = d * 3 + (d >> 2);
        
        /* Mixed integer/floating point calculations */
        double f = double_array[i] * 1.234;
        float g = float_array[i] / 2.345f;
        int h = (int)(f * g) + e;
        
        /* Switch statement with many cases - creates control flow complexity */
        switch (i % 10) {
            case 0: h = helper_mul(h, a); break;
            case 1: h = helper_add(h, b); break;
            case 2: h = helper_xor(h, c); break;
            case 3: h = complex_transform(h); break;
            case 4: h = conditional_chain(h, d); break;
            case 5: h = h * 2 - b; break;
            case 6: h = (h ^ a) | (h & b); break;
            case 7: h = h + (int)(f * 100.0); break;
            case 8: h = h - (int)(g * 50.0f); break;
            case 9: h = func_table[i % 5](h, e); break;  /* Computed jump */
        }
        
        /* Conditional with function call in one branch */
        if (i & 1) {
            /* Call helper in one branch only */
            h = helper_mul(h, complex_transform(i));
        } else {
            /* Different computation in other branch */
            h = helper_add(h, conditional_chain(i, h));
        }
        
        /* Memory operations with different alignments */
        packed_array[i].i = h;
        int_array[chase_index] = h ^ i;
        double_array[i] = (double)h / 456.789;
        float_array[i] = (float)h * 0.12345f;
        
        /* Accumulate result */
        final_result ^= (uint64_t)h;
        final_result += (uint64_t)packed_array[i].i;
        
        /* Occasionally update volatile barrier */
        if ((i & 0xFF) == 0) {
            g_volatile_barrier = i;
            asm volatile("" ::: "memory");
        }
    }
    
    /* Second pass: large basic block to fill instruction queue */
    int temp_sum = 0;
    for (int i = 0; i < N; i++) {
        /* Many independent operations - scheduler can reorder aggressively */
        int t1 = int_array[i] * 3;
        int t2 = (int)double_array[i] + 5;
        int t3 = packed_array[i].i >> 2;
        int t4 = (int)float_array[i] * 7;
        int t5 = t1 + t2;
        int t6 = t3 - t4;
        int t7 = t5 * t6;
        int t8 = t7 ^ int_array[(i + 1) % N];
        int t9 = t8 + packed_array[(i + 2) % N].s;
        int t10 = t9 * 11;
        
        /* Store results back */
        int_array[i] = t10;
        temp_sum += t10;
        
        /* More independent operations */
        double_array[i] = (double)t10 / 12.34;
        float_array[i] = (float)t10 * 0.567f;
        packed_array[i].c = (char)(t10 & 0xFF);
        packed_array[i].s = (short)(t10 >> 8);
    }
    
    final_result ^= (uint64_t)temp_sum;
    
    /* Final reduction across all arrays */
    for (int i = 0; i < N; i++) {
        final_result += int_array[i];
        final_result ^= (uint64_t)double_array[i];
        final_result += packed_array[i].i;
        final_result ^= (uint64_t)float_array[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %lu\n", (unsigned long)final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(packed_array);
    
    return (int)(final_result & 0x7FFFFFFF);
}
