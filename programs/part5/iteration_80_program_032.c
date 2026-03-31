/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array[1024];
extern short global_short_array[2048];
static unsigned int static_array[512];

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int external_compute(int a, int b);
__attribute__((noinline)) void memory_barrier(void);
__attribute__((noinline)) short process_short(short val, int modifier);

/* Pure function for loop-invariant computation */
__attribute__((const)) int pure_transform(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

/* Non-inlineable function implementation */
__attribute__((noinline)) int external_compute(int a, int b) {
    volatile int result = a ^ b;  /* Prevent optimization */
    return result + (a & b);
}

__attribute__((noinline)) void memory_barrier(void) {
    /* Use asm volatile to create scheduling boundary */
    asm volatile("" ::: "memory");
}

__attribute__((noinline)) short process_short(short val, int modifier) {
    return (short)(val + (modifier % 256));
}

/* Helper function with mixed operations */
static int __attribute__((noinline)) 
complex_loop_core(int start, int limit, int step, volatile int* counter) {
    int acc = 0;
    unsigned int uacc = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    
    /* Outer loop with varying trip count */
    for (int i = start; __builtin_expect(i < limit, 1); i += step) {
        int temp = i * i;
        
        /* Conditional inner loop execution */
        if (i > (limit / 4)) {
            /* Inner loop with different data type */
            for (unsigned short j = 0; j < (unsigned short)(i % 256 + 1); ++j) {
                /* Loop-carried dependency */
                acc = global_array[(i + j) % 1024] + acc;
                
                /* Memory operation with potential aliasing */
                uacc += static_array[(i * j) % 512];
                
                /* Function call with loop-variant arguments */
                short sval = process_short((short)j, i);
                global_short_array[(i + j) % 2048] = sval;
                
                /* Scheduling barrier */
                if (j % 8 == 0) {
                    asm volatile("" ::: "memory");
                }
            }
        }
        
        /* Another conditional inner loop */
        if (i % 3 == 0) {
            for (int k = 0; k < (i % 16 + 1); ++k) {
                /* Mixed operations creating varied RTL */
                reg_acc += pure_transform(i + k);
                
                /* External function call */
                int ext_result = external_compute(i, k);
                acc ^= ext_result;
                
                /* Memory access pattern */
                uacc |= global_array[(i * k) % 1024];
            }
        }
        
        /* Update volatile counter to prevent optimization */
        *counter += (acc & 1);
        
        /* Branch with prediction hint */
        if (__builtin_expect((i & 0xFF) == 0, 0)) {
            memory_barrier();
            reg_acc = external_compute(reg_acc, uacc);
        }
    }
    
    return acc + uacc + reg_acc;
}

/* Main computation function with nested loops */
int __attribute__((noinline))
compute_checksum(int seed, int outer_iter, int inner_base) {
    int total = 0;
    volatile int safety_counter = 0;
    
    /* Triple nested loop structure */
    for (int outer = 0; outer < outer_iter; ++outer) {
        int outer_mod = outer % 64;
        
        for (int mid = outer_mod; mid < inner_base + outer_mod; ++mid) {
            /* Variable scope inside middle loop */
            int mid_acc = 0;
            unsigned short us_mid = (unsigned short)mid;
            
            /* Innermost loop with register variable */
            register int inner_reg = 0;
            for (register int inner = 0; inner < (mid % 32 + 4); ++inner) {
                /* Complex expression with multiple operations */
                int idx = (outer * 997 + mid * 31 + inner) % 1024;
                int val = global_array[idx];
                
                /* Multiple uses of same variable */
                mid_acc = (mid_acc * 3 + val) / 2;
                inner_reg ^= val;
                
                /* Conditional with arithmetic */
                if ((inner + outer) % 7 == 0) {
                    mid_acc = external_compute(mid_acc, inner_reg);
                    asm volatile("" ::: "memory");
                }
                
                /* Short type operations */
                short s1 = global_short_array[(idx * 2) % 2048];
                short s2 = process_short(s1, inner);
                global_short_array[(idx * 3) % 2048] = s2;
            }
            
            total += mid_acc + inner_reg;
            safety_counter += (mid_acc > 0);
        }
        
        /* Call complex loop core with varying parameters */
        if (outer % 2 == 0) {
            int core_result = complex_loop_core(
                outer, 
                outer + (inner_base % 16) + 8,
                1 + (outer % 3),
                &safety_counter
            );
            total ^= core_result;
        }
    }
    
    /* Use safety_counter to prevent dead code elimination */
    if (safety_counter == 0) {
        asm volatile("" ::: "memory");
    }
    
    return total;
}

/* Warm-up function */
void __attribute__((noinline))
warmup_computation(int iterations) {
    int dummy = 0;
    for (int i = 0; i < iterations; ++i) {
        /* Simple warm-up loop */
        dummy += i * i;
        if (i % 100 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Prevent optimization */
    if (dummy == 0) {
        volatile int* p = &dummy;
        *p = 1;
    }
}

/* Initialize arrays with pseudo-random data */
void initialize_arrays(int seed) {
    unsigned int lcg = seed;
    for (int i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_array[i] = (int)(lcg % 1000);
    }
    
    for (int i = 0; i < 2048; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_short_array[i] = (short)(lcg % 1000);
    }
    
    for (int i = 0; i < 512; ++i) {
        lcg = lcg * 1103515245 + 12345;
        static_array[i] = lcg;
    }
}

int main(int argc, char* argv[]) {
    /* Use arguments to vary loop bounds */
    int outer_iter = (argc > 1) ? atoi(argv[1]) : 50;
    int inner_base = (argc > 2) ? atoi(argv[2]) : 20;
    int seed = (argc > 3) ? atoi(argv[3]) : 42;
    
    if (outer_iter <= 0) outer_iter = 50;
    if (inner_base <= 0) inner_base = 20;
    
    /* Initialize data */
    initialize_arrays(seed);
    
    /* Warm-up execution */
    warmup_computation(100);
    
    /* Main computation */
    int checksum = compute_checksum(seed, outer_iter, inner_base);
    
    /* Print verifiable result */
    printf("Computed checksum: %d\n", checksum);
    printf("Seed: %d, Outer: %d, Inner: %d\n", seed, outer_iter, inner_base);
    
    return 0;
}
