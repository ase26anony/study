/* sel_sched_test.c - Test program for selective scheduling dump coverage */
#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
static unsigned int static_array[512];

/* Volatile variables to prevent optimization */
volatile int vol_bound = 100;
volatile short vol_short = 50;

/* Non-inlineable functions */
__attribute__((noinline)) int non_inline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline)) unsigned int non_inline_unsigned(unsigned int a, unsigned int b) {
    return (a + b) * (a - b);
}

/* Pure functions for scheduling complexity */
__attribute__((const)) int pure_func(int x) {
    return (x * 3) / 2;
}

__attribute__((const)) short pure_short(short x) {
    return (short)(x + (x >> 2));
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int *arr, int n) {
    register int acc = 0;  /* Hint for register allocation */
    int i;
    
    /* Outer loop with varying trip count */
    for (i = 0; i < n; ++i) {
        int temp = arr[i];
        
        /* Inner loop executed conditionally */
        if (__builtin_expect((i & 3) == 0, 0)) {
            short j;
            for (j = 0; j < (short)(vol_short & 15); ++j) {
                temp += pure_short((short)j);
                asm volatile ("" ::: "memory");  /* Scheduling barrier */
            }
        }
        
        /* Loop-carried dependency */
        acc = temp + acc;
        
        /* Conditional with different data types */
        if (i % 7 == 0) {
            acc = non_inline_func(acc, i);
        } else if (i % 5 == 0) {
            unsigned int uacc = (unsigned int)acc;
            uacc = non_inline_unsigned(uacc, (unsigned int)i);
            acc = (int)uacc;
        }
    }
    return acc;
}

/* Complex nested loop structure */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K) {
    int result = 0;
    int i, j, k;
    
    /* Triple nested loops with different counter types */
    for (i = 0; i < N; ++i) {
        unsigned int ui = (unsigned int)i;
        
        /* First inner loop - always executed */
        for (j = 0; j < M; ++j) {
            short sj = (short)j;
            int temp = global_array[(ui * 16 + j) % 1024];
            
            /* Mix of operations */
            temp = pure_func(temp);
            temp += non_inline_func(i, j);
            
            /* Memory operation with potential aliasing */
            global_short_array[(sj * 2) % 2048] = (short)(temp & 0xFFFF);
            
            /* Conditional inner-inner loop */
            if (__builtin_expect(i > (vol_bound >> 1), 0)) {
                register int k;  /* Register hint */
                for (k = 0; k < K; ++k) {
                    /* Complex expression with multiple uses */
                    int val = static_array[k % 512];
                    val = val * 3 + (val >> 2);
                    
                    /* Function call with loop-variant arguments */
                    val = non_inline_func(val, k);
                    
                    /* Scheduling barrier */
                    asm volatile ("" ::: "memory");
                    
                    result += val;
                }
            }
            
            result += temp;
        }
        
        /* Second inner loop - conditionally executed */
        if (__builtin_expect((i % K) == 0, 1)) {
            unsigned int j;
            for (j = 0; j < (unsigned int)(M >> 1); ++j) {
                /* Different data type operations */
                unsigned int uval = ui * j;
                uval = non_inline_unsigned(uval, j);
                
                /* Access different global array */
                int idx = (int)((ui + j) % 512);
                static_array[idx] = (int)uval;
                
                result += (int)uval;
            }
        }
    }
    
    return result;
}

/* Warm-up function to trigger compilation paths */
__attribute__((noinline)) void warm_up(void) {
    int i;
    volatile int dummy = 0;
    
    /* Simple warm-up loop */
    for (i = 0; i < 100; ++i) {
        dummy += pure_func(i);
        asm volatile ("" ::: "memory");
    }
    
    /* Prevent unused variable warning */
    (void)dummy;
}

/* Initialize arrays with pseudo-random values */
void initialize_data(void) {
    unsigned int seed = 42;  /* Simple LCG */
    int i;
    
    for (i = 0; i < 1024; ++i) {
        seed = seed * 1103515245 + 12345;
        global_array[i] = (int)(seed >> 16) & 0x7FFF;
    }
    
    for (i = 0; i < 2048; ++i) {
        seed = seed * 1103515245 + 12345;
        global_short_array[i] = (short)(seed & 0xFFFF);
    }
    
    for (i = 0; i < 512; ++i) {
        seed = seed * 1103515245 + 12345;
        static_array[i] = (int)seed;
    }
}

int main(int argc, char *argv[]) {
    int N, M, K;
    int checksum1, checksum2, final_result;
    
    /* Use command line arguments for variability */
    if (argc >= 4) {
        N = atoi(argv[1]);
        M = atoi(argv[2]);
        K = atoi(argv[3]);
    } else {
        /* Default values that create interesting loop structures */
        N = vol_bound;
        M = (int)vol_short * 2;
        K = 8;
    }
    
    /* Initialize data */
    initialize_data();
    
    /* Warm up */
    warm_up();
    
    /* Main computation with nested loops */
    checksum1 = compute_checksum(global_array, N);
    checksum2 = nested_loop_computation(N, M, K);
    
    /* Combine results */
    final_result = checksum1 + checksum2;
    
    /* Print verifiable result */
    printf("Result: %d (Checksum1: %d, Checksum2: %d)\n", 
           final_result, checksum1, checksum2);
    
    return 0;
}
