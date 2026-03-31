/* reorg_coverage.c - Target GCC delay slot filling logic */
#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for predictable resource usage */
#ifdef __mips__
#define REG1 asm("$t0")
#define REG2 asm("$t1")
#define REG3 asm("$t2")
#define REG4 asm("$t3")
#define REG5 asm("$t4")
#define REG6 asm("$t5")
#elif __sparc__
#define REG1 asm("%l0")
#define REG2 asm("%l1")
#define REG3 asm("%l2")
#define REG4 asm("%l3")
#define REG5 asm("%l4")
#define REG6 asm("%l5")
#else
/* Generic register hints for RISC architectures */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

int main(void) {
    /* Declare variables in specific registers to avoid resource conflicts */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    volatile int result = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Varying number of nops to force different trial evaluations */
        switch (i % 4) {
            case 0:
                asm volatile("nop" ::: "memory");
                break;
            case 1:
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                break;
            case 2:
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                break;
        }
        
        /* Conditional branch with predictable pattern for delay slot filling */
        /* Branch condition uses registers a and b only */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop as potential delay slot filler */
            asm volatile("nop" ::: "memory");
            
            /* Target label 1 - candidate for delay slot filling */
            /* Simple arithmetic using different registers (c,d) than branch condition */
            target_label_1:
                c = d + 1;  /* Eligible: register move/add, no trap, no resource conflict */
            
            /* Continue with other operations */
            result += c;
        } else {
            /* Alternative path to create different branch patterns */
            asm volatile("nop" ::: "memory");
            target_label_2:
                e = f + 2;  /* Another eligible candidate */
            result += e;
        }
        
        /* Another branch with different register usage pattern */
        if (__builtin_expect(c < d, 1)) {
            asm volatile("nop" ::: "memory");
            target_label_3:
                a = b + 3;  /* Swapped register usage */
            result += a;
        }
        
        /* Update variables to change branch outcomes */
        a = (a + 1) % 10;
        b = (b + 2) % 10;
        c = (c + 3) % 10;
        d = (d + 4) % 10;
        e = (e + 5) % 10;
        f = (f + 6) % 10;
        
        /* Prevent optimization of loop body */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), 
                          "+r"(d), "+r"(e), "+r"(f));
    }
    
    /* Complex nested branch structure to increase slots_to_fill opportunities */
    {
        register int x REG1 = 0;
        register int y REG2 = 10;
        register int z REG3 = 20;
        register int w REG4 = 30;
        
        for (int j = 0; j < 50; j++) {
            /* Multiple nops to create filler candidates */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            
            if (__builtin_expect(x < y, 0)) {
                asm volatile("nop" ::: "memory");
                target_label_4:
                    z = w + j;  /* Simple arithmetic, no trap */
                
                /* Inner branch to create more complex control flow */
                if (__builtin_expect(z > 25, 1)) {
                    asm volatile("nop" ::: "memory");
                    target_label_5:
                        x = y * 2;  /* Multiplication is safe, no division */
                    result += x;
                }
            }
            
            x++;
            y--;
            z += 2;
            w -= 1;
        }
    }
    
    /* Final computation using all variables to keep them live */
    result = a + b + c + d + e + f;
    
    printf("Result: %d\n", result);
    return result != 0;
}
