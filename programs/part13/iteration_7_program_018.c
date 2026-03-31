/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_counter = 0;
int global_array[256] = {0};
static const long global_const = 0x7FFFFFFFFFFFFFFFL;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", 
                      "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y, void *z) {
    __asm__ volatile ("# dummy asm" : : "r"(x), "r"(y), "r"(z));
    return x + (int)((intptr_t)z & 0xFF);
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    int iterations = argc > 2 ? atoi(argv[2]) : 1000000;
    
    /* Volatile sink to prevent optimizations */
    volatile int sink = 0;
    
    /* Outer loop to create many register pressures */
    for (int outer = 0; outer < 1000; outer++) {
        /* Inner loop with multiple basic blocks */
        for (int i = 0; i < iterations; i++) {
            /* --- BLOCK 1: Create many rematerialization candidates --- */
            
            /* Small integer constants (require multiple instructions) */
            int const1 = 0x12345678;  /* Large constant */
            long const2 = 0x7FFFFFFFFFFFFFFFL;  /* Max signed 64-bit */
            int const3 = -0x76543210;  /* Negative large constant */
            
            /* Constants derived from function arguments */
            int derived1 = base + 1;      /* param + 1 */
            int derived2 = base << 2;     /* param << 2 */
            int derived3 = (base * 3) / 2;
            
            /* Symbol addresses that can be recomputed */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            long *addr3 = (long *)&global_array[(i + 2) & 0xFF];
            
            /* Constants with different modes */
            short const_short = 0x7FFF;
            char const_char = 0x7F;
            uint64_t const_uint64 = 0xFFFFFFFFFFFFFFFFULL;
            
            /* --- BLOCK 2: Conditional branch creating new basic block --- */
            if (i & 1) {
                /* Use constants in arithmetic */
                int temp1 = const1 + derived1;
                long temp2 = const2 - (long)derived2;
                int temp3 = const3 * derived3;
                
                /* Pointer arithmetic */
                int *temp_addr1 = addr1 + (derived1 >> 2);
                int *temp_addr2 = addr2 - (derived2 >> 2);
                
                /* Mixed mode operations */
                long mixed1 = (long)const1 * const2;
                int64_t mixed2 = (int64_t)const_short * const_uint64;
                
                /* Force register pressure with many live values */
                sink += temp1 + (int)temp2 + temp3;
                sink += (int)(temp_addr1 - temp_addr2);
                sink += (int)(mixed1 >> 32) + (int)mixed2;
                
                /* Call to clobber registers */
                clobber_registers();
            } else {
                /* Alternative path with different computations */
                /* More rematerialization candidates */
                int alt_const1 = 0xABCDEF01;
                long alt_const2 = global_const;
                int alt_derived1 = base - 1;
                int alt_derived2 = base >> 1;
                
                /* Complex expressions */
                int alt1 = alt_const1 ^ alt_derived1;
                long alt2 = alt_const2 | (long)alt_derived2;
                int alt3 = (alt_const1 & 0xFFFF) * alt_derived2;
                
                /* More pointer computations */
                int *alt_addr1 = &global_array[(i * 3) & 0xFF];
                int *alt_addr2 = alt_addr1 + alt_derived1;
                
                /* Use in conditional expressions */
                int result = (alt1 > alt3) ? alt1 : alt3;
                long result2 = (alt2 < 0) ? -alt2 : alt2;
                
                sink += result + (int)result2;
                sink += (int)(alt_addr2 - alt_addr1);
                
                /* Another register-clobbering call */
                clobber_registers();
            }
            
            /* --- BLOCK 3: More computations after branch --- */
            
            /* Create more intermediate values */
            int post1 = const1 ^ const3;
            long post2 = const2 & 0xFFFFFFFF;
            int post3 = derived1 | derived2;
            
            /* More pointer arithmetic with different scales */
            char *byte_ptr = (char *)&global_array[0];
            byte_ptr += (i * sizeof(int)) & 0x3FF;
            
            short *short_ptr = (short *)byte_ptr;
            short_ptr += derived3 & 0x7F;
            
            /* Function call that uses multiple values */
            int func_result = use_value(post1, post2, short_ptr);
            
            /* Use results to prevent elimination */
            sink += func_result + post3;
            
            /* --- BLOCK 4: Nested conditional for more complexity --- */
            if ((i % 3) == 0) {
                /* Even more remat candidates */
                int nested_const = 0x33333333;
                long nested_derived = (long)base * 100;
                void *nested_addr = &&global_array[0] + (i * 16);
                
                /* Complex chain of computations */
                int chain1 = nested_const + (int)nested_derived;
                int chain2 = chain1 * 2;
                int chain3 = chain2 - nested_const;
                int chain4 = chain3 ^ chain1;
                
                sink += chain4 + (int)((intptr_t)nested_addr & 0xFF);
                
                /* Final clobber */
                clobber_registers();
            }
            
            /* Prevent loop elimination */
            if (sink > 1000000) {
                global_counter = sink;
                sink = 0;
            }
        }
    }
    
    return global_counter > 0 ? 0 : 1;
}
