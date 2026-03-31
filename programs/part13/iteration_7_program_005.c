/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register spills
 * Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fdump-rtl-all
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to create rematerialization candidates */
volatile int global_base = 100;
volatile long global_offset = 1000;
int global_array[256] = {0};
static volatile int sink;

/* Dummy function to clobber registers */
static void __attribute__((noinline)) clobber_registers(void) {
    __asm__ volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Another noinline function to force spills */
static int __attribute__((noinline)) compute_offset(int idx) {
    __asm__ volatile ("");
    return idx * 3 + 7;
}

/* Main function with high register pressure loop */
int main(int argc, char *argv[]) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    int param = (argc > 2) ? atoi(argv[2]) : 42;
    
    /* Loop variant values that become remat candidates */
    volatile int loop_counter;
    
    for (int outer = 0; outer < 10; outer++) {
        for (int i = 0; i < iterations; i++) {
            /* Create many short-lived intermediate values with different modes */
            
            /* SImode constants (expensive to materialize) */
            int const1 = 0x7FFFFFFF;  /* Large constant requiring multiple insns */
            int const2 = 0x12345678;
            int const3 = 0x89ABCDEF;
            int const4 = 0x55555555;
            int const5 = 0xAAAAAAAA;
            
            /* DImode constants */
            long const6 = 0x7FFFFFFFFFFFFFFFLL;
            long const7 = 0x123456789ABCDEF0LL;
            
            /* Constants derived from function arguments (remat candidates) */
            int derived1 = param + 1;
            int derived2 = param << 2;
            int derived3 = param * 3;
            int derived4 = param / 2;
            int derived5 = param & 0xFF;
            
            /* Symbol addresses (Pmode remat candidates) */
            int *addr1 = &global_array[i % 256];
            int *addr2 = &global_array[(i + 1) % 256];
            int *addr3 = &global_array[(i + param) % 256];
            
            /* More derived values using loop counter */
            int val1 = i + const1;
            int val2 = i * const2;
            int val3 = i ^ const3;
            int val4 = i | const4;
            int val5 = i & const5;
            
            long val6 = (long)i + const6;
            long val7 = (long)i * const7;
            
            /* Complex expressions creating data dependencies */
            int expr1 = (derived1 * val1) >> 2;
            int expr2 = (derived2 + val2) & 0xFF;
            int expr3 = (derived3 ^ val3) | 0x80;
            int expr4 = (derived4 - val4) * 3;
            int expr5 = (derived5 | val5) + global_base;
            
            /* Multiple basic blocks to increase register pressure */
            if (i % 3 == 0) {
                /* Branch 1: More computations */
                int branch1_a = expr1 + expr2;
                int branch1_b = expr3 * expr4;
                long branch1_c = val6 + val7;
                
                /* Use addresses */
                *addr1 = branch1_a;
                *addr2 = branch1_b;
                
                sink = branch1_a + branch1_b;
            } else if (i % 3 == 1) {
                /* Branch 2: Different computations */
                int branch2_a = expr2 - expr3;
                int branch2_b = expr4 / (expr5 ? expr5 : 1);
                long branch2_c = val6 - val7;
                
                /* More address computations */
                int *addr4 = &global_array[(i + derived1) % 256];
                int *addr5 = &global_array[(i + derived2) % 256];
                
                *addr3 = branch2_a;
                *addr4 = branch2_b;
                
                sink = branch2_a * branch2_b;
            } else {
                /* Branch 3: Yet more computations */
                int branch3_a = expr3 ^ expr4;
                int branch3_b = expr5 << (i % 8);
                long branch3_c = val6 ^ val7;
                
                /* Pointer arithmetic */
                int *addr6 = addr1 + (derived3 % 16);
                int *addr7 = addr2 + (derived4 % 16);
                
                *addr6 = branch3_a;
                *addr7 = branch3_b;
                
                sink = branch3_a | branch3_b;
            }
            
            /* Force register clobbering between computations */
            clobber_registers();
            
            /* More computations after clobbering */
            int post1 = compute_offset(i);
            int post2 = compute_offset(i + 1);
            int post3 = compute_offset(i + param);
            
            /* Use all the values to prevent elimination */
            int sum = const1 + const2 + const3 + const4 + const5 +
                     derived1 + derived2 + derived3 + derived4 + derived5 +
                     val1 + val2 + val3 + val4 + val5 +
                     expr1 + expr2 + expr3 + expr4 + expr5 +
                     post1 + post2 + post3;
            
            /* Mix data types */
            long mixed = (long)sum + const6 + const7 + val6 + val7;
            
            /* Store to volatile to prevent dead code elimination */
            loop_counter = i;
            sink = sum + (int)mixed;
            
            /* Another clobber to increase spill pressure */
            clobber_registers();
            
            /* Final computations with different modes */
            int final1 = (sum << 2) | 0xF;
            int final2 = (sum >> 3) & 0x1F;
            long final3 = (mixed * 3) + global_offset;
            
            /* Use results */
            global_array[i % 256] = final1;
            sink = final2 + (int)final3;
        }
    }
    
    /* Verify some results to prevent complete optimization */
    int total = 0;
    for (int i = 0; i < 256; i++) {
        total += global_array[i];
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
