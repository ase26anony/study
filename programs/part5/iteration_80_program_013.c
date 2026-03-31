/* sel_sched_test.c - Test program to trigger selective scheduling dump logic */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays with different types to create varied RTL */
int global_arr_int[1024];
unsigned global_arr_unsigned[1024];
short global_arr_short[2048];
volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func1(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline)) unsigned noinline_func2(unsigned a, unsigned b) {
    return (a + b) * (a - b);
}

/* Pure function for loop-invariant computations */
__attribute__((const)) int pure_func(int x) {
    return x * x - x + 1;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* arr, int n) {
    int acc = 0;
    for (int i = 0; i < n; ++i) {
        acc = arr[i] + acc * 31;
    }
    return acc;
}

/* Struct with different field types to test memory aliasing */
struct MixedData {
    int a;
    short b;
    unsigned c;
    char d;
};

/* Global struct array */
struct MixedData global_structs[256];

/* Secondary computation in separate function */
extern void secondary_computation(int start, int end, int* result);

/* Main computation with nested loops */
__attribute__((noinline)) 
int core_computation(int N, int M, int K, int limit) {
    int total = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int local_arr[128];
    
    /* Initialize local array */
    for (int i = 0; i < 128; ++i) {
        local_arr[i] = pure_func(i);
    }
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < N; ++i) {
        int outer_temp = noinline_func1(i, volatile_seed);
        
        /* Middle loop with different data type */
        for (unsigned j = 0; j < (unsigned)M; ++j) {
            short short_acc = 0;
            
            /* Memory operations with potential aliasing */
            global_arr_int[i] = outer_temp + j;
            global_arr_unsigned[j] = (unsigned)outer_temp * j;
            
            /* Conditional inner loop execution */
            if (__builtin_expect(i > limit, 0)) {
                /* Innermost loop with mixed operations */
                for (int k = 0; k < K; ++k) {
                    /* Arithmetic operations with different types */
                    int temp = i * k + j;
                    unsigned utemp = (unsigned)temp * 7;
                    short stemp = (short)(temp % 256);
                    
                    /* Loop-carried dependency */
                    reg_acc = global_arr_int[k % 1024] + reg_acc;
                    
                    /* Conditional branch inside innermost loop */
                    if (i % 8 == 0) {
                        utemp = noinline_func2(utemp, (unsigned)k);
                        /* Optimization barrier */
                        asm volatile ("" ::: "memory");
                    }
                    
                    /* Memory access with struct */
                    global_structs[k % 256].a = temp;
                    global_structs[k % 256].b = stemp;
                    
                    /* Mixed computations */
                    short_acc += (short)((temp ^ utemp) & 0xFF);
                    
                    /* Another conditional with different modulus */
                    if (k % 5 == 0) {
                        total += pure_func(temp % 64);
                    }
                }
                
                /* Call to external function */
                int sec_result;
                secondary_computation(i, i + j, &sec_result);
                total += sec_result;
            }
            
            /* More arithmetic with register variable */
            reg_acc = reg_acc * 3 + local_arr[j % 128];
            
            /* Conditional based on j */
            if (j % 3 == 0) {
                total += noinline_func1(reg_acc, (int)j);
            } else {
                total += (int)noinline_func2((unsigned)reg_acc, (unsigned)j);
            }
        }
        
        /* Update total with complex expression */
        total = (total * 17 + reg_acc) % 1000000;
        
        /* Another optimization barrier */
        asm volatile ("" ::: "memory");
    }
    
    return total;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline))
void warm_up_computation(void) {
    int dummy = 0;
    /* Simple warm-up loop */
    for (int i = 0; i < 100; ++i) {
        dummy += pure_func(i);
        asm volatile ("" ::: "memory");
    }
    /* Prevent optimization */
    if (dummy == 0) {
        volatile_seed = dummy;
    }
}

/* Initialize data with pseudo-random values using LCG */
void initialize_data(void) {
    unsigned lcg = 123456789;
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr_int[i] = (int)(lcg % 1000);
        global_arr_unsigned[i] = lcg % 2000;
    }
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr_short[i] = (short)(lcg % 500);
    }
    for (int i = 0; i < 256; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_structs[i].a = (int)(lcg % 100);
        global_structs[i].b = (short)(lcg % 50);
        global_structs[i].c = lcg % 300;
        global_structs[i].d = (char)(lcg % 26);
    }
}

int main(int argc, char* argv[]) {
    /* Use command line args for variability */
    int N = (argc > 1) ? atoi(argv[1]) : 50;
    int M = (argc > 2) ? atoi(argv[2]) : 40;
    int K = (argc > 3) ? atoi(argv[3]) : 30;
    int limit = (argc > 4) ? atoi(argv[4]) : 25;
    
    /* Ensure reasonable bounds */
    if (N <= 0) N = 50;
    if (M <= 0) M = 40;
    if (K <= 0) K = 30;
    if (limit < 0) limit = 25;
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up to potentially trigger different compilation paths */
    warm_up_computation();
    
    /* Main computation with nested loops */
    int result = core_computation(N, M, K, limit);
    
    /* Additional verification computation */
    int checksum = compute_checksum(global_arr_int, 1024);
    
    /* Print results */
    printf("Main result: %d\n", result);
    printf("Checksum: %d\n", checksum);
    printf("Volatile seed: %d\n", volatile_seed);
    
    return 0;
}
