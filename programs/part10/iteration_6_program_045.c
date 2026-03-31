/* Target: reorg.cc lines 2135-2149 - Delay slot filling logic */
/* Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish -o test_delay test_delay.c */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for critical variables */
register int a asm("$t0");
register int b asm("$t1");
register int c asm("$t2");
register int d asm("$t3");
register int e asm("$t4");
register int f asm("$t5");

/* Volatile counter to prevent loop unrolling */
volatile int iterations = 100;

/* Simple trap-free arithmetic operations for delay slot candidates */
static inline int safe_add(int x, int y) {
    return x + y;  /* Never traps */
}

static inline int safe_mul(int x, int y) {
    return x * y;  /* Safe for integers */
}

int main() {
    int result = 0;
    
    /* Initialize registers with non-zero values */
    a = 1;
    b = 2;
    c = 3;
    d = 4;
    e = 5;
    f = 6;
    
    /* Loop to create multiple branch opportunities */
    for (volatile int i = 0; i < iterations; i++) {
        /* Varying number of nops to create different filling scenarios */
        int nop_count = i % 4;
        
        /* BRANCH 1: Conditional with predictable outcome */
        if (__builtin_expect(a > b, 0)) {
            /* Insert varying number of nops before label */
            for (int j = 0; j < nop_count; j++) {
                asm volatile("nop" :::);
            }
            
        target_label_1:
            /* Candidate for delay slot: simple, trap-free, uses different registers */
            e = f + 1;  /* Independent of branch condition registers */
            
            /* Continue execution */
            result += e;
        } else {
            a = b + 1;  /* Change branch outcome for next iteration */
        }
        
        /* BRANCH 2: Different condition, different registers */
        if (__builtin_expect(c < d, 1)) {
            /* Different nop pattern */
            for (int j = 0; j < ((nop_count + 1) % 3); j++) {
                asm volatile("nop" :::);
            }
            
        target_label_2:
            /* Another delay slot candidate */
            a = safe_add(b, c);  /* Uses different registers than branch condition */
            
            result += a;
        } else {
            d = c - 1;
        }
        
        /* BRANCH 3: More complex to trigger multiple trial evaluations */
        if (__builtin_expect((a ^ b) > (c & d), 0)) {
            /* Minimal nops */
            asm volatile("nop" :::);
            
        target_label_3:
            /* Simple move operation - ideal delay slot candidate */
            f = e;
            
            result += f;
        } else {
            e = safe_mul(f, 2);
        }
        
        /* Modify variables to change branch behavior */
        a = (a * 3 + 1) & 0xFF;
        b = (b * 5 + 2) & 0xFF;
        c = (c * 7 + 3) & 0xFF;
        d = (d * 11 + 4) & 0xFF;
        
        /* Prevent optimization */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
    }
    
    /* BRANCH 4: Outside loop with clear delay slot opportunity */
    if (__builtin_expect(result > 1000, 1)) {
        /* Single nop for filler */
        asm volatile("nop" :::);
        
    final_target_label:
        /* Perfect delay slot candidate: uses completely independent registers */
        /* $t6 and $t7 are typically unused in this context */
        register int g asm("$t6") = 100;
        register int h asm("$t7") = 200;
        int temp = safe_add(g, h);
        result += temp;
    }
    
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}

/* Additional function to create more code for the optimizer */
void create_more_branches(int x, int y) {
    /* This creates additional branch contexts that might be optimized */
    register int p asm("$s0") = x;
    register int q asm("$s1") = y;
    register int r asm("$s2") = 0;
    
    for (int i = 0; i < 10; i++) {
        if (__builtin_expect(p > q, 0)) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);
        branch_target_1:
            r = p + q;  /* Delay slot candidate */
            p = r;
        } else {
            q = p;
        }
    }
}
