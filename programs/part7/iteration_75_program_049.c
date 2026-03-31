#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile void* g_volatile_ptr = NULL;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) MixedData {
    char c;
    int i;
    double d;
    short s;
    float f;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Static helper functions that won't be inlined easily */
static __attribute__((noinline)) int helper1(int a, int b) {
    asm volatile("" : : : "memory");  /* Memory barrier */
    return (a * b) + (a ^ b) - (a & b);
}

static __attribute__((noinline)) float helper2(float x, float y) {
    volatile float temp = x * y;  /* Volatile to force memory access */
    return temp + x - y;
}

static __attribute__((noinline)) double helper3(double* arr, int idx) {
    /* Pointer chasing simulation */
    double sum = 0.0;
    for (int i = 0; i < 3; i++) {
        sum += arr[(idx + i) % 256];
        asm volatile("" : : : "memory");
    }
    return sum;
}

/* Different computation kernels */
static int kernel_add_chain(int start) {
    int a = start;
    a = a + (a << 2);
    a = a ^ (a >> 3);
    a = a * 1103515245;
    a = a + (a << 16);
    return a;
}

static int kernel_mul_chain(int start) {
    int a = start;
    a = a * 1664525;
    a = a ^ (a << 13);
    a = a * 1103515245;
    a = a ^ (a >> 15);
    return a;
}

static float kernel_float_ops(float a, float b) {
    float t1 = a * b;
    float t2 = a / (b + 1.0f);
    float t3 = sqrtf(fabsf(t1 - t2));
    return t1 + t2 + t3;
}

/* Main computation with complex control flow */
static __attribute__((noinline)) uint64_t complex_computation(int iterations, 
                                                             int* int_arr,
                                                             float* float_arr,
                                                             double* double_arr,
                                                             struct MixedData* mixed) {
    uint64_t result = 0;
    int state = 0;
    
    /* Array of function pointers for computed jumps */
    ComputeFunc funcs[] = {kernel_add_chain, kernel_mul_chain};
    
    for (int i = 0; i < iterations; i++) {
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i & 2) {
                if (i & 4) {
                    if (i & 8) {
                        /* Branch 1: Integer operations with dependencies */
                        int a = int_arr[i % 256];
                        int b = int_arr[(i + 1) % 256];
                        int c = a * b + (a ^ b);
                        int d = c << (b & 7);
                        int e = d ^ (c >> 3);
                        int f = e * 1103515245;
                        result ^= (uint64_t)f;
                        
                        /* Call helper function */
                        f = helper1(f, i);
                        result += f;
                    } else {
                        /* Branch 2: Floating point operations */
                        float x = float_arr[i % 256];
                        float y = float_arr[(i + 127) % 256];
                        float z = helper2(x, y);
                        float w = kernel_float_ops(x, z);
                        result ^= *(uint64_t*)&w;  /* Type punning */
                    }
                } else {
                    /* Branch 3: Pointer chasing through arrays */
                    double sum = 0.0;
                    int idx = i % 256;
                    for (int j = 0; j < 8; j++) {
                        sum += double_arr[idx];
                        idx = (idx * 1103515245) % 256;
                        asm volatile("" : : : "memory");
                    }
                    result ^= *(uint64_t*)&sum;
                }
            } else {
                /* Branch 4: Mixed data type accesses */
                struct MixedData* m = &mixed[i % 128];
                result ^= m->c;
                result += m->i;
                result ^= *(uint32_t*)&m->f;
                result += *(uint64_t*)&m->d;
            }
        } else {
            /* Branch 5: Computed goto via function pointer */
            int val = funcs[i & 1](i, int_arr[i % 256]);
            result += val;
        }
        
        /* Switch statement with many cases */
        switch (i % 10) {
            case 0: {
                /* Large basic block with independent operations */
                int t0 = int_arr[0] * int_arr[1];
                int t1 = int_arr[2] + int_arr[3];
                int t2 = int_arr[4] ^ int_arr[5];
                int t3 = int_arr[6] & int_arr[7];
                int t4 = int_arr[8] | int_arr[9];
                int t5 = int_arr[10] << (int_arr[11] & 7);
                int t6 = int_arr[12] >> (int_arr[13] & 7);
                int t7 = t0 + t1 - t2;
                int t8 = t3 * t4 / (t5 + 1);
                int t9 = t6 ^ t7 ^ t8;
                result += t9;
                break;
            }
            case 1:
                result ^= (uint64_t)helper3(double_arr, i % 256);
                break;
            case 2:
                result += int_arr[i % 256] * 1103515245;
                break;
            case 3:
                result ^= (uint64_t)(float_arr[i % 256] * 3.14159f);
                break;
            case 4:
                result += (uint64_t)(sin(double_arr[i % 256]) * 1000.0);
                break;
            case 5:
                result ^= (uint64_t)kernel_add_chain(i);
                break;
            case 6:
                result += (uint64_t)kernel_mul_chain(i);
                break;
            case 7:
                result ^= (uint64_t)helper1(i, i * 2);
                break;
            case 8:
                result += (uint64_t)helper2(float_arr[i % 256], float_arr[(i + 1) % 256]);
                break;
            case 9:
                result ^= (uint64_t)((i * 1103515245) & 0xFFFFFFFF);
                break;
        }
        
        /* Loop-carried dependency */
        state = (state * 1103515245 + i) & 0x7FFFFFFF;
        int_arr[i % 256] = state;
        
        /* Volatile access to force scheduling constraints */
        g_volatile_counter++;
        g_volatile_ptr = &int_arr[i % 256];
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result;
}

int main(int argc, char** argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 100000) iterations = 100000;
    }
    
    /* Allocate and initialize arrays with different alignments */
    int* int_arr = (int*)aligned_alloc(32, 256 * sizeof(int));
    float* float_arr = (float*)aligned_alloc(16, 256 * sizeof(float));
    double* double_arr = (double*)aligned_alloc(64, 256 * sizeof(double));
    struct MixedData* mixed = (struct MixedData*)malloc(128 * sizeof(struct MixedData));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < 256; i++) {
        int_arr[i] = (i * 1103515245) & 0x7FFFFFFF;
        float_arr[i] = (float)(i * 1103515245) / 1000.0f;
        double_arr[i] = (double)(i * 1103515245) / 10000.0;
    }
    
    for (int i = 0; i < 128; i++) {
        mixed[i].c = (char)(i & 0xFF);
        mixed[i].i = i * 1664525;
        mixed[i].d = (double)i / 3.14159;
        mixed[i].s = (short)(i * 1103515245);
        mixed[i].f = (float)i * 2.71828f;
    }
    
    /* Perform complex computation */
    uint64_t result = complex_computation(iterations, int_arr, float_arr, double_arr, mixed);
    
    /* Additional reduction to prevent dead code elimination */
    uint64_t final_check = 0;
    for (int i = 0; i < 256; i++) {
        final_check ^= int_arr[i];
        final_check += *(uint32_t*)&float_arr[i];
        final_check ^= *(uint64_t*)&double_arr[i % 128];
    }
    
    for (int i = 0; i < 128; i++) {
        final_check += mixed[i].i;
        final_check ^= *(uint64_t*)&mixed[i].d;
    }
    
    result ^= final_check;
    
    /* Print result to prevent optimization */
    printf("Result: %llu\n", (unsigned long long)result);
    
    /* Cleanup */
    free(int_arr);
    free(float_arr);
    free(double_arr);
    free(mixed);
    
    return 0;
}
