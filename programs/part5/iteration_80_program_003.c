/* sel_sched_test.c - Test program to trigger selective scheduling dump logic */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
volatile int g_volatile_seed = 42;
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Struct to create complex memory access patterns */
struct DataStruct {
    int a;
    int b;
    short c;
    unsigned d;
    char e[8];
};

struct DataStruct g_struct_array[256];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int non_inline_func1(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline)) unsigned non_inline_func2(unsigned a, unsigned b) {
    return (a + b) * (a - b);
}

__attribute__((noinline)) short non_inline_func3(short x, short y) {
    return (short)(x * 3 + y * 2);
}

/* Pure functions for const attribute testing */
__attribute__((const)) int pure_func1(int x) {
    return x * x - x + 1;
}

__attribute__((const)) unsigned pure_func2(unsigned x) {
    return (x << 3) | (x >> 5);
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* arr, int size) {
    register int acc = 0;  /* Hint register allocation */
    int i;
    
    for (i = 0; i < size; ++i) {
        /* Loop-carried dependency */
        acc = arr[i] + acc;
        
        /* Conditional with builtin expect */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            acc += non_inline_func1(acc, i);
        }
        
        /* Memory barrier to create scheduling boundary */
        asm volatile ("" ::: "memory");
    }
    return acc;
}

/* Complex nested loop structure */
__attribute__((noinline)) unsigned complex_nested_loops(int N, int M, int K) {
    unsigned total = 0;
    int i, j, k;
    volatile int vol_var = g_volatile_seed;  /* Prevent optimization */
    
    /* Outer loop with varying trip count */
    for (i = 0; i < N; ++i) {
        int local_acc = 0;
        
        /* First inner loop - always executed */
        for (j = 0; j < M; ++j) {
            /* Mixed data types */
            short s_val = (short)(i + j);
            unsigned u_val = (unsigned)(i * j);
            
            /* Memory operations with potential aliasing */
            g_array1[j] = i + j;
            g_short_array[i * 2 + j] = s_val;
            
            /* Function calls with different types */
            local_acc += non_inline_func1(i, j);
            u_val = non_inline_func2(u_val, (unsigned)j);
            
            /* Conditional branch */
            if (i % K == 0) {
                local_acc += pure_func1(j);
                asm volatile ("" ::: "memory");  /* Scheduling boundary */
            }
            
            /* Struct access */
            g_struct_array[j % 256].a = i;
            g_struct_array[j % 256].b = j;
            g_struct_array[j % 256].c = s_val;
            
            /* Use register variable */
            register int reg_temp = local_acc * 2;
            total += (unsigned)reg_temp;
        }
        
        /* Conditionally executed second inner loop */
        if (i > (N / 2)) {
            unsigned inner_total = 0;
            
            for (k = 0; k < (M / 2); ++k) {
                /* Different computation pattern */
                int temp = g_array1[k] + g_array2[k];
                
                /* Nested condition */
                if (__builtin_expect((k & 0x7) == 0, 1)) {
                    temp = non_inline_func3((short)temp, (short)k);
                    inner_total += pure_func2((unsigned)temp);
                } else {
                    inner_total += (unsigned)temp * 3;
                }
                
                /* Array access with different stride */
                g_unsigned_array[k % 512] = inner_total;
                
                /* Another memory barrier */
                asm volatile ("" ::: "memory");
            }
            
            total += inner_total;
        }
        
        /* Loop-carried dependency across outer loop */
        g_array2[i % 1024] = local_acc;
    }
    
    return total;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warm_up_computation(void) {
    int i, j;
    volatile int dummy = 0;
    
    /* Simple warm-up loop */
    for (i = 0; i < 100; ++i) {
        for (j = 0; j < 50; ++j) {
            dummy += i * j;
            if (j % 10 == 0) {
                dummy += non_inline_func1(i, j);
            }
        }
    }
    
    /* Prevent optimization */
    g_volatile_seed = dummy;
}

/* Initialize data with pseudo-random values */
void initialize_data(void) {
    int i;
    unsigned lcg = 123456789;  /* Simple LCG seed */
    
    for (i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_array1[i] = (int)(lcg % 1000);
        g_array2[i] = (int)(lcg % 1000);
    }
    
    for (i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_short_array[i] = (short)(lcg % 1000);
    }
    
    for (i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_unsigned_array[i] = lcg;
    }
    
    for (i = 0; i < 256; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_struct_array[i].a = (int)(lcg % 1000);
        g_struct_array[i].b = (int)(lcg % 1000);
        g_struct_array[i].c = (short)(lcg % 1000);
        g_struct_array[i].d = lcg;
    }
}

int main(int argc, char* argv[]) {
    unsigned final_result;
    int checksum1, checksum2;
    
    /* Use command line args for variability */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int M = (argc > 2) ? atoi(argv[2]) : 50;
    int K = (argc > 3) ? atoi(argv[3]) : 7;
    
    /* Ensure non-zero, reasonable bounds */
    if (N <= 0) N = 100;
    if (M <= 0) M = 50;
    if (K <= 0) K = 7;
    
    printf("Starting selective scheduling test with N=%d, M=%d, K=%d\n", N, M, K);
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up computation */
    printf("Performing warm-up...\n");
    warm_up_computation();
    
    /* Main computation with nested loops */
    printf("Running main computation...\n");
    final_result = complex_nested_loops(N, M, K);
    
    /* Additional checksum computations */
    checksum1 = compute_checksum(g_array1, 1024);
    checksum2 = compute_checksum(g_array2, 1024);
    
    /* Print verifiable results */
    printf("Final result: %u\n", final_result);
    printf("Checksum1: %d\n", checksum1);
    printf("Checksum2: %d\n", checksum2);
    printf("Total: %lu\n", (unsigned long)final_result + checksum1 + checksum2);
    
    return 0;
}
