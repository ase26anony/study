/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 42;
volatile long global_array[256] = {0};
static volatile int static_counter = 0;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) compute_offset(int idx, int scale) {
    __asm__ volatile ("" : "+r" (idx), "+r" (scale));
    return idx * scale;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int seed = (argc > 2) ? atoi(argv[2]) : 12345;
    
    /* Volatile sink to prevent optimization */
    volatile int sink = 0;
    volatile long addr_sink = 0;
    
    /* Outer loop to create many virtual registers */
    for (int outer = 0; outer < loop_count; outer++) {
        /* Create many short-lived intermediate values */
        /* Constants that are rematerialization candidates */
        const int const_small = 17;          /* Small integer constant */
        const long const_large = 0x7FFFFFFF; /* Large constant requiring multiple insns */
        const int const_derived = global_base + 1; /* Derived from global */
        
        /* Address expressions - good remat candidates */
        long *addr1 = (long*)&global_array[0];
        long *addr2 = (long*)&global_array[const_small];
        long *addr3 = (long*)&global_array[outer & 0xFF];
        
        /* Values derived from function arguments */
        int arg_derived1 = seed + const_small;
        int arg_derived2 = seed << 2;
        int arg_derived3 = (seed * 3) + outer;
        
        /* More complex expressions with different modes */
        long long_val1 = (long)arg_derived1 * const_large;
        long long_val2 = (long)arg_derived2 << 3;
        int int_val1 = arg_derived3 & 0xFF;
        int int_val2 = arg_derived3 | 0x80;
        
        /* First basic block with computations */
        if (outer & 1) {
            /* Branch 1: More computations creating register pressure */
            int temp1 = int_val1 + const_derived;
            long temp2 = long_val1 - (long)temp1;
            int temp3 = compute_offset(temp1, 4);
            
            /* Use addresses */
            addr_sink = (long)addr1 + temp2;
            sink = temp3;
            
            /* More values to keep alive */
            int temp4 = arg_derived1 * 7;
            long temp5 = (long)temp4 + (long)addr2 - (long)addr1;
            int temp6 = temp4 ^ temp1;
            
            /* Call to clobber registers */
            clobber_registers();
            
            /* Use all values after clobber */
            sink += temp6;
            addr_sink += temp5;
            
            /* Another set of computations */
            int temp7 = int_val2 * 3;
            long temp8 = long_val2 >> 2;
            int temp9 = temp7 - temp6;
            
            /* Store to global to prevent elimination */
            global_array[outer & 0xFF] = temp8 + temp9;
        } else {
            /* Branch 2: Different computations but same register pressure */
            int temp1 = int_val2 - const_derived;
            long temp2 = long_val2 + (long)temp1;
            int temp3 = compute_offset(temp1, 8);
            
            /* Different address computation */
            addr_sink = (long)addr3 - temp2;
            sink = temp3 * 2;
            
            /* More intermediate values */
            int temp4 = arg_derived2 / 3;
            long temp5 = (long)temp4 * (long)addr1;
            int temp6 = temp4 | temp1;
            
            /* Clobber registers */
            clobber_registers();
            
            /* Use values */
            sink -= temp6;
            addr_sink ^= temp5;
            
            /* Additional computations */
            int temp7 = int_val1 << 1;
            long temp8 = long_val1 & 0xFFFF;
            int temp9 = temp7 + temp6;
            
            /* Store result */
            global_array[(outer + 1) & 0xFF] = temp8 * temp9;
        }
        
        /* Third basic block (common tail) */
        {
            /* More computations to increase live ranges */
            int final1 = sink + static_counter;
            long final2 = addr_sink * 2;
            int final3 = final1 ^ (final2 & 0xFF);
            
            /* Use pointer arithmetic with different modes */
            char *char_ptr = (char*)addr1 + final3;
            int *int_ptr = (int*)addr2 + (final1 >> 2);
            
            /* Final clobber */
            clobber_registers();
            
            /* Use pointers to prevent elimination */
            sink = *char_ptr + *int_ptr;
            static_counter += final1;
        }
        
        /* Nested conditional to create more basic blocks */
        if (outer % 3 == 0) {
            /* Block with many constants */
            int c1 = 255;
            int c2 = 4096;
            int c3 = 0xABCD;
            long c4 = 0x123456789ABCDEF0LL;
            
            /* Use them in computations */
            int mix1 = c1 * sink;
            long mix2 = c4 / (c2 + 1);
            int mix3 = c3 & mix1;
            
            /* Force register usage */
            __asm__ volatile (""
                : "+r" (mix1), "+r" (mix2), "+r" (mix3)
                : "r" (c1), "r" (c2), "r" (c3), "r" (c4));
            
            sink = mix1 + mix2 + mix3;
        } else if (outer % 3 == 1) {
            /* Another block with different operations */
            double fp_val = (double)sink / 3.14159;
            int int_from_fp = (int)fp_val;
            long long_from_fp = (long)fp_val * 1000;
            
            /* Mix types to get different REG modes */
            __asm__ volatile (""
                : "+r" (int_from_fp), "+r" (long_from_fp)
                : "r" (fp_val));
            
            sink = int_from_fp ^ (long_from_fp & 0x7FFF);
        }
        
        /* Final computation using all types of values */
        int result = (sink + static_counter) & 0xFF;
        long address_result = (long)&global_array[result] + outer;
        
        /* Prevent everything from being optimized away */
        __asm__ volatile (""
            : "+r" (result), "+r" (address_result)
            : "r" (sink), "r" (static_counter));
    }
    
    return sink & 0xFF;
}
