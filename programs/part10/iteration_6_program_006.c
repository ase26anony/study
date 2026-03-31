/* Program to trigger GCC's delay slot filler logic for RISC architectures */
#include <stdio.h>
#include <stdlib.h>

/* Force the compiler to keep branches as-is */
#define UNLIKELY(x) __builtin_expect((x), 0)
#define LIKELY(x) __builtin_expect((x), 1)

/* Use volatile to prevent optimization of loop and branches */
static volatile int global_counter = 0;

int main(void) {
    /* Use register variables to control register allocation */
    register int a asm("$2") = 0;  /* Branch condition variable 1 */
    register int b asm("$3") = 1;  /* Branch condition variable 2 */
    register int c asm("$4") = 2;  /* Delay slot candidate variable 1 */
    register int d asm("$5") = 3;  /* Delay slot candidate variable 2 */
    register int e asm("$6") = 0;  /* Result accumulator */
    
    /* Volatile iteration count to prevent loop unrolling */
    volatile int iterations = 100;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* First branch pattern: multiple nops before label */
        if (UNLIKELY(a > b)) {
            /* Multiple nops to create filler opportunities */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target1;
        }
        
        /* Filler instruction that might be moved into delay slot */
        asm volatile("nop" ::: "memory");
        
    target1:
        /* Simple arithmetic instruction - eligible for delay slot */
        /* Uses different registers than branch condition (c,d vs a,b) */
        c = d + 1;  /* This should be the 'next_trial' candidate */
        
        /* Update variables to change branch behavior */
        a = i % 10;
        b = (i + 1) % 10;
        e += c;
        
        /* Second branch pattern: different number of nops */
        if (LIKELY(d < 50)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target2;
        }
        
        asm volatile("nop" ::: "memory");
        
    target2:
        /* Another simple arithmetic instruction */
        d = c + a;  /* Uses mixed registers but safe */
        
        /* Third branch pattern: minimal nops */
        if (UNLIKELY((a + b) > 15)) {
            asm volatile("nop" ::: "memory");
            goto target3;
        }
        
    target3:
        /* Register move operation - very simple candidate */
        int temp = d;  /* Simple move that doesn't trap */
        (void)temp;    /* Prevent unused variable warning */
        
        /* Fourth branch: complex enough to require multiple trials */
        if (LIKELY(e < 1000)) {
            /* Multiple filler instructions */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target4;
        }
        
        /* More filler */
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        
    target4:
        /* Safe arithmetic with immediate constant */
        c = b + 2;  /* Immediate add - won't trap */
        
        /* Update global to prevent dead code elimination */
        global_counter += a + b + c + d;
        
        /* Vary the branch conditions */
        if (i % 3 == 0) {
            a++;
        } else if (i % 3 == 1) {
            b--;
        } else {
            d += 2;
        }
    }
    
    /* Additional branch patterns outside loop */
    register int x asm("$7") = 100;
    register int y asm("$8") = 200;
    register int z asm("$9") = 0;
    
    /* Nested conditional to create more complex CFG */
    if (UNLIKELY(x < y)) {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        goto final_target;
    }
    
    asm volatile("nop" ::: "memory");
    
final_target:
    /* Final simple instruction for delay slot candidate */
    z = x + y;  /* Simple addition, different registers */
    
    /* Use results to prevent optimization */
    printf("Result: %d (global: %d)\n", e + z, global_counter);
    
    return (e + z) > 0 ? 0 : 1;
}

/* Helper function to create more branch opportunities */
static void branch_helper(int limit) {
    register int p asm("$10") = 0;
    register int q asm("$11") = 1;
    register int r asm("$12") = 0;
    
    for (int j = 0; j < limit; j++) {
        /* Alternate between likely and unlikely branches */
        if (j % 2 == 0 ? UNLIKELY(p < q) : LIKELY(p < q)) {
            asm volatile("nop" ::: "memory");
            goto helper_target;
        }
        
        asm volatile("nop" ::: "memory");
        
    helper_target:
        /* Simple, safe instruction */
        r = p + q;
        
        /* Update variables */
        p = (p + 1) % 20;
        q = (q + 2) % 20;
        
        /* Prevent optimization */
        global_counter += r;
    }
}

/* Call helper to increase coverage */
void call_helper(void) {
    branch_helper(50);
}
