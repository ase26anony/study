/* Program to trigger GCC's delay slot filling logic for uncovered lines in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage to avoid resource conflicts */
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

int main() {
    /* Use register variables to control allocation and avoid spills */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    int result = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Vary branch predictability to create different filling scenarios */
        int branch_taken = (i % 3) == 0;
        
        /* BRANCH 1: Simple conditional with predictable outcome */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop to create filler opportunity */
            asm volatile("nop" ::: "memory");
target_label1:
            /* Candidate for delay slot: simple arithmetic with different registers */
            e = f + 1;  /* Uses REG5 and REG6, not involved in branch condition */
            /* Continue with other operations */
            c = d * 2;
        } else {
            a = b + i;
        }
        
        /* BRANCH 2: Different condition with varying nop count */
        if (__builtin_expect(c < d, 1)) {
            /* Multiple nops to create different trial scenarios */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label2:
            /* Another delay slot candidate */
            f = e - 1;  /* Uses different register set */
            b = a + c;
        }
        
        /* BRANCH 3: More complex condition */
        if (__builtin_expect((a ^ b) > (c & d), branch_taken)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label3:
            /* Simple move operation - good delay slot candidate */
            d = c;  /* Register move operation */
            e = f + i;
        }
        
        /* BRANCH 4: Nested condition to create deeper analysis */
        if (__builtin_expect(i % 5 == 0, 0)) {
            if (__builtin_expect(a != 0, 1)) {
                asm volatile("nop" ::: "memory");
target_label4:
                /* Arithmetic with immediate - safe and trap-free */
                f = e + 2;  /* Immediate addition, no trap risk */
                a = b >> 1; /* Shift operation, safe */
            }
        }
        
        /* Update variables to change branch outcomes */
        a = (a + b) ^ c;
        b = (b - d) | e;
        c = c + (i & 0xF);
        d = d * 3 + 1;
        
        /* Accumulate result to keep computations live */
        result += a + b + c + d + e + f;
        
        /* Prevent optimization of loop */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), 
                          "+r"(d), "+r"(e), "+r"(f));
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test case: Function with multiple return paths */
    {
        register int x REG1 = 10;
        register int y REG2 = 20;
        register int z REG3 = 30;
        
        /* Conditional with multiple delay slot opportunities */
        for (int j = 0; j < 50; j++) {
            if (__builtin_expect(x < y, j % 2)) {
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
delay_target1:
                /* Perfect delay slot candidate: independent operation */
                z = x + 5;  /* Uses different registers than condition */
                x = y - j;
            } else {
                asm volatile("nop" ::: "memory");
delay_target2:
                /* Another candidate */
                y = z * 2;
                z = x + y;
            }
            
            /* Rotate values */
            int tmp = x;
            x = y;
            y = z;
            z = tmp;
        }
        
        printf("Final values: x=%d, y=%d, z=%d\n", x, y, z);
    }
    
    return result != 0 ? 0 : 1;
}
