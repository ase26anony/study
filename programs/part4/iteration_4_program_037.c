/* 
 * This program is designed to trigger GCC's delay slot filling logic
 * in the Reorg pass, specifically targeting the uncovered block in reorg.cc
 * lines 2135-2149.
 *
 * Compile with: gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg
 * Or for execution: gcc -O2 -march=mips1 -static -o reorg_test reorg_test.c
 * Run with QEMU: qemu-mips ./reorg_test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which may not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile variables to prevent optimization and create specific resource patterns */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_array[16] = {0};

/* Register variables to control register allocation */
register int reg_a asm("t0");
register int reg_b asm("t1");
register int reg_c asm("t2");
register int reg_d asm("t3");
register int reg_e asm("t4");
register int reg_f asm("t5");

/* Function with NOMIPS16 attribute to ensure delay slot filling is attempted */
void NOMIPS16 delay_slot_test(int iterations) {
    int i;
    int result = 0;
    
    /* Initialize register variables with volatile values */
    reg_a = g_volatile_counter + 1;
    reg_b = g_volatile_counter + 2;
    reg_c = g_volatile_counter + 3;
    reg_d = g_volatile_counter + 4;
    reg_e = g_volatile_counter + 5;
    reg_f = g_volatile_counter + 6;
    
    for (i = 0; i < iterations; i++) {
        /* 
         * Pattern 1: Simple conditional jump with arithmetic before it.
         * The arithmetic (reg_a + reg_b) could be a delay slot candidate.
         */
        int temp1 = reg_a + reg_b;  /* Potential delay slot candidate */
        
        /* Simple conditional jump to label1 */
        if (reg_a > reg_b) {
            /* This goto creates a simplejump_p with a label target */
            goto label1;
        }
        
        /* Some intermediate code to avoid fall-through optimization */
        reg_c = reg_c + 1;
        continue;
        
    label1:
        /* 
         * This instruction (reg_d + reg_e) is next_trial - the instruction
         * after the jump label. It uses different registers than the
         * delay slot candidate (temp1 used reg_a, reg_b).
         * It's a simple integer operation that doesn't trap.
         */
        int temp2 = reg_d + reg_e;  /* Potential next_trial */
        result += temp2;
        
        /* 
         * Pattern 2: Another conditional jump with different registers
         * to increase slots_to_fill analysis opportunities.
         */
        int temp3 = reg_c & reg_f;  /* Another delay slot candidate */
        
        if (reg_d < reg_e) {
            goto label2;
        }
        
        reg_a = reg_a - 1;
        continue;
        
    label2:
        /* Another independent instruction after label */
        int temp4 = reg_b ^ reg_c;  /* Another potential next_trial */
        result += temp4;
        
        /* 
         * Pattern 3: Use volatile memory access to create specific
         * resource patterns that the reorg pass must analyze.
         */
        volatile int* volatile_ptr = &g_volatile_array[i & 0xF];
        int temp5 = *volatile_ptr;  /* Delay slot candidate with memory */
        
        if (reg_f != 0) {
            goto label3;
        }
        
        reg_d = reg_d + 2;
        continue;
        
    label3:
        /* Independent arithmetic after label, different register set */
        int temp6 = reg_e * 2;  /* Potential next_trial */
        result += temp6;
        
        /* Update volatile to prevent optimization */
        g_volatile_counter = result;
        
        /* Rotate registers to create varying patterns */
        int rot = reg_a;
        reg_a = reg_b;
        reg_b = reg_c;
        reg_c = reg_d;
        reg_d = reg_e;
        reg_e = reg_f;
        reg_f = rot;
    }
    
    /* Use result to prevent dead code elimination */
    g_volatile_array[0] = result;
}

/* Additional test function with more complex control flow */
void NOMIPS16 nested_branch_test(int limit) {
    register int r1 asm("s0");
    register int r2 asm("s1");
    register int r3 asm("s2");
    
    r1 = g_volatile_counter;
    r2 = r1 + 100;
    r3 = 0;
    
    while (r1 < r2) {
        /* Multiple nested conditions to create multiple jump opportunities */
        int t1 = r1 * r2;  /* Delay slot candidate */
        
        if (r1 > 50) {
            goto outer_label;
        }
        
        /* Inner conditional */
        int t2 = r3 & 0xFF;  /* Another candidate */
        
        if (r2 < 200) {
            goto inner_label;
        }
        
        r3 = r3 + 1;
        continue;
        
    inner_label:
        /* Independent instruction after inner label */
        int t3 = r1 | r2;  /* Potential next_trial */
        r3 += t3;
        continue;
        
    outer_label:
        /* Independent instruction after outer label */
        int t4 = r1 - r2;  /* Potential next_trial */
        r3 -= t4;
        
        r1++;
    }
    
    g_volatile_array[1] = r3;
}

int main() {
    printf("Starting delay slot test...\n");
    
    /* Call test functions multiple times to ensure reorg pass is invoked */
    delay_slot_test(1000);
    nested_branch_test(500);
    
    printf("Results: g_volatile_counter = %d, g_volatile_array[0] = %d, g_volatile_array[1] = %d\n",
           g_volatile_counter, g_volatile_array[0], g_volatile_array[1]);
    
    /* Verify computation was performed */
    if (g_volatile_counter != 0 || g_volatile_array[0] != 0 || g_volatile_array[1] != 0) {
        printf("Test completed with non-zero results.\n");
    } else {
        printf("Warning: All results are zero - optimization may have removed too much.\n");
    }
    
    return 0;
}
