/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 42;
volatile long global_array[256] = {0};
static const int const_table[8] = {1, 2, 4, 8, 16, 32, 64, 128};

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force register pressure */
static int __attribute__((noinline)) use_value(int x, long y) {
    volatile int sink;
    __asm__ volatile ("# dummy asm" : "=r"(sink) : "0"(x + (int)y));
    return sink;
}

/* Main function with complex loop structure */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int param_base = (argc > 2) ? atoi(argv[2]) : 12345;
    
    /* Volatile sink to prevent optimizations */
    volatile int result_sink = 0;
    volatile long addr_sink = 0;
    
    /* Outer loop to create many intermediate values */
    for (int outer = 0; outer < 10; outer++) {
        /* Hot inner loop with high register pressure */
        for (int i = 0; i < loop_count; i++) {
            /* --- Basic Block 1: Create many remat candidates --- */
            
            /* Small integer constants requiring multiple instructions */
            int const1 = 0x12345678;          /* Large constant */
            int const2 = 0x87654321;          /* Another large constant */
            long const3 = 0xFEDCBA9876543210LL; /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = param_base + 1;    /* param + 1 */
            int derived2 = param_base << 2;   /* param << 2 */
            int derived3 = param_base * 3;    /* param * 3 */
            int derived4 = param_base / 4;    /* param / 4 */
            
            /* Symbol addresses that can be recomputed */
            long *addr1 = &global_array[i % 256];
            long *addr2 = &global_array[(i + 1) % 256];
            const int *addr3 = &const_table[i % 8];
            
            /* Complex expressions mixing types */
            long mixed1 = (long)const1 * (long)derived1;
            int mixed2 = const2 + derived2;
            long mixed3 = const3 >> (i % 16);
            
            /* --- Basic Block 2: Conditional branch --- */
            if (i & 1) {
                /* Use values in one branch */
                int temp1 = mixed2 * 7;
                long temp2 = mixed1 + *addr1;
                int temp3 = derived3 | const1;
                
                /* More computations */
                temp1 += global_base;
                temp2 -= (long)temp3;
                temp3 ^= derived4;
                
                /* Store to volatile to prevent elimination */
                result_sink = temp1;
                addr_sink = (long)addr2;
                
                /* Call to clobber registers */
                clobber_registers();
                
                /* Use clobbered values again */
                temp1 = use_value(temp1, temp2);
                temp2 = temp2 + (long)temp3;
                
                result_sink += temp1;
                addr_sink += temp2;
            } else {
                /* Alternative branch with different computations */
                int temp4 = derived1 * derived2;
                long temp5 = (long)derived3 * mixed3;
                int temp6 = const2 & derived4;
                
                /* Pointer arithmetic */
                long *addr4 = addr1 + (i % 16);
                const int *addr5 = addr3 + (i % 4);
                
                /* More operations */
                temp4 = temp4 >> 1;
                temp5 = temp5 / 3;
                temp6 = temp6 << 2;
                
                /* Use global and addresses */
                temp4 += *addr5;
                temp5 += (long)*addr4;
                temp6 += global_base;
                
                /* Store results */
                result_sink = temp4;
                addr_sink = temp5;
                
                /* Clobber registers */
                clobber_registers();
                
                /* Recompute some values */
                int temp7 = derived1 + const1;
                long temp8 = (long)derived2 * const3;
                
                /* More mixing */
                temp4 = use_value(temp4, temp5);
                temp5 = temp5 + temp8;
                temp6 = temp6 ^ temp7;
                
                result_sink += temp4 + temp6;
                addr_sink += temp5;
            }
            
            /* --- Basic Block 3: Common code after branch --- */
            
            /* Create more intermediate values */
            int final1 = result_sink & 0xFF;
            long final2 = addr_sink | 0xFFFF;
            int final3 = (final1 * 3) + (final2 & 0xFF);
            
            /* More pointer computations */
            long *final_addr = &global_array[(i + outer) % 256];
            *final_addr = final2 + final3;
            
            /* Complex expression with many operands */
            int complex = ((final1 << 3) | (final3 >> 2)) + 
                         (derived1 & derived2) - 
                         (const1 % 17) + 
                         (global_base ^ i);
            
            /* Use in volatile asm to prevent optimization */
            __asm__ volatile ("# complex use %0" : : "r"(complex));
            
            /* Another register-clobbering call */
            clobber_registers();
            
            /* Final computations using all live values */
            result_sink = complex + final1 + final3;
            addr_sink = (long)final_addr + final2;
            
            /* Prevent loop unrolling */
            __asm__ volatile ("# loop barrier" : : : "memory");
        }
        
        /* Modify parameter to create different remat patterns */
        param_base += outer * 7;
    }
    
    /* Return result to prevent dead code elimination */
    return result_sink + (int)addr_sink;
}
