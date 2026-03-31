/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Global arrays to create memory dependencies */
extern int global_array1[1024];
extern int global_array2[1024];
static volatile int volatile_seed = 42;

/* Non-inlineable functions to create scheduling boundaries */
__attribute__((noinline)) int noinline_func1(int x, int y) {
    return (x * y) ^ (x + y);
}

__attribute__((noinline)) int noinline_func2(int a, int b) {
    return (a << 3) | (b & 0xFF);
}

/* Pure function for loop computations */
__attribute__((const)) int pure_multiply(int a, int b) {
    return a * b;
}

/* Helper function with mixed operations */
__attribute__((noinline)) int complex_operation(int x, int y, int z) {
    int result = 0;
    if (x > y) {
        result = (x - y) * z;
    } else {
        result = (y - x) / (z ? z : 1);
    }
    return result ^ (x & y);
}

/* Secondary computation in separate loop */
static void inner_loop_computation(int start, int end, int *acc) {
    register int reg_acc = *acc;  /* Hint register allocation */
    unsigned short us_counter;
    int temp;
    
    for (us_counter = 0; us_counter < (end - start); ++us_counter) {
        int idx = start + us_counter;
        
        /* Memory operations with potential aliasing */
        temp = global_array1[idx % 1024] + global_array2[(idx * 7) % 1024];
        
        /* Loop-carried dependency */
        reg_acc = reg_acc ^ temp;
        
        /* Conditional execution */
        if (__builtin_expect((idx & 0x3F) == 0, 0)) {
            /* Function call creates scheduling boundary */
            reg_acc += noinline_func1(idx, reg_acc);
        }
        
        /* Optimization barrier */
        asm volatile ("" ::: "memory");
        
        /* Pure function call */
        reg_acc += pure_multiply(idx, idx % 256);
    }
    
    *acc = reg_acc;
}

/* Main computation with nested loops */
__attribute__((noinline)) int compute_checksum(int outer_limit, int inner_limit) {
    int checksum = 0;
    int i, j;
    unsigned u_counter;
    
    /* Warm-up loop (executed once) */
    for (u_counter = 0; u_counter < 10; ++u_counter) {
        checksum ^= u_counter * volatile_seed;
    }
    
    /* Outer loop with varying trip count */
    for (i = 0; i < outer_limit; ++i) {
        int local_acc = i * 7;
        short s_counter;
        
        /* First inner loop - always executed */
        for (s_counter = 0; s_counter < (inner_limit % 256); ++s_counter) {
            /* Mixed data types in computation */
            int idx = i * 256 + s_counter;
            
            /* Memory access pattern */
            int val1 = global_array1[idx % 1024];
            int val2 = global_array2[(idx * 13) % 1024];
            
            /* Arithmetic with different types */
            local_acc += (val1 * val2) / (i + 1);
            local_acc ^= (unsigned)val1 >> (s_counter & 0x7);
            
            /* Conditional branch */
            if (i % 4 == 0) {
                local_acc -= noinline_func2(val1, val2);
            } else if (i % 3 == 0) {
                local_acc += complex_operation(val1, val2, s_counter);
            }
            
            /* Another optimization barrier */
            asm volatile ("" ::: "memory");
        }
        
        /* Second inner loop - conditionally executed */
        if (__builtin_expect(i > (outer_limit / 2), 0)) {
            register int reg_temp = local_acc;
            
            for (j = 0; j < (i % 8); ++j) {
                /* Different computation pattern */
                reg_temp = (reg_temp << 1) | (reg_temp >> 31);
                reg_temp += global_array1[(i * j) % 1024];
                
                /* Function call with loop-variant arguments */
                if (j % 2 == 0) {
                    reg_temp ^= noinline_func1(reg_temp, j);
                }
            }
            
            local_acc = reg_temp;
        }
        
        /* Call to function with inner loop */
        if (i % 5 == 0) {
            inner_loop_computation(i * 16, i * 16 + 32, &local_acc);
        }
        
        checksum += local_acc;
        
        /* Variable scope stress */
        {
            int block_scoped = checksum & 0xFF;
            checksum ^= block_scoped * (i + 1);
        }
    }
    
    return checksum;
}

/* Initialize arrays with pseudo-random values */
void initialize_arrays(void) {
    int i;
    unsigned lcg = 123456789;
    
    for (i = 0; i < 1024; ++i) {
        lcg = lcg * 1103515245 + 12345;
        global_array1[i] = (lcg >> 16) & 0x7FFF;
        
        lcg = lcg * 1103515245 + 12345;
        global_array2[i] = (lcg >> 16) & 0x7FFF;
    }
}

int main(int argc, char *argv[]) {
    int outer_loop_count, inner_loop_count;
    int result;
    
    /* Use command line arguments for variability */
    if (argc > 2) {
        outer_loop_count = atoi(argv[1]);
        inner_loop_count = atoi(argv[2]);
    } else {
        /* Default values that create interesting scheduling regions */
        outer_loop_count = 50;
        inner_loop_count = 100;
    }
    
    /* Ensure loops aren't optimized away */
    if (outer_loop_count < 1) outer_loop_count = 1;
    if (inner_loop_count < 1) inner_loop_count = 1;
    
    printf("Initializing arrays...\n");
    initialize_arrays();
    
    printf("Running computation with outer=%d, inner=%d\n", 
           outer_loop_count, inner_loop_count);
    
    result = compute_checksum(outer_loop_count, inner_loop_count);
    
    printf("Result checksum: %d\n", result);
    printf("Volatile seed was: %d\n", volatile_seed);
    
    return 0;
}
