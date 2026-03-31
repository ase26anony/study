/* delay_slot_filler.c
 * Designed to trigger GCC's delay slot filling logic in reorg.cc lines 2135-2149
 * Compile with: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish -o test delay_slot_filler.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register usage for MIPS-like architectures */
#ifdef __mips__
#define REG1 asm("$8")   /* t0 */
#define REG2 asm("$9")   /* t1 */
#define REG3 asm("$10")  /* t2 */
#define REG4 asm("$11")  /* t3 */
#define REG5 asm("$12")  /* t4 */
#define REG6 asm("$13")  /* t5 */
#else
/* Generic register hints for other RISC architectures */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

int main(void) {
    /* Declare variables with register hints to control allocation */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    int result = 0;
    
    /* Loop to create multiple branch opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Create predictable branch pattern using __builtin_expect */
        if (__builtin_expect((a > b), 0)) {
            /* Branch target 1 - will be reached occasionally */
            asm volatile("nop" ::: "memory");  /* Filler for delay slot candidate */
target1:
            /* Simple arithmetic instruction - eligible for delay slot */
            /* Uses registers not involved in branch condition (c,d vs a,b) */
            c = d + 1;  /* This should be moved into delay slot */
            
            /* Continue with more independent operations */
            e = f * 2;
        } else {
            /* Fall through path */
            a = b + i;
        }
        
        /* Second branch with different pattern */
        if (__builtin_expect((c < d), 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple trial attempts */
target2:
            /* Another simple arithmetic with different registers */
            f = e - 1;  /* Another delay slot candidate */
            
            d = c * 3;
        } else {
            b = a - i;
        }
        
        /* Third branch with alternating pattern */
        if (__builtin_expect((i % 3) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Three nops */
target3:
            /* Register move operation - very simple candidate */
            a = b;  /* Simple move, no traps, no resource conflicts */
            
            c = d + i;
        }
        
        /* Update variables to change branch outcomes */
        a = (a + 1) & 0xFF;
        b = (b * 3) & 0xFF;
        c = (c - 1) & 0xFF;
        d = (d + i) & 0xFF;
        
        /* Accumulate result to keep computations live */
        result += a + b + c + d + e + f;
    }
    
    /* Force result to be used */
    printf("Result: %d\n", result);
    
    /* Additional test case with nested branches */
    {
        register int x REG1 = 0;
        register int y REG2 = 10;
        register int z REG3 = 20;
        register int w REG4 = 30;
        
        /* Create a more complex branching pattern */
        for (int j = 0; j < 50; j++) {
            if (__builtin_expect(x < y, 1)) {
                asm volatile("nop" ::: "memory");
target4:
                /* Multiple independent instructions after label */
                z = w + j;      /* First instruction - delay slot candidate */
                x = y * 2;      /* Second instruction - stays after branch */
                
                if (__builtin_expect(z > w, 0)) {
                    asm volatile("nop" ::: "memory");
target5:
                    w = z - 5;  /* Another candidate in nested branch */
                }
            }
            
            x++;
            y--;
            result += z + w;
        }
    }
    
    printf("Final result: %d\n", result);
    return result != 0;
}
