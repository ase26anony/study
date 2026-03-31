/* Selective Scheduling Dump Test Program
 * Designed to trigger dump_insn_rtx_1 in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-dump sel_sched_test.c helper.c -o sel_sched_test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External declarations for multi-file compilation stress */
extern int global_array[1024];
extern volatile int g_volatile_seed;
extern int helper_compute(int a, int b) __attribute__((noinline));

/* Non-inlineable functions to create scheduling boundaries */
int __attribute__((noinline)) noinline_multiply(int a, int b) {
    return a * b;
}

int __attribute__((noinline)) noinline_modulus(int a, int b) {
    return b != 0 ? a % b : 0;
}

/* Pure function for constant propagation testing */
int __attribute__((const)) pure_square(int x) {
    return x * x;
}

/* Global variables for aliasing stress */
int global_data[256];
int global_counter = 0;
volatile int volatile_trigger = 1;

/* Struct for complex memory access patterns */
struct DataPoint {
    int value;
    short tag;
    unsigned char flags;
    int *pointer;
};

/* Main computation with nested loops targeting selective scheduler */
unsigned long long __attribute__((noinline)) 
compute_checksum(int outer_limit, int inner_limit, int step) {
    unsigned long long checksum = 0;
    register int reg_acc = 0;  /* Hint for register allocation */
    int stack_acc = 0;
    short short_counter;
    unsigned unsigned_index;
    
    /* Outer loop with varying data types */
    for (int i = 0; i < outer_limit; ++i) {
        int loop_variant = i * step;
        
        /* Conditional inner loop execution */
        if (__builtin_expect(i > (outer_limit / 4), 0)) {
            /* First inner loop with different counter type */
            for (short_counter = 0; short_counter < (short)(inner_limit / 2); ++short_counter) {
                /* Loop-carried dependency */
                reg_acc = global_data[(i * 16 + short_counter) % 256] + reg_acc;
                
                /* Memory operation with potential aliasing */
                global_data[(short_counter * 7) % 256] ^= reg_acc;
                
                /* Non-inlineable function call */
                int temp = noinline_multiply(reg_acc, short_counter);
                
                /* Conditional branch creating control flow */
                if (i % 8 == 0) {
                    temp = noinline_modulus(temp, 17);
                }
                
                /* Pure function call with loop-variant arguments */
                checksum += pure_square(temp);
                
                /* Optimization barrier creating scheduling boundary */
                asm volatile ("" ::: "memory");
            }
        }
        
        /* Second inner loop with unsigned counter */
        for (unsigned_index = 0; unsigned_index < (unsigned)inner_limit; ++unsigned_index) {
            /* Different scope variable */
            int local_var = i * 3 + unsigned_index;
            
            /* Multiple uses of same variable */
            local_var = local_var * 2;
            stack_acc += local_var;
            local_var = stack_acc - local_var;
            
            /* Complex expression with mixed types */
            checksum += (unsigned long long)((local_var & 0xFF) * 
                         (unsigned_index % 32));
            
            /* Access external global array */
            if (global_array[(i + unsigned_index) % 1024] > 0) {
                checksum += 1;
            }
            
            /* Conditional based on volatile */
            if (volatile_trigger && (unsigned_index % 16 == 0)) {
                /* Another optimization barrier */
                asm volatile ("" ::: "memory");
                
                /* Helper function call from another file */
                checksum += helper_compute(i, unsigned_index);
            }
            
            /* Nested conditional loop */
            if (local_var % 64 == 0) {
                for (int k = 0; k < 3; ++k) {
                    checksum += (k * local_var) % 7;
                }
            }
        }
        
        /* Third loop with struct access */
        if (i % 3 == 0) {
            struct DataPoint dp;
            dp.value = i;
            dp.tag = (short)(i % 1000);
            dp.flags = (unsigned char)(checksum & 0xFF);
            
            for (int j = 0; j < 5; ++j) {
                dp.value += j;
                checksum += dp.value * dp.tag;
                
                /* Memory barrier between struct operations */
                asm volatile ("" ::: "memory");
            }
        }
    }
    
    return checksum + reg_acc + stack_acc;
}

/* Warm-up function to trigger different compilation paths */
void __attribute__((noinline)) warm_up_computation(void) {
    int warm_data[64];
    unsigned long long warm_sum = 0;
    
    /* Simple warm-up loop */
    for (int i = 0; i < 64; ++i) {
        warm_data[i] = i * 3;
    }
    
    for (int i = 0; i < 64; ++i) {
        for (int j = 0; j < 8; ++j) {
            warm_sum += warm_data[(i + j) % 64] * j;
        }
    }
    
    /* Use the result to prevent optimization */
    global_counter = (int)(warm_sum % 1000);
}

/* Simple LCG for pseudo-random initialization */
static unsigned int lcg_seed = 12345;
static unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return (unsigned int)(lgc_seed / 65536) % 32768;
}

/* Initialize data arrays */
void initialize_data(void) {
    for (int i = 0; i < 256; ++i) {
        global_data[i] = lcg_rand() % 1000;
    }
    
    /* Make some patterns for aliasing */
    for (int i = 0; i < 256; i += 8) {
        global_data[i] = i;
        global_data[i + 1] = global_data[i] * 2;
    }
}

int main(int argc, char *argv[]) {
    /* Use arguments for variability, preventing constant propagation */
    int outer_lim = argc > 1 ? atoi(argv[1]) : 50;
    int inner_lim = argc > 2 ? atoi(argv[2]) : 40;
    int step_val = argc > 3 ? atoi(argv[3]) : 3;
    
    /* Ensure reasonable bounds */
    if (outer_lim <= 0) outer_lim = 50;
    if (inner_lim <= 0) inner_lim = 40;
    if (step_val <= 0) step_val = 3;
    
    /* Limit sizes for reasonable runtime */
    if (outer_lim > 200) outer_lim = 200;
    if (inner_lim > 100) inner_lim = 100;
    
    printf("Starting selective scheduling test...\n");
    printf("Parameters: outer=%d, inner=%d, step=%d\n", 
           outer_lim, inner_lim, step_val);
    
    /* Initialize data */
    initialize_data();
    
    /* Warm-up execution */
    printf("Warm-up phase...\n");
    warm_up_computation();
    
    /* Main computation with nested loops */
    printf("Main computation phase...\n");
    unsigned long long result = compute_checksum(outer_lim, inner_lim, step_val);
    
    /* Additional verification computation */
    unsigned long long verify = 0;
    for (int i = 0; i < 10; ++i) {
        verify += global_data[i * 10] + global_counter;
    }
    
    printf("Result checksum: %llu\n", result);
    printf("Verification value: %llu\n", verify);
    printf("Final combined: %llu\n", result + verify);
    
    return 0;
}
