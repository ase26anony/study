/* sel_sched_test.c - Test program for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays with different types to create varied RTL */
volatile int g_volatile_seed = 42;
int g_array_int[1024];
unsigned short g_array_short[2048];
float g_array_float[512];
double g_array_double[256];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int helper_noinline_1(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline)) unsigned helper_noinline_2(unsigned a, unsigned b) {
    asm volatile ("" : : : "memory");  /* Memory barrier */
    return (a << 3) | (b >> 5);
}

/* Pure function for loop computations */
__attribute__((const)) int pure_multiply(int a, int b) {
    return a * b;
}

__attribute__((const)) double pure_sqrt_approx(double x) {
    /* Simple approximation */
    double y = x;
    for (int i = 0; i < 3; i++) {
        y = 0.5 * (y + x / y);
    }
    return y;
}

/* Secondary computation function with complex loops */
__attribute__((noinline)) long complex_nested_loops(int outer_limit, 
                                                    int inner_limit,
                                                    int threshold) {
    long total = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_var = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < outer_limit; ++i) {
        int outer_mod = i % 7;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > threshold, 0)) {
            /* Inner loop with different counter type */
            for (unsigned j = 0; j < (unsigned)inner_limit; ++j) {
                /* Mix of arithmetic operations */
                int temp = g_array_int[i] + g_array_short[j];
                temp = pure_multiply(temp, outer_mod);
                
                /* Loop-carried dependency */
                reg_acc = reg_acc + temp;
                
                /* Memory operation with potential aliasing */
                g_array_double[j % 256] = (double)temp * 0.5;
                
                /* Conditional branch inside innermost loop */
                if (__builtin_expect((j & 3) == 0, 1)) {
                    stack_var += helper_noinline_1(i, j);
                    asm volatile ("" : : : "memory");  /* Scheduling boundary */
                } else {
                    stack_var -= helper_noinline_2(i, j);
                }
                
                /* Another memory operation */
                g_array_float[(i + j) % 512] = (float)(reg_acc % 1000);
            }
        } else {
            /* Different path with short inner loop */
            for (short k = 0; k < (short)(inner_limit / 2); ++k) {
                double approx = pure_sqrt_approx(g_array_double[k] + 1.0);
                g_array_int[k] = (int)(approx * 100.0);
                reg_acc ^= g_array_int[k];
            }
        }
        
        /* Function call with loop-variant arguments */
        total += helper_noinline_1(reg_acc, stack_var);
        
        /* Reset some variables for next iteration */
        if (i % 5 == 0) {
            stack_var = 0;
            asm volatile ("" : : : "memory");
        }
    }
    
    return total + reg_acc;
}

/* Another computation with different patterns */
__attribute__((noinline)) double floating_point_loops(int iterations) {
    double sum = 0.0;
    volatile double vol_d = 1.0;  /* Prevent optimization */
    
    /* Loop with floating point computations */
    for (int i = 0; i < iterations; i++) {
        double a = g_array_double[i % 256];
        double b = g_array_float[i % 512] * vol_d;
        
        /* Complex floating point expression */
        double c = a * b + a / (b + 1.0);
        
        /* Conditional with floating comparison */
        if (c > 100.0) {
            sum += c * 0.9;
            asm volatile ("" : : : "memory");
        } else if (c < -50.0) {
            sum -= c * 1.1;
        } else {
            sum += c;
        }
        
        /* Update global with result */
        g_array_double[(i * 3) % 256] = sum;
    }
    
    return sum;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) void warm_up_computation(void) {
    int warm_acc = 0;
    for (int i = 0; i < 100; i++) {
        warm_acc += helper_noinline_1(i, i * 2);
        warm_acc ^= helper_noinline_2(i, i + 1);
    }
    g_array_int[0] = warm_acc;  /* Store result to prevent elimination */
}

/* Initialize data with pseudo-random values */
void initialize_data(void) {
    /* Simple LCG for pseudo-random values */
    unsigned lcg = g_volatile_seed;
    
    for (int i = 0; i < 1024; i++) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_int[i] = (int)(lcg % 1000);
    }
    
    for (int i = 0; i < 2048; i++) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_short[i] = (unsigned short)(lcg % 65535);
    }
    
    for (int i = 0; i < 512; i++) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_float[i] = (float)((lcg % 2000) - 1000) / 100.0f;
    }
    
    for (int i = 0; i < 256; i++) {
        lcg = (lcg * 1103515245 + 12345) & 0x7fffffff;
        g_array_double[i] = (double)((lcg % 3000) - 1500) / 50.0;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments for variability */
    int outer_lim = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_lim = (argc > 2) ? atoi(argv[2]) : 100;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    int iterations = (argc > 4) ? atoi(argv[4]) : 200;
    
    /* Initialize with pseudo-random data */
    initialize_data();
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, threshold=%d, iter=%d\n",
           outer_lim, inner_lim, threshold, iterations);
    
    /* Warm-up to trigger compilation paths */
    warm_up_computation();
    
    /* Main computation with nested loops */
    long result1 = complex_nested_loops(outer_lim, inner_lim, threshold);
    
    /* Second computation with different patterns */
    double result2 = floating_point_loops(iterations);
    
    /* Combine results into checksum */
    unsigned long long checksum = (unsigned long long)result1;
    checksum ^= (unsigned long long)(result2 * 1000000.0);
    
    printf("Result 1: %ld\n", result1);
    printf("Result 2: %f\n", result2);
    printf("Checksum: %llu\n", checksum);
    
    /* Use results to prevent optimization */
    g_array_int[1] = (int)(checksum & 0xFFFFFFFF);
    g_array_int[2] = (int)(checksum >> 32);
    
    return (checksum % 1000) == 0 ? 0 : 1;
}
