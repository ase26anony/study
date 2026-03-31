/* haifa_sched_coverage.c
 * Program designed to trigger GCC's HAIFA scheduler state save/restore
 * and ensure free_state() is called with populated scheduler structures.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Volatile variables to create scheduling barriers */
static volatile int g_volatile_counter = 0;
static volatile int* g_volatile_ptr = NULL;

/* Packed struct to force misaligned accesses */
struct __attribute__((packed)) PackedData {
    char c;
    int i;
    double d;
    char trailing;
};

/* Function pointer type for computed jumps */
typedef int (*ComputeFunc)(int, int);

/* Non-inlineable helper functions to create scheduling boundaries */
static __attribute__((noinline)) int helper_complex(int a, int b) {
    volatile int barrier = a * b;
    asm volatile("" ::: "memory");
    int result = (a + b) * (a - b);
    asm volatile("" ::: "memory");
    return result + barrier;
}

static __attribute__((noinline)) double helper_fp(double x, double y) {
    volatile double v = x;
    asm volatile("" ::: "memory");
    double r = x * y + x / (y + 1.0);
    asm volatile("" ::: "memory");
    return r + v;
}

/* Different computation kernels for switch statement */
static int kernel_add(int a, int b) { return a + b; }
static int kernel_mul(int a, int b) { return a * b; }
static int kernel_sub(int a, int b) { return a - b; }
static int kernel_xor(int a, int b) { return a ^ b; }
static int kernel_and(int a, int b) { return a & b; }
static int kernel_or(int a, int b) { return a | b; }
static int kernel_shift(int a, int b) { return (a << 3) | (b >> 2); }
static int kernel_mix(int a, int b) { return (a * 3 + b * 7) / 2; }
static int kernel_mod(int a, int b) { return (a % (b + 1)) + (b % (a + 1)); }
static int kernel_chain(int a, int b) { 
    int t = a * b;
    t = t + (a << 2);
    t = t ^ (b >> 1);
    return t;
}

/* Array of function pointers for computed jumps */
static ComputeFunc kernels[] = {
    kernel_add, kernel_mul, kernel_sub, kernel_xor, kernel_and,
    kernel_or, kernel_shift, kernel_mix, kernel_mod, kernel_chain
};

/* Main computation with complex control flow and dependencies */
static uint64_t complex_computation(int* int_array, double* dbl_array, 
                                   float* flt_array, struct PackedData* packed,
                                   int size, int iterations) {
    uint64_t accumulator = 0;
    int i, j;
    
    /* Create complex pointer-chasing pattern */
    int* chase_ptr = int_array;
    double* dbl_chase = dbl_array;
    
    for (i = 0; i < iterations; i++) {
        /* Deeply nested conditional chain */
        if (i & 1) {
            if (i % 3 == 0) {
                if (i % 5 == 0) {
                    /* Branch 1: Pointer chasing with arithmetic */
                    for (j = 0; j < 8; j++) {
                        *chase_ptr = *chase_ptr * 1103515245 + 12345;
                        chase_ptr = &int_array[*chase_ptr % size];
                        accumulator += *chase_ptr;
                    }
                } else {
                    /* Branch 2: Mixed FP and integer */
                    double temp = dbl_array[i % size];
                    for (j = 0; j < 4; j++) {
                        temp = helper_fp(temp, dbl_array[(i + j) % size]);
                        int_array[(i + j) % size] = (int)temp;
                        accumulator += (int)temp;
                    }
                }
            } else {
                /* Branch 3: Call helper function */
                int result = helper_complex(i, int_array[i % size]);
                accumulator += result;
                g_volatile_counter = result; /* Memory barrier */
            }
        } else {
            /* Branch 4: Large basic block with independent operations */
            /* This should fill the instruction queue */
            int a = int_array[(i + 0) % size];
            int b = int_array[(i + 1) % size];
            int c = int_array[(i + 2) % size];
            int d = int_array[(i + 3) % size];
            int e = int_array[(i + 4) % size];
            int f = int_array[(i + 5) % size];
            int g = int_array[(i + 6) % size];
            int h = int_array[(i + 7) % size];
            
            /* Chain of dependent operations */
            a = a * b + c;
            b = b * c + d;
            c = c * d + e;
            d = d * e + f;
            e = e * f + g;
            f = f * g + h;
            g = g * h + a;
            h = h * a + b;
            
            /* More operations to create scheduling pressure */
            a = (a << 3) | (b >> 2);
            b = (b << 3) | (c >> 2);
            c = (c << 3) | (d >> 2);
            d = (d << 3) | (e >> 2);
            
            /* Store results back */
            int_array[(i + 0) % size] = a;
            int_array[(i + 1) % size] = b;
            int_array[(i + 2) % size] = c;
            int_array[(i + 3) % size] = d;
            
            accumulator += a + b + c + d;
        }
        
        /* Switch statement with many cases - creates control flow complexity */
        switch (i % 10) {
            case 0: accumulator += kernels[0](i, int_array[i % size]); break;
            case 1: accumulator += kernels[1](i, int_array[i % size]); break;
            case 2: accumulator += kernels[2](i, int_array[i % size]); break;
            case 3: accumulator += kernels[3](i, int_array[i % size]); break;
            case 4: accumulator += kernels[4](i, int_array[i % size]); break;
            case 5: accumulator += kernels[5](i, int_array[i % size]); break;
            case 6: accumulator += kernels[6](i, int_array[i % size]); break;
            case 7: accumulator += kernels[7](i, int_array[i % size]); break;
            case 8: accumulator += kernels[8](i, int_array[i % size]); break;
            case 9: accumulator += kernels[9](i, int_array[i % size]); break;
        }
        
        /* Packed struct access with varying alignments */
        if (i % 7 == 0) {
            packed[i % size].c = (char)i;
            packed[i % size].i = int_array[i % size];
            packed[i % size].d = dbl_array[i % size];
            packed[i % size].trailing = (char)(i & 0xFF);
            
            /* Access with type punning */
            accumulator += packed[i % size].i;
        }
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Volatile access */
        g_volatile_ptr = &int_array[i % size];
        accumulator += *g_volatile_ptr;
    }
    
    return accumulator;
}

int main(int argc, char** argv) {
    int iterations = 1000;
    const int array_size = 1024;
    
    /* Parse iteration count from command line */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Allocate and initialize arrays with different types */
    int* int_array = (int*)malloc(array_size * sizeof(int));
    double* dbl_array = (double*)malloc(array_size * sizeof(double));
    float* flt_array = (float*)malloc(array_size * sizeof(float));
    struct PackedData* packed = (struct PackedData*)malloc(array_size * sizeof(struct PackedData));
    
    if (!int_array || !dbl_array || !flt_array || !packed) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < array_size; i++) {
        int_array[i] = i * 1103515245;
        dbl_array[i] = (double)i * 3.1415926535;
        flt_array[i] = (float)i * 2.7182818284f;
        packed[i].c = (char)i;
        packed[i].i = i * 12345;
        packed[i].d = (double)i * 1.41421356;
        packed[i].trailing = (char)(i & 0xFF);
    }
    
    /* Perform complex computation */
    uint64_t result = complex_computation(int_array, dbl_array, flt_array, 
                                         packed, array_size, iterations);
    
    /* Additional reduction to prevent dead code elimination */
    uint64_t final_sum = result;
    for (int i = 0; i < array_size; i++) {
        final_sum += int_array[i];
        final_sum += (uint64_t)dbl_array[i];
        final_sum += (uint64_t)flt_array[i];
        final_sum += packed[i].i;
    }
    
    /* Print result to prevent optimization */
    printf("Result: %lu\n", (unsigned long)final_sum);
    
    /* Cleanup */
    free(int_array);
    free(dbl_array);
    free(flt_array);
    free(packed);
    
    return 0;
}
