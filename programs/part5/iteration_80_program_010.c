/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern volatile int volatile_seed;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func(int x, int y) {
    return x * y + (x ^ y);
}

__attribute__((noinline, const)) int pure_func(int x) {
    return (x * 3) / 2;
}

/* Helper function with loop-carried dependency */
__attribute__((noinline)) int compute_checksum(int* data, int size) {
    int acc = 0;
    for (int i = 0; i < size; ++i) {
        acc = data[i] + acc * 31;
    }
    return acc;
}

/* Function with nested loops and varied operations */
__attribute__((noinline)) int nested_loop_computation(int N, int M, int K, 
                                                     short* sdata, 
                                                     unsigned* udata) {
    int result = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    
    /* Outer loop with volatile dependency to prevent optimization */
    for (int i = 0; i < N + (volatile_seed & 1); ++i) {
        int temp = i * 2;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > K, 0)) {
            /* First inner loop with short type */
            for (short j = 0; j < M; ++j) {
                /* Mix of arithmetic operations */
                int val = sdata[j] * 3;
                val += pure_func(j);  /* Pure function call */
                
                /* Memory operation with potential aliasing */
                global_array[j % 1024] = val;
                
                /* Loop-carried dependency */
                reg_acc = reg_acc + val;
                
                /* Optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Conditional branch */
                if (j % 7 == 0) {
                    result += noinline_func(val, i);
                }
            }
        }
        
        /* Second inner loop with unsigned type */
        for (unsigned u = 0; u < (unsigned)M; u += 2) {
            /* Different data type operations */
            unsigned prod = udata[u] * udata[u + 1];
            
            /* Complex expression with multiple uses */
            int comp = (prod >> 3) + (prod & 0xFF);
            comp = comp * i - reg_acc;
            
            /* Another optimization barrier */
            asm volatile ("" ::: "memory");
            
            /* Nested conditional */
            if (u % 5 == 0) {
                result += comp;
            } else if (u % 3 == 0) {
                result -= comp / 2;
            }
            
            /* Function call with loop-variant arguments */
            temp += pure_func(comp);
        }
        
        /* Variable with different scope */
        {
            int local_var = temp * i;
            result += local_var;
            
            /* Memory operation to global array */
            global_array[i % 1024] = result;
        }
    }
    
    return result;
}

/* Warm-up function to trigger different compilation paths */
__attribute__((noinline)) void warmup_computation(void) {
    int dummy = 0;
    for (int i = 0; i < 100; ++i) {
        dummy += i * 2;
        asm volatile ("" ::: "memory");
    }
    /* Use dummy to prevent optimization */
    global_array[0] = dummy;
}

/* Main computation with complex loop structures */
__attribute__((noinline)) int main_computation(int seed) {
    int checksum = seed;
    
    /* Initialize local arrays */
    short sdata[256];
    unsigned udata[512];
    
    /* Fill with pseudo-random values using LCG */
    int lcg = seed;
    for (int i = 0; i < 256; ++i) {
        lcg = lcg * 1103515245 + 12345;
        sdata[i] = (short)(lcg & 0xFFFF);
    }
    
    for (int i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        udata[i] = (unsigned)lcg;
    }
    
    /* Triple nested loops with varying trip counts */
    for (int outer = 0; outer < 10; ++outer) {
        int N = 20 + (outer % 3);
        int M = 15 + (outer % 5);
        int K = 5 + (outer % 2);
        
        /* Call nested loop computation */
        checksum += nested_loop_computation(N, M, K, sdata, udata);
        
        /* Additional loop with different characteristics */
        for (int i = 0; i < N; ++i) {
            int acc = 0;
            
            /* Unrolled-like computation */
            for (int j = 0; j < 4; ++j) {
                acc += sdata[(i + j) % 256] * udata[(i * j) % 512];
                
                /* Conditional with __builtin_expect */
                if (__builtin_expect((i + j) % 8 == 0, 0)) {
                    acc = acc >> 1;
                }
            }
            
            checksum ^= acc;  /* Non-linear combination */
            
            /* Memory barrier */
            asm volatile ("" ::: "memory");
        }
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    /* Initialize seed from arguments or volatile */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize global array */
    for (int i = 0; i < 1024; ++i) {
        global_array[i] = i * 3;
    }
    
    /* Warm-up to potentially trigger different compilation paths */
    warmup_computation();
    
    /* Main computation with nested loops */
    int result = main_computation(seed);
    
    /* Compute final checksum */
    int final_checksum = compute_checksum(global_array, 1024);
    final_checksum ^= result;
    
    printf("Result: %d\n", result);
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
