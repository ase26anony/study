/* reorg_coverage.c - Target program for GCC delay slot filling coverage */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage to avoid resource conflicts */
register int a asm("$2");  /* MIPS $v0, SPARC %o0 - branch condition var 1 */
register int b asm("$3");  /* MIPS $v1, SPARC %o1 - branch condition var 2 */
register int c asm("$4");  /* MIPS $a0, SPARC %o2 - delay slot candidate 1 */
register int d asm("$5");  /* MIPS $a1, SPARC %o3 - delay slot candidate 2 */
register int e asm("$6");  /* MIPS $a2, SPARC %o4 - independent computation */
register int f asm("$7");  /* MIPS $a3, SPARC %o5 - independent computation */
register int g asm("$8");  /* MIPS $t0, SPARC %l0 - loop counter */
register int h asm("$9");  /* MIPS $t1, SPARC %l1 - accumulator */

/* Volatile to prevent optimization */
volatile int iterations = 100;
volatile int seed = 42;

/* Simple trap-free arithmetic operations for delay slot candidates */
#define SAFE_OP1(x, y) ((x) + (y))        /* Never traps */
#define SAFE_OP2(x, y) ((x) - (y))        /* Never traps */
#define SAFE_OP3(x)    ((x) << 1)         /* Never traps */
#define SAFE_OP4(x)    ((x) & 0x7FFFFFFF) /* Never traps */

int main(void) {
    int result = 0;
    
    /* Initialize registers with non-zero values */
    a = seed + 1;
    b = seed;
    c = 100;
    d = 200;
    e = 300;
    f = 400;
    g = 0;
    h = 0;
    
    /* Loop to create multiple branch opportunities */
    for (g = 0; g < iterations; g++) {
        /* BRANCH 1: Predictable taken branch with nop filler */
        /* This creates a trial instruction for delay slot filling */
        if (__builtin_expect(a > b, 1)) {
            /* Multiple nops to create filler instructions */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            
            /* Target label for the branch */
            target_label_1:
            /* ELIGIBLE DELAY SLOT CANDIDATE:
               Simple register-to-register operation, no traps,
               uses registers not involved in branch condition */
            e = f + 1;  /* Independent of a,b - no resource conflict */
            
            /* Continue with some computation */
            h += e;
        }
        
        /* BRANCH 2: Predictable not-taken branch with different structure */
        /* Vary the pattern to explore different code paths in reorg */
        if (__builtin_expect(c < d, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            
            target_label_2:
            /* Another eligible candidate using different registers */
            a = b + c;  /* Still safe, though uses b and c */
            
            h += a;
        }
        
        /* BRANCH 3: Unpredictable branch for more coverage */
        /* Use volatile to prevent dead code elimination */
        volatile int r = rand() % 100;
        if (__builtin_expect(r > 50, r > 50)) {
            asm volatile("nop" ::: "memory");
            
            target_label_3:
            /* Simple shift operation - trap-free */
            f = SAFE_OP3(e);
            
            h += f;
        }
        
        /* BRANCH 4: Complex condition with multiple nops */
        /* This creates more filler instructions to process */
        if (__builtin_expect((a ^ b) > 0, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            
            target_label_4:
            /* Bitwise operation - completely safe */
            d = SAFE_OP4(c);
            
            h += d;
        }
        
        /* Update variables to change branch patterns */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = (b * 1664525 + 1013904223) & 0x7FFFFFFF;
        c = c + g;
        d = d - g;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(g) : : "memory");
    }
    
    /* BRANCH 5: Final branch outside loop for additional coverage */
    if (__builtin_expect(h > 0, 1)) {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        
        target_label_5:
        /* Final eligible delay slot candidate */
        result = e + f;
    } else {
        result = h;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}

/* Additional function to create more context for delay slot filling */
int process_values(int x, int y) {
    register int r1 asm("$10") = x;
    register int r2 asm("$11") = y;
    register int r3 asm("$12") = 0;
    
    /* Create another branch context */
    if (__builtin_expect(r1 != r2, 1)) {
        asm volatile("nop" ::: "memory");
        
        another_target_label:
        /* Another simple, safe operation */
        r3 = r1 | r2;
    }
    
    return r3;
}
