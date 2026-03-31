/* sel-sched-test.c - Main test file for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int external_array[256];
extern void init_external_data(void);

/* Non-inlineable functions */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline)) unsigned noinline_unsigned(unsigned a, unsigned b) {
    return (a + b) * (a - b);
}

/* Pure function for scheduling interest */
__attribute__((const)) int pure_func(int x) {
    return x * x - x + 1;
}

/* Global arrays for memory operations */
int global_arr[1024];
short short_arr[2048];
volatile int volatile_seed = 42;

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* arr, int n) {
    register int acc = 0;  /* Hint register allocation */
    int i;
    
    /* Loop with varying data types */
    for (i = 0; i < n; ++i) {
        if (__builtin_expect(i % 16 == 0, 0)) {
            acc += arr[i] * 2;
        } else {
            acc += arr[i];
        }
        
        /* Memory barrier to create scheduling boundary */
        asm volatile("" ::: "memory");
        
        /* Call pure function with loop-variant argument */
        acc += pure_func(i & 0xFF);
    }
    return acc;
}

/* Core computation with nested loops */
__attribute__((noinline)) int nested_loop_computation(int limit1, int limit2, int threshold) {
    int result = 0;
    unsigned outer_acc = 0;
    short inner_acc = 0;
    
    /* Outer loop with volatile-dependent bound */
    int outer_bound = limit1 + (volatile_seed & 0xF);
    
    for (int i = 0; i < outer_bound; ++i) {
        /* Conditional inner loop execution */
        if (i > threshold) {
            /* Inner loop with different counter type */
            unsigned j;
            for (j = 0; j < (unsigned)limit2; ++j) {
                /* Mixed arithmetic operations */
                int temp = global_arr[i * 32 + (j % 32)];
                short stemp = short_arr[j % 2048];
                
                /* Loop-carried dependency */
                inner_acc += stemp;
                
                /* Non-inlineable function call */
                temp += noinline_func(i, j);
                
                /* Conditional with builtin expect */
                if (__builtin_expect((j & 3) == 0, 1)) {
                    temp *= 2;
                }
                
                /* Memory operation with potential aliasing */
                global_arr[(i * 31 + j) % 1024] = temp;
                
                /* Another scheduling barrier */
                asm volatile("" ::: "memory");
                
                /* Call unsigned function */
                outer_acc += noinline_unsigned(i, j);
                
                /* Use result to prevent optimization */
                result += temp + inner_acc;
            }
        } else {
            /* Different path for scheduler */
            for (short k = 0; k < (short)(limit2 / 2); ++k) {
                result += pure_func(k) - noinline_func(i, k);
            }
        }
        
        /* Access external array */
        if (i < 256) {
            result += external_array[i];
        }
    }
    
    return result + outer_acc + inner_acc;
}

/* Warm-up function */
__attribute__((noinline)) void warm_up(void) {
    int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += noinline_func(i, i + 1);
        asm volatile("" ::: "memory");
    }
    /* Use dummy to prevent optimization */
    volatile int use_dummy = dummy;
    (void)use_dummy;
}

int main(int argc, char** argv) {
    /* Initialize data */
    int limit1 = (argc > 1) ? atoi(argv[1]) : 50;
    int limit2 = (argc > 2) ? atoi(argv[2]) : 100;
    int threshold = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Simple LCG for pseudo-random values */
    unsigned lcg = 123456789;
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_arr[i] = (int)(lcg % 1000);
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        short_arr[i] = (short)(lcg % 1000);
    }
    
    /* Initialize external data if linked */
    init_external_data();
    
    /* Warm-up execution */
    warm_up();
    
    /* Main computation */
    int checksum1 = compute_checksum(global_arr, 256);
    int result = nested_loop_computation(limit1, limit2, threshold);
    int checksum2 = compute_checksum(global_arr, 256);
    
    /* Verifiable output */
    printf("Result: %d\n", result);
    printf("Checksum1: %d, Checksum2: %d\n", checksum1, checksum2);
    printf("Difference: %d\n", checksum2 - checksum1);
    
    return 0;
}
