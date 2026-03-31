#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
static volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_compute(int a, int b) {
    return (a * b) ^ (a + b);
}

__attribute__((noinline, const)) int pure_multiply(int a, int b) {
    return a * b;
}

__attribute__((noinline)) void memory_barrier(void) {
    asm volatile ("" ::: "memory");
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int process_chunk(int start, int end, int* arr) {
    int acc = 0;
    register int i asm ("r12") = start;  /* Hint at register allocation */
    
    for (; i < end; ++i) {
        /* Mix of arithmetic operations */
        int val = arr[i];
        if (__builtin_expect((val & 1) == 0, 1)) {
            acc += val * 3;
        } else {
            acc -= val / 2;
        }
        
        /* Call pure function with loop-variant arguments */
        acc += pure_multiply(i, val % 16);
        
        /* Memory barrier to create scheduling boundary */
        if (i % 8 == 0) {
            memory_barrier();
        }
    }
    return acc;
}

/* Core computation with nested loops */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K) {
    int total = 0;
    unsigned outer_counter;
    short inner_counter;
    
    /* Outer loop with varying data types */
    for (outer_counter = 0; outer_counter < (unsigned)N; ++outer_counter) {
        int temp = global_array[outer_counter % 1024];
        
        /* Conditional inner loop execution */
        if (__builtin_expect(outer_counter > (unsigned)K, 0)) {
            /* Inner loop with different data type */
            for (inner_counter = 0; inner_counter < (short)M; ++inner_counter) {
                /* Complex expression with multiple uses of same variable */
                int idx = (outer_counter * 31 + inner_counter * 17) % 2048;
                short val = global_short_array[idx];
                
                /* Loop-carried dependency */
                total += val * temp;
                
                /* Conditional with arithmetic */
                if (outer_counter % 3 == 0) {
                    total -= noinline_compute(val, inner_counter);
                } else if (outer_counter % 7 == 0) {
                    total += pure_multiply(val, outer_counter);
                }
                
                /* Another memory barrier */
                asm volatile ("" ::: "memory");
                
                /* Multiple operations on same variable */
                temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                if (temp % 19 == 0) {
                    total ^= temp;
                }
            }
        } else {
            /* Different path with its own loop */
            for (int j = 0; j < M / 2; ++j) {
                total += global_array[(outer_counter + j) % 1024];
                total = (total << 3) | (total >> 29);  /* Rotate */
            }
        }
        
        /* Function call with side effects */
        if (outer_counter % 5 == 0) {
            total += process_chunk(0, 16, global_array);
        }
    }
    
    return total;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warm_up(void) {
    int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += i * 2;
        if (i % 10 == 0) {
            dummy ^= noinline_compute(i, i + 1);
        }
    }
    /* Use dummy to prevent optimization */
    volatile int* volatile_ptr = &volatile_seed;
    *volatile_ptr += dummy;
}

/* Initialize arrays with pseudo-random values */
void init_arrays(void) {
    int lcg = 123456789;
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_array[i] = (lcg >> 16) & 0x7FFF;
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_short_array[i] = (short)(lcg & 0xFFFF);
    }
}

int main(int argc, char* argv[]) {
    /* Use arguments to make loop bounds non-constant */
    int N = (argc > 1) ? atoi(argv[1]) : 100;
    int M = (argc > 2) ? atoi(argv[2]) : 50;
    int K = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Ensure bounds are reasonable */
    if (N <= 0) N = 100;
    if (M <= 0) M = 50;
    if (K <= 0) K = 25;
    
    /* Initialize data */
    init_arrays();
    
    /* Warm up to potentially trigger different compilation paths */
    warm_up();
    
    /* Main computation with nested loops */
    int result = nested_loop_computation(N, M, K);
    
    /* Additional loop with different pattern */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 10; ++j) {
            checksum ^= global_array[(i * j) % 1024];
            checksum += pure_multiply(i, j);
            
            /* Variable with limited scope */
            {
                int local_var = i * j;
                if (local_var % 11 == 0) {
                    checksum -= noinline_compute(local_var, checksum);
                }
            }
        }
    }
    
    /* Combine results */
    result ^= checksum;
    
    printf("Result: %d\n", result);
    return 0;
}
