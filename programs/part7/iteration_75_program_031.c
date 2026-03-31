/* haifa_scheduler_test.c
 * Complex program to trigger HAIFA scheduler state save/restore operations
 * Compile with: gcc -O3 -fschedule-insns -fschedule-insns2 -fno-omit-frame-pointer haifa_scheduler_test.c -o haifa_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* Volatile variables to create scheduling hazards */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_barrier = 0;

/* Packed struct to force unaligned accesses */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    short s;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Various computation kernels */
static int kernel_add(int a, int b) { return a + b + g_volatile_counter; }
static int kernel_mul(int a, int b) { return a * b - g_volatile_counter; }
static int kernel_xor(int a, int b) { return a ^ b ^ g_volatile_counter; }
static int kernel_shift(int a, int b) { return (a << 3) | (b >> 2); }
static int kernel_mod(int a, int b) { return b ? a % b : a; }

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
static int complex_helper(int x, int y, int z) {
    int result = 0;
    for (int i = 0; i < 8; i++) {
        result += (x * y) >> i;
        result ^= (z + i) * 1103515245;
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    g_volatile_barrier = result;
    return result;
}

/* Another helper with mixed operations */
__attribute__((noinline))
static double floating_helper(double* arr, int idx, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        sum += arr[(idx + i) % 256] * 1.5;
        sum -= arr[(idx - i + 256) % 256] * 0.75;
        /* Volatile store to prevent reordering */
        g_volatile_counter = i;
    }
    return sum;
}

int main(int argc, char** argv) {
    int N = 1000;
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N <= 0) N = 1000;
    }
    
    /* Allocate arrays with different types and alignments */
    int* int_array = (int*)malloc(1024 * sizeof(int));
    double* double_array = (double*)malloc(1024 * sizeof(double));
    float* float_array = (float*)malloc(1024 * sizeof(float));
    struct MixedData* mixed_array = (struct MixedData*)malloc(256 * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 1024; i++) {
        int_array[i] = (i * 1103515245) & 0x7FFFFFFF;
        double_array[i] = (double)(i * 1103515245) / 1000000.0;
        float_array[i] = (float)(i * 1103515245) / 1000000.0f;
    }
    for (int i = 0; i < 256; i++) {
        mixed_array[i].c = (char)(i & 0xFF);
        mixed_array[i].i = i * 123456789;
        mixed_array[i].d = (double)i * 3.14159;
        mixed_array[i].s = (short)(i * 54321);
    }
    
    /* Array of function pointers for computed jumps */
    ComputeFunc funcs[] = {kernel_add, kernel_mul, kernel_xor, kernel_shift, kernel_mod};
    
    /* Main computation loop with complex control flow */
    int result = 0;
    double acc_double = 0.0;
    float acc_float = 0.0f;
    
    for (int iter = 0; iter < N; iter++) {
        /* Pointer chasing through int_array (simulated linked list) */
        int idx = iter & 1023;
        int chase_sum = 0;
        for (int chase = 0; chase < 32; chase++) {
            chase_sum += int_array[idx];
            idx = int_array[idx] & 1023;  /* Next "pointer" */
            if (idx == 0) idx = 1;
        }
        
        /* Chain of dependent arithmetic operations */
        int a = chase_sum;
        int b = int_array[iter & 1023];
        int c = a * b + 12345;
        int d = c ^ (a << 3);
        int e = d % (b + 1);
        int f = e + (c >> 2);
        int g = f * 3 - d;
        
        /* Deeply nested conditional chain */
        if (iter & 1) {
            g = complex_helper(a, b, c);
            if (iter & 2) {
                acc_double += floating_helper(double_array, iter & 255, 16);
                if (iter & 4) {
                    acc_float += float_array[iter & 1023] * 2.0f;
                    if (iter & 8) {
                        /* Access packed struct with unaligned reads */
                        struct MixedData* m = &mixed_array[iter & 255];
                        g += m->i + m->s;
                    }
                }
            }
        } else if (iter & 16) {
            /* Different computation path */
            for (int j = 0; j < 8; j++) {
                g += int_array[(iter + j) & 1023] * j;
            }
        } else {
            g -= int_array[iter & 1023] / 7;
        }
        
        /* Switch statement with many cases */
        switch (iter % 10) {
            case 0: result += g * 2; break;
            case 1: result += g + int_array[iter & 1023]; break;
            case 2: result ^= g; break;
            case 3: result += g >> 3; break;
            case 4: result += g << 2; break;
            case 5: result += g % 17; break;
            case 6: result += g / 5; break;
            case 7: result += g & 0xFF; break;
            case 8: result += g | 0xAA; break;
            case 9: result += ~g; break;
        }
        
        /* Computed jump using function pointers */
        int func_idx = (iter * 1103515245) % 5;
        result += funcs[func_idx](g, iter);
        
        /* Large basic block with many independent operations */
        int temp[16];
        for (int i = 0; i < 16; i++) {
            temp[i] = int_array[(iter + i) & 1023] * i;
            temp[i] += double_array[(iter + i) & 1023] > 0.0 ? 1 : 0;
            temp[i] ^= float_array[(iter + i) & 1023] * 100;
            /* Memory barrier every 4 iterations */
            if (i % 4 == 0) {
                asm volatile("" ::: "memory");
            }
        }
        
        /* Reduction across temp array */
        int block_sum = 0;
        for (int i = 0; i < 16; i++) {
            block_sum += temp[i];
        }
        result += block_sum;
        
        /* Update volatile variables */
        g_volatile_counter = iter & 0xFF;
        g_volatile_barrier = result & 0xFFFF;
    }
    
    /* Final reduction across all arrays */
    int final_sum = result;
    for (int i = 0; i < 1024; i += 64) {
        final_sum += int_array[i];
        final_sum ^= (int)(double_array[i] * 1000.0);
        final_sum += (int)(float_array[i] * 100.0f);
    }
    
    /* Access mixed array with unaligned reads */
    for (int i = 0; i < 256; i += 8) {
        final_sum += mixed_array[i].i;
        final_sum += mixed_array[i].s;
    }
    
    /* Add accumulated floating point values */
    final_sum += (int)acc_double;
    final_sum += (int)acc_float;
    
    printf("Final result: %d\n", final_sum);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(mixed_array);
    
    return 0;
}
