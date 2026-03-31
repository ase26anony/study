/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* External declarations for multi-file compilation stress */
extern int helper_pure(int a, int b) __attribute__((const));
extern void helper_noinline(int *arr, int idx) __attribute__((noinline));
extern volatile int g_volatile_seed;

/* Global arrays for memory operations with potential aliasing */
int g_array1[1024];
int g_array2[1024];
short g_short_array[2048];
unsigned g_unsigned_array[512];

/* Non-inlineable function to create scheduling boundaries */
__attribute__((noinline)) 
int compute_value(int a, int b, int c) {
    /* Complex enough to not be inlined */
    int result = (a * b) + (c << 3);
    result ^= (result >> 16);
    result += (a % 17);
    return result;
}

/* Pure function for predictable but non-trivial computations */
__attribute__((const))
int pure_transform(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

/* Warm-up function to trigger different compilation paths */
void warm_up_computation(int limit) {
    register int i;
    int local_acc = 0;
    
    for (i = 0; i < limit; ++i) {
        /* Mix of operations with register variable */
        local_acc += i * 3;
        if (i % 7 == 0) {
            local_acc -= i / 2;
        }
        /* Memory barrier to create scheduling boundary */
        asm volatile ("" ::: "memory");
    }
    
    /* Use the result to prevent optimization */
    g_array1[0] = local_acc;
}

/* Core computation with nested loops and varied patterns */
unsigned long long core_computation(int outer_limit, int inner_limit, 
                                   int conditional_limit) {
    unsigned long long total_checksum = 0;
    int i, j, k;
    unsigned u_counter;
    short s_counter;
    
    /* Outer loop with int counter */
    for (i = 0; i < outer_limit; ++i) {
        int outer_acc = 0;
        int *ptr1 = &g_array1[i % 1024];
        int *ptr2 = &g_array2[(i * 3) % 1024];
        
        /* First inner loop with unsigned counter */
        for (u_counter = 0; u_counter < (unsigned)inner_limit; ++u_counter) {
            /* Loop-carried dependency */
            outer_acc = *ptr1 + outer_acc;
            
            /* Memory operation with potential aliasing */
            *ptr2 = outer_acc ^ (int)u_counter;
            
            /* Conditional execution within inner loop */
            if (u_counter % 8 == 0) {
                /* Function call with loop-variant arguments */
                outer_acc = compute_value(outer_acc, i, (int)u_counter);
                
                /* Pure function call */
                outer_acc = pure_transform(outer_acc);
            }
            
            /* Access different data types */
            g_short_array[u_counter % 2048] = (short)(outer_acc & 0xFFFF);
            
            /* Optimization barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Conditional inner loop based on outer index */
        if (__builtin_expect(i > conditional_limit, 0)) {
            /* Second inner loop with short counter */
            for (s_counter = 0; s_counter < (short)(inner_limit / 2); ++s_counter) {
                int inner_acc = 0;
                
                /* Nested loop with different trip count */
                for (k = 0; k < 5; ++k) {
                    /* Complex expression with multiple uses of same variable */
                    inner_acc = (inner_acc * 3) + (s_counter * k);
                    inner_acc ^= g_unsigned_array[(i + k) % 512];
                    
                    /* Helper function from another file */
                    if (k % 3 == 0) {
                        helper_noinline(g_array1, inner_acc & 1023);
                    }
                }
                
                /* Mixed-type computation */
                total_checksum += (unsigned long long)inner_acc * 
                                 (unsigned long long)s_counter;
                
                /* Another memory barrier */
                asm volatile ("" ::: "memory");
            }
        } else {
            /* Different path with volatile dependency */
            int volatile_mod = g_volatile_seed % 64;
            
            for (j = 0; j < volatile_mod; ++j) {
                /* Use helper function */
                int temp = helper_pure(i, j);
                total_checksum += temp;
                
                /* Array access with complex index */
                g_array2[(i * j) % 1024] = temp;
            }
        }
        
        /* Update total checksum with outer loop result */
        total_checksum += (unsigned long long)outer_acc * 1000003ULL;
        
        /* Conditional branch with prediction hint */
        if (__builtin_expect(i % 13 == 0, 1)) {
            /* Additional computation on certain iterations */
            total_checksum ^= (total_checksum >> 32);
        }
    }
    
    return total_checksum;
}

/* Initialize arrays with pseudo-random values using LCG */
void initialize_data(int seed) {
    int lcg = seed;
    int i;
    
    for (i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_array1[i] = (lcg >> 16) & 0x7FFF;
        g_array2[i] = (lcg >> 8) & 0xFF;
    }
    
    for (i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_short_array[i] = (short)(lcg & 0xFFFF);
    }
    
    for (i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        g_unsigned_array[i] = (unsigned)lcg;
    }
}

int main(int argc, char *argv[]) {
    /* Use command line arguments for variability */
    int outer_lim = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_lim = (argc > 2) ? atoi(argv[2]) : 100;
    int cond_lim = (argc > 3) ? atoi(argv[3]) : 25;
    
    /* Initialize with time-based seed for variability */
    int seed = (argc > 4) ? atoi(argv[4]) : 12345;
    initialize_data(seed);
    
    /* Warm-up computation (executed once) */
    warm_up_computation(100);
    
    /* Main computation with nested loops */
    unsigned long long result = core_computation(outer_lim, inner_lim, cond_lim);
    
    /* Print verifiable result */
    printf("Computation checksum: %llu\n", result);
    
    /* Additional verification step */
    printf("Array1[0] = %d, Array2[0] = %d\n", g_array1[0], g_array2[0]);
    
    return 0;
}
