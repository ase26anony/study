/* Test program for GCC reorg.cc fill_eager_delay_slots uncovered lines */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent optimization */
volatile int global_acc = 0;

/* Optimization barrier functions */
__attribute__((noinline)) int barrier1(int x) { return x ^ 0x55AA55AA; }
__attribute__((noinline)) int barrier2(int x) { return x ^ 0xAA55AA55; }

/* MIPS-specific test function */
#ifdef __mips__
__attribute__((target("arch=mips32")))
#endif
int test_mips_delay_slot(int a, int b) {
    /* Create temporaries independent of condition */
    int temp1 = barrier1(a);
    int temp2 = barrier1(b);
    int temp3 = temp1 + temp2;
    int temp4 = temp1 - temp2;
    
    /* Dynamic condition using input args */
    if (a > b && (a % 3) != 0) {
        /* This should compile to a simple jump to label */
        goto target_label;
    }
    
    /* Some other code to create basic blocks */
    temp3 = barrier2(temp3);
    temp4 = barrier2(temp4);
    
    /* Return path 1 */
    return temp3 + temp4;
    
target_label:
    /* SAFE instruction after label: simple arithmetic on temporaries */
    /* This should be eligible for delay slot filling */
    temp3 = temp1 & 0xFF;  /* Safe bitwise operation */
    
    /* Use result to prevent dead code elimination */
    return temp3 + 1;
}

/* SPARC-specific test function */
#ifdef __sparc__
__attribute__((target("arch=sparc")))
#endif
int test_sparc_delay_slot(int x, int y, int z) {
    /* Independent temporaries */
    int t1 = barrier1(x);
    int t2 = barrier1(y);
    int t3 = barrier1(z);
    int t4 = t1 * 2;
    int t5 = t2 * 3;
    
    /* Complex enough condition to prevent optimization */
    volatile int cond_check = x;
    if ((cond_check & 7) == 3 && y != 0) {
        /* Simple jump to label */
        goto sparc_target;
    }
    
    /* Alternative path with operations */
    t4 = t4 ^ t5;
    t5 = t3 << 2;
    
    return t4 - t5;
    
sparc_target:
    /* Safe instruction: logical operation on independent temps */
    t4 = t2 | 0x0F;  /* No trap possible */
    
    return t4 + t3;
}

/* Generic delay slot test with multiple basic blocks */
int test_generic_delay(int seed) {
    int i, j, k, result = 0;
    
    /* Initialize independent variables */
    i = barrier1(seed);
    j = barrier2(seed + 1);
    k = i + j;
    
    /* Create multiple basic blocks with loops */
    for (int n = 0; n < 3; n++) {
        int local_temp = i + n;
        
        /* Condition that varies per iteration */
        if ((seed + n) % 5 == 2) {
            /* Jump to label - should be simple jump */
            goto generic_label;
        }
        
        /* Other computation */
        local_temp = local_temp * 2;
        result += local_temp;
    }
    
    /* Fall-through return */
    return result + k;
    
generic_label:
    /* Safe instruction: arithmetic on local temporaries */
    k = j ^ 0x3C;  /* XOR with constant - always safe */
    
    return k + 1;
}

/* Test with nested control flow */
int test_nested_control(int a, int b, int c) {
    int t1 = a;
    int t2 = b;
    int t3 = c;
    int t4 = 0;
    
    /* Outer condition */
    if (a > 10) {
        /* Inner condition */
        if (b < 20) {
            /* Multiple temporaries defined before jump */
            int inner_temp1 = t1 + t2;
            int inner_temp2 = t2 + t3;
            
            /* Dynamic condition */
            volatile int check = c;
            if ((check & 1) == 0) {
                /* Simple jump to label */
                goto nested_target;
            }
            
            inner_temp1 = inner_temp1 * 2;
            t4 = inner_temp1 + inner_temp2;
        }
    }
    
    t4 = t4 + t3;
    return t4;
    
nested_target:
    /* Safe instruction using inner temporaries */
    t4 = t1 & t2;  /* Bitwise AND - no traps */
    
    return t4 + 5;
}

/* Test with multiple candidate delay slots */
int test_multiple_candidates(int base) {
    int vars[4];
    int result = 0;
    
    /* Initialize array with independent values */
    for (int i = 0; i < 4; i++) {
        vars[i] = barrier1(base + i);
    }
    
    /* Multiple conditional jumps to different labels */
    if ((base % 7) == 1) {
        goto label_a;
    } else if ((base % 7) == 3) {
        goto label_b;
    }
    
    result = vars[0] + vars[1];
    return result;
    
label_a:
    /* First safe instruction candidate */
    vars[2] = vars[0] + 1;  /* Simple increment */
    return vars[2];
    
label_b:
    /* Second safe instruction candidate */
    vars[3] = vars[1] | 0x7F;  /* Bitwise OR */
    return vars[3];
}

/* Main driver that exercises all test functions */
int main() {
    int checksum = 0;
    
    /* Test with various inputs to explore different paths */
    for (int i = 0; i < 10; i++) {
        checksum ^= test_mips_delay_slot(i, i * 2);
        checksum ^= test_sparc_delay_slot(i, i + 1, i + 2);
        checksum ^= test_generic_delay(i);
        checksum ^= test_nested_control(i, i + 5, i + 10);
        checksum ^= test_multiple_candidates(i);
        
        /* Update global to prevent optimization */
        global_acc += checksum;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("Global accumulator: %d\n", global_acc);
    
    return 0;
}
