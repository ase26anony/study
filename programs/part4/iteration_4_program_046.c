/* reorg_delay_slot_test.c
 * Target: MIPS with delay slots (-march=mips1)
 * Compile with: mips-linux-gnu-gcc -O2 -march=mips1 -fdump-rtl-dbr -fdump-rtl-reorg -o test test.c
 * Or for QEMU: mips-linux-gnu-gcc -O2 -march=mips1 -static -o test test.c && qemu-mips test
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent MIPS16 mode which might not have delay slots */
#define NOMIPS16 __attribute__((nomips16))

/* Volatile variables to prevent optimization and create specific resource patterns */
static volatile int g_volatile_counter = 0;
static volatile int g_volatile_array[16] = {0};

/* Function to create complex control flow with potential delay slot filling */
NOMIPS16 static int test_delay_slot_pattern(int iterations) {
    /* Explicit register variables to control resource allocation */
    register int r0 asm("t0") = 0;  /* Candidate for delay slot */
    register int r1 asm("t1") = 0;  /* Used in jump condition */
    register int r2 asm("t2") = 0;  /* Independent register for next_trial */
    register int r3 asm("t3") = 0;  /* Another independent register */
    register int r4 asm("t4") = 0;  /* Yet another independent register */
    
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Pattern 1: Simple arithmetic that could fill delay slot */
        r0 = g_volatile_array[i & 0xF] + 1;  /* Potential delay slot candidate */
        
        /* Conditional jump to label - must compile to simplejump_p */
        r1 = g_volatile_counter++;
        if (r1 > (iterations / 2)) {
            /* This goto creates a jump_to_label_p with plain label target */
            goto label_a;
        }
        
        /* Some code to avoid fall-through optimization */
        r3 = r0 * 2;
        result += r3;
        continue;
        
    label_a:
        /* Independent instruction after label - potential next_trial */
        /* Uses different register (r2) than delay slot candidate (r0) */
        r2 = r4 ^ 0x55AA;  /* Simple arithmetic, non-trapping */
        result += r2;
        
        /* Ensure label is used to prevent removal */
        g_volatile_array[(i + 1) & 0xF] = r2;
    }
    
    /* Pattern 2: Another variation with different register usage */
    for (i = 0; i < iterations; i++) {
        /* Different delay slot candidate using memory operation */
        int temp = g_volatile_array[i & 0xF];
        r0 = temp << 2;  /* Shift operation as candidate */
        
        /* Another conditional jump */
        if ((i & 3) == 0) {
            goto label_b;
        }
        
        r3 = r0 | 0xFF;
        result += r3;
        continue;
        
    label_b:
        /* Independent arithmetic after label */
        r2 = r1 + r4;  /* Uses different registers than candidate */
        result += r2;
        
        /* Volatile store to prevent reordering */
        g_volatile_counter = r2;
    }
    
    /* Pattern 3: Nested control flow to increase reorg analysis */
    {
        register int a asm("s0") = result;
        register int b asm("s1") = iterations;
        register int c asm("s2") = 0;
        
        while (b-- > 0) {
            /* Candidate in loop */
            c = a & 0xFFFF;
            
            /* Multiple conditional jumps in sequence */
            if (c < 1000) {
                goto label_c1;
            }
            
            a = c * 3;
            continue;
            
        label_c1:
            /* First independent instruction */
            r2 = b * 2;  /* Different register set */
            result += r2;
            
            /* Immediate second conditional jump */
            if (r2 > 500) {
                goto label_c2;
            }
            
            a = r2 + 1;
            continue;
            
        label_c2:
            /* Second independent instruction */
            r3 = a ^ b;  /* Still independent from original candidate */
            result += r3;
        }
    }
    
    /* Pattern 4: Switch-like pattern with multiple labels */
    {
        int selector = iterations & 7;
        
        /* Arithmetic candidate */
        r0 = selector * 4;
        
        /* Multiple conditional jumps to different labels */
        if (selector == 0) goto label_d0;
        if (selector == 1) goto label_d1;
        if (selector == 2) goto label_d2;
        
        result += r0;
        return result;
        
    label_d0:
        r2 = 0x1000 + r4;  /* Independent */
        result += r2;
        goto label_done;
        
    label_d1:
        r2 = 0x2000 - r4;  /* Independent, different operation */
        result += r2;
        goto label_done;
        
    label_d2:
        r2 = r4 & 0x0FFF;  /* Independent, bitwise operation */
        result += r2;
        goto label_done;
        
    label_done:
        /* Ensure label is referenced */
        g_volatile_array[0] = r2;
    }
    
    return result;
}

/* Main function with forced register usage */
NOMIPS16 int main(void) {
    int total = 0;
    int i;
    
    /* Initialize volatile array */
    for (i = 0; i < 16; i++) {
        g_volatile_array[i] = i * 3;
    }
    
    /* Run multiple iterations to ensure reorg pass is triggered */
    for (i = 0; i < 10; i++) {
        total += test_delay_slot_pattern(100);
        
        /* Modify volatile to change execution path */
        g_volatile_counter += 50;
    }
    
    printf("Result: %d\n", total);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Use result to prevent dead code elimination */
    return total > 0 ? 0 : 1;
}
