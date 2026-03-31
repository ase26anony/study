/* Program to trigger GCC's delay slot filling logic (reorg.cc lines 2135-2149) */
#include <stdio.h>
#include <stdlib.h>

/* Force the compiler to keep branches as-is */
#define BARRIER() asm volatile("" ::: "memory")

/* Create predictable branch patterns */
#define UNLIKELY(x) __builtin_expect((x), 0)
#define LIKELY(x) __builtin_expect((x), 1)

int main(void) {
    /* Use register variables to control allocation */
    register int a asm("$2") = 0;  /* Branch condition variable 1 */
    register int b asm("$3") = 1;  /* Branch condition variable 2 */
    register int c asm("$4") = 0;  /* Delay slot candidate variable 1 */
    register int d asm("$5") = 10; /* Delay slot candidate variable 2 */
    register int e asm("$6") = 0;  /* Loop counter */
    register int f asm("$7") = 0;  /* Result accumulator */
    
    volatile int iterations = 100; /* Prevent loop unrolling */
    
    /* Main loop with multiple branch patterns */
    for (e = 0; e < iterations; e++) {
        /* Pattern 1: Branch with single nop before target */
        if (UNLIKELY(a > b)) {
            /* Force a nop that could be replaced */
            asm volatile("nop" :::);
            /* Target label for delay slot candidate */
target1:
            /* Simple arithmetic - safe for delay slot */
            c = d + 1;  /* Uses different registers than branch condition */
            f += c;
        }
        
        BARRIER();
        
        /* Pattern 2: Branch with two nops (different slot count) */
        if (LIKELY(b < a + e)) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);
target2:
            /* Another simple, safe operation */
            d = c - 1;  /* Register move/arithmetic, no trap possible */
            f += d;
        }
        
        BARRIER();
        
        /* Pattern 3: Branch that usually falls through */
        if (UNLIKELY((a & 0x1F) == (b & 0x1F))) {
            asm volatile("nop" :::);
            asm volatile("nop" :::);
            asm volatile("nop" :::);
target3:
            /* Another independent operation */
            a = b ^ 0x55;  /* Bitwise operation, no division/trapping */
            f += a;
        }
        
        BARRIER();
        
        /* Pattern 4: Complex condition but simple target */
        if (UNLIKELY((a * b) < 1000)) {
            asm volatile("nop" :::);
target4:
            /* Simple increment - ideal delay slot candidate */
            b = a + 2;  /* Safe, non-trapping operation */
            f += b;
        }
        
        /* Update variables to change branch behavior */
        a = (a + 3) & 0xFF;  /* Keep in range */
        b = (b * 5 + 1) & 0xFF;
        c = (c + d) & 0xFF;
        d = (d - 1) & 0xFF;
        
        BARRIER();
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (a=%d, b=%d, c=%d, d=%d)\n", f, a, b, c, d);
    
    /* Additional test case: nested branches */
    {
        register int x asm("$8") = 0;
        register int y asm("$9") = 100;
        register int z asm("$10") = 0;
        
        for (x = 0; x < 50; x++) {
            /* Outer branch */
            if (UNLIKELY(x > y/2)) {
                asm volatile("nop" :::);
outer_target:
                /* Inner branch with its own target */
                if (LIKELY(y < 75)) {
                    asm volatile("nop" :::);
inner_target:
                    /* Candidate for inner branch's delay slot */
                    z = x + y;  /* Simple addition */
                }
                /* Candidate for outer branch's delay slot */
                y = x - 1;  /* Simple subtraction */
            }
            y++;
        }
        printf("Nested result: z=%d\n", z);
    }
    
    return f != 0 ? 0 : 1;
}
