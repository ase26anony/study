/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for predictable resource usage */
register int a asm("$t0");
register int b asm("$t1");
register int c asm("$t2");
register int d asm("$t3");
register int e asm("$t4");
register int f asm("$t5");
register int g asm("$t6");
register int h asm("$t7");

/* Volatile to prevent optimization */
volatile int iterations = 100;
volatile int seed = 42;

int main() {
    int result = 0;
    
    /* Initialize registers with distinct values */
    a = seed + 1;
    b = seed + 2;
    c = seed + 3;
    d = seed + 4;
    e = seed + 5;
    f = seed + 6;
    g = seed + 7;
    h = seed + 8;
    
    /* Loop to create multiple branch filling opportunities */
    for (int i = 0; i < iterations; i++) {
        /* BRANCH 1: Simple conditional with predictable pattern */
        /* This creates a trial instruction (nop) that can be moved */
        if (__builtin_expect(a > b, 0)) {
            /* Multiple nops to create filler slots */
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            
            /* Target label with simple arithmetic instruction */
            /* This is the next_trial candidate */
            /* Uses registers not involved in branch condition */
            c = d + 1;  /* Simple trap-free arithmetic */
            
            /* Continue with more operations */
            e = f * 2;
        } else {
            /* Alternate path with different register usage */
            g = h - 1;
        }
        
        /* BRANCH 2: Different condition, different register sets */
        /* Creates another opportunity for delay slot filling */
        if (__builtin_expect(c < d, 1)) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            
            /* Another candidate instruction after label */
            e = f + g;  /* Simple addition, no traps */
            
            h = a * 2;
        }
        
        /* BRANCH 3: More complex pattern with varying nop count */
        /* This helps trigger slots_to_fill != slots_filled logic */
        if (__builtin_expect((e & 0x1) == 0, 0)) {
            asm volatile("nop" :::);
            
            /* Candidate: register move operation */
            a = b;  /* Simple move, no resource conflicts */
            
            f = g + h;
        } else {
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            
            b = c;  /* Another move candidate */
        }
        
        /* Update variables to change branch outcomes */
        a += i;
        b += (i * 2);
        c ^= i;
        d += 3;
        
        /* Mix in some arithmetic to create more independent ops */
        int temp = e + f;
        g = temp - h;
        h = (temp & 0xFF) + 1;
        
        /* Accumulate result to keep computation live */
        result += a + b + c + d;
    }
    
    /* BRANCH 4: Final branch outside loop */
    /* Creates one more opportunity for delay slot filling */
    if (__builtin_expect(result > 1000, 1)) {
        asm volatile("nop" :::);
        asm volatile("nop" :::);
        asm volatile("nop" :::);
        
        /* Final candidate instruction */
        d = e + f;  /* Independent arithmetic */
        
        printf("Result: %d\n", result + d);
    } else {
        printf("Result: %d\n", result);
    }
    
    return 0;
}

/* Additional function to create more context */
void helper_function(int x, int y) {
    /* Creates more basic blocks with simple operations */
    register int r1 asm("$t8") = x;
    register int r2 asm("$t9") = y;
    
    if (__builtin_expect(r1 != r2, 0)) {
        asm volatile("nop" :::);
        
        /* Another delay slot candidate */
        int r3 = r1 + 1;  /* Simple increment */
        
        printf("Diff: %d\n", r3 - r2);
    }
}
