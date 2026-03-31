/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 1000;
volatile long global_array[256] = {0};
volatile char global_flags[256] = {0};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function with arguments */
static int __attribute__((noinline)) dummy_compute(int a, int b) {
    __asm__ volatile ("" : : "r"(a), "r"(b) : "memory");
    return 0;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    int param_base = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Volatile sink to prevent optimizations */
    volatile long result_sink = 0;
    volatile int flag_sink = 0;
    
    /* Outer loop to create pressure */
    for (int outer = 0; outer < 10; outer++) {
        /* Inner hot loop with many basic blocks */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            
            /* Small integer constants (require multiple instructions) */
            int const_small = 0x7FFFFFFF;  /* Large constant */
            long const_large = 0x123456789ABCDEF0LL;
            
            /* Constants derived from function arguments */
            int derived1 = param_base + 1;          /* param + 1 */
            int derived2 = param_base << 2;         /* param << 2 */
            int derived3 = (param_base * 3) / 2;    /* Complex derivation */
            
            /* Symbol addresses that can be recomputed */
            long *addr1 = (long*)&global_array[i % 256];
            char *addr2 = (char*)&global_flags[i % 256];
            
            /* Loop-variant constants */
            int loop_const = i * 4;
            long loop_const_large = (long)i * 1000;
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use constants in computations */
                int temp1 = const_small + derived1;
                int temp2 = derived2 - loop_const;
                long temp3 = const_large + loop_const_large;
                
                /* Pointer arithmetic with different modes */
                long *ptr1 = addr1 + derived3;
                char *ptr2 = addr2 + (derived1 & 0xFF);
                
                /* More computations mixing types */
                long mixed1 = (long)temp1 * (long)temp2;
                int mixed2 = (int)(temp3 >> 32) + derived3;
                
                /* Store to volatile to prevent elimination */
                result_sink = mixed1;
                flag_sink = mixed2;
                
                /* Call to clobber registers */
                clobber_registers();
                
                /* More computations after call */
                int revived1 = const_small - derived2;  /* Should be rematerialized */
                long revived2 = const_large ^ loop_const_large;
                
                /* Use revived values */
                result_sink += revived1 + revived2;
                
            } else {
                /* Alternative path with different computations */
                
                /* Different set of constants */
                int alt_const1 = 0x55555555;
                int alt_const2 = 0xAAAAAAAA;
                long alt_const3 = 0xF0F0F0F0F0F0F0F0LL;
                
                /* Complex derivations */
                int alt_derived1 = param_base * 5 + 7;
                int alt_derived2 = (param_base << 3) | 0xF;
                long alt_derived3 = (long)param_base * 1000000;
                
                /* More pointer arithmetic */
                long *alt_ptr1 = (long*)&global_array[(i + 1) % 256];
                char *alt_ptr2 = (char*)&global_flags[(i + 128) % 256];
                
                /* Computations using all values */
                int alt_temp1 = alt_const1 & alt_derived1;
                int alt_temp2 = alt_const2 | alt_derived2;
                long alt_temp3 = alt_const3 + alt_derived3;
                
                /* Mix with global */
                alt_temp1 += global_base;
                alt_temp3 -= (long)global_base * 100;
                
                /* Store results */
                result_sink = alt_temp3;
                flag_sink = alt_temp1 ^ alt_temp2;
                
                /* Function call with arguments - forces register pressure */
                dummy_compute(alt_temp1, alt_temp2);
                
                /* Recomputation of constants after call */
                int revived_alt1 = alt_const1 + alt_derived1;  /* Remat candidate */
                long revived_alt2 = alt_const3 - alt_derived3;
                
                /* Final use */
                result_sink += revived_alt1 * revived_alt2;
            }
            
            /* --- Basic Block 3: Common tail with more computations --- */
            
            /* More constants that might need rematerialization */
            int tail_const1 = 0x12345678;
            int tail_const2 = 0x9ABCDEF0;
            long tail_const3 = 0x1111111111111111LL;
            
            /* Use them in final computations */
            int final1 = tail_const1 + (i & 0xFF);
            int final2 = tail_const2 - (i >> 8);
            long final3 = tail_const3 * (long)(i + 1);
            
            /* Mix with parameter again */
            final1 += param_base;
            final2 -= param_base;
            final3 += (long)param_base << 32;
            
            /* Final store */
            result_sink = final1 + final2 + final3;
            
            /* Another register-clobbering call */
            clobber_registers();
            
            /* Force one more use of constants */
            if ((i % 16) == 0) {
                flag_sink = tail_const1 ^ tail_const2;
            }
        }
    }
    
    /* Use results to prevent complete optimization */
    printf("Result: %ld, Flag: %d\n", (long)result_sink, flag_sink);
    return 0;
}
