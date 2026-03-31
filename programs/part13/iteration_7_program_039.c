/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all -fno-omit-frame-pointer -fno-schedule-insns -fno-schedule-insns2 --param=max-early-remat-iterations=5 -o early_remat_test early-remat-trigger.c
 */

#include <stdint.h>
#include <stdlib.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 200;
int global_array[256] = {0};
static volatile int sink;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another dummy function to prevent optimization */
static void __attribute__((noinline)) use_value(int val) {
    sink = val;
}

/* Main function with complex loop to stress register allocation */
int main(int argc, char *argv[]) {
    /* Use arguments to create variant values */
    int loop_count = (argc > 1) ? atoi(argv[1]) : 1000;
    int param_base = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Volatile to prevent optimization */
    volatile int vol_counter = 0;
    
    /* Outer loop to create many intermediate values */
    for (int outer = 0; outer < 100; outer++) {
        /* Inner hot loop with high register pressure */
        for (int i = 0; i < loop_count; i++) {
            /* Create many rematerialization candidates - constants and addresses */
            
            /* Small integer constants (require multiple instructions on some arches) */
            int const1 = 0x12345678;  /* Large constant */
            int const2 = 0x87654321;  /* Another large constant */
            long const3 = 0xFFFFFFFF00000000UL;  /* 64-bit constant */
            
            /* Constants derived from function arguments */
            int derived1 = param_base + 1;      /* param + 1 */
            int derived2 = param_base << 2;     /* param << 2 */
            int derived3 = param_base * 3;      /* param * 3 */
            int derived4 = param_base | 0xFF;   /* param | 0xFF */
            
            /* Constants derived from loop counter */
            int loop_derived1 = i + 100;
            int loop_derived2 = i * 4;
            int loop_derived3 = i & 0x3F;
            
            /* Symbol addresses that can be rematerialized */
            int *addr1 = &global_array[i & 0xFF];
            int *addr2 = &global_array[(i + 1) & 0xFF];
            int *addr3 = &global_array[(i + 2) & 0xFF];
            
            /* More complex address calculations */
            long addr_offset = (long)(&global_array[0]) + (i * sizeof(int));
            int *addr4 = (int *)addr_offset;
            
            /* Mix different data types */
            long long_val1 = (long)const1 * const2;
            long long_val2 = (long)derived1 * derived2;
            int *ptr_val = addr1 + (derived3 >> 2);
            
            /* Create multiple basic blocks with conditional branches */
            if (i & 1) {
                /* Branch 1: Use some values */
                int temp1 = const1 + derived1;
                int temp2 = const2 + derived2;
                int temp3 = loop_derived1 * loop_derived2;
                
                /* More operations to increase live ranges */
                temp1 = temp1 ^ temp2;
                temp2 = temp3 + (i * 7);
                temp3 = temp1 - temp2;
                
                /* Use values to prevent elimination */
                use_value(temp1);
                use_value(temp2);
                use_value(temp3);
                
                /* Pointer arithmetic */
                int *temp_ptr = addr1 + (temp1 & 0xF);
                if (temp_ptr < &global_array[255]) {
                    *temp_ptr = temp2;
                }
            } else {
                /* Branch 2: Use different values */
                long temp4 = long_val1 + long_val2;
                int temp5 = derived3 + derived4;
                int temp6 = loop_derived3 * 2;
                
                /* More complex operations */
                temp4 = temp4 >> (i & 0x7);
                temp5 = temp5 ^ const1;
                temp6 = temp6 + (temp5 & 0xFF);
                
                /* Use values */
                use_value((int)temp4);
                use_value(temp5);
                use_value(temp6);
                
                /* Different pointer usage */
                if (addr2 != addr3) {
                    *addr2 = temp5;
                    *addr3 = temp6;
                }
            }
            
            /* Third basic block (always executed) */
            {
                /* Create more intermediate values */
                int mix1 = const1 ^ const2;
                int mix2 = derived1 | derived2;
                long mix3 = long_val1 - long_val2;
                int *mix_ptr = addr3 + (mix1 & 0x7);
                
                /* Complex expression chain */
                int chain1 = mix1 + mix2;
                int chain2 = chain1 * (i + 1);
                int chain3 = chain2 - mix1;
                int chain4 = chain3 ^ mix2;
                
                /* Use in volatile operations */
                vol_counter += chain1 + chain2 + chain3 + chain4;
                
                /* More address calculations */
                if (mix_ptr >= &global_array[0] && mix_ptr < &global_array[256]) {
                    *mix_ptr = chain4;
                }
            }
            
            /* Call function that clobbers registers - forces spills */
            clobber_registers();
            
            /* Another conditional block */
            if (i % 3 == 0) {
                /* More remat candidates */
                int extra1 = 0x55555555;
                int extra2 = 0xAAAAAAAA;
                int extra3 = param_base + i;
                long extra4 = (long)extra1 * extra2;
                
                /* Use them */
                int combined = extra1 + extra2 + extra3;
                use_value(combined);
                use_value((int)extra4);
                
                /* More pointer work */
                int *extra_ptr = &global_array[(i * 3) & 0xFF];
                *extra_ptr = combined;
            }
            
            /* Final computations using all types of values */
            int final1 = const1 + derived1 + loop_derived1;
            long final2 = long_val1 + (long)derived2 + (long)loop_derived2;
            int *final_ptr = addr1 + (final1 & 0xF);
            
            /* Store to prevent elimination */
            if (final_ptr >= &global_array[0] && final_ptr < &global_array[256]) {
                *final_ptr = final1 + (int)final2;
            }
            
            /* Use volatile to ensure computations aren't optimized away */
            sink = final1;
        }
        
        /* Prevent loop invariant motion */
        clobber_registers();
    }
    
    /* Return something based on computations */
    return vol_counter & 0xFF;
}
