/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish -o test test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables */
register int a asm("$t0");
register int b asm("$t1");
register int c asm("$t2");
register int d asm("$t3");
register int e asm("$t4");
register int f asm("$t5");
register int g asm("$t6");
register int h asm("$t7");

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

int main() {
    int result = 0;
    
    /* Initialize registers with distinct values */
    a = 1; b = 2; c = 3; d = 4;
    e = 5; f = 6; g = 7; h = 8;
    
    /* Loop to create multiple branch opportunities */
    for (int i = 0; i < iterations; i++) {
        /* BRANCH 1: Predictable taken branch with nop filler */
        /* This creates a trial instruction (nop) that can be moved */
        if (__builtin_expect(a < b, 1)) {
            asm volatile("nop" ::: "memory");
            goto label1;
        }
        /* Fall-through path */
        c = d + 1;
        continue;
        
    label1:
        /* Candidate for delay slot filling: Simple arithmetic with
           registers not used in branch condition (e vs f) */
        e = f + 2;  /* This should be eligible for delay slot */
        
        /* Update branch condition variables to change behavior */
        a = b + i;
        b = c - 1;
        
        /* BRANCH 2: Predictable not-taken branch with different nop count */
        /* Varying nop count forces different trial evaluations */
        if (__builtin_expect(c > d, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto label2;
        }
        /* Fall-through */
        f = g + 3;
        continue;
        
    label2:
        /* Another eligible candidate using different registers */
        g = h + 4;
        
        /* BRANCH 3: Unpredictable branch to test multiple paths */
        /* Uses volatile to prevent optimization */
        int volatile cond = i & 1;
        if (__builtin_expect(cond, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto label3;
        }
        h = a + 5;
        continue;
        
    label3:
        /* Final candidate - very simple operation */
        d = e + 6;
        
        /* Mix up register usage to prevent resource conflicts */
        int temp = a;
        a = b;
        b = temp;
        
        /* Accumulate result to keep computation live */
        result += (a + b + c + d + e + f + g + h) & 0xFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test case: Nested branches */
    {
        register int x asm("$s0") = 10;
        register int y asm("$s1") = 20;
        register int z asm("$s2") = 30;
        register int w asm("$s3") = 40;
        
        /* Create a branch whose target is immediately after label */
        if (__builtin_expect(x < y, 1)) {
            asm volatile("nop" ::: "memory");
            goto target_label;
        }
        z = w + 7;
        
    target_label:
        /* Perfect delay slot candidate: uses s4/s5, not s0/s1/s2/s3 */
        register int p asm("$s4") = 50;
        register int q asm("$s5") = 60;
        p = q + 8;  /* Should be moved into delay slot */
        
        result += p;
    }
    
    return result & 0xFF;
}

/* Helper function to create more complex control flow */
static void create_branch_chain(int limit) {
    register int i asm("$t8") = 0;
    register int j asm("$t9") = limit;
    
    while (i < j) {
        /* Branch with multiple nops before target */
        if (__builtin_expect(i < j/2, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto chain_label;
        }
        i++;
        continue;
        
    chain_label:
        /* Candidate instruction after label */
        register int k asm("$v0") = i;
        register int m asm("$v1") = j;
        k = m + i;  /* Simple arithmetic, no traps */
        
        i = k;
        j--;
    }
}
