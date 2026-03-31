/* delay_slot_test.c - Target GCC's delay slot filler for MIPS/SPARC */
#include <stdio.h>
#include <stdlib.h>

/* Force register usage for predictable resource allocation */
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
/* Generic register hints for other RISC targets */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

int main() {
    /* Declare variables in registers to avoid memory ops that could trap */
    register int a REG1 = 0;
    register int b REG2 = 100;
    register int c REG3 = 0;
    register int d REG4 = 50;
    register int e REG5 = 0;
    register int f REG6 = 25;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    volatile int counter = 0;
    
    int result = 0;
    
    /* Loop creates multiple opportunities for delay slot filling */
    for (int i = 0; i < iterations; i++) {
        /* Vary branch outcomes to exercise different paths */
        counter++;
        
        /* BRANCH 1: Predictable taken branch with nop filler */
        /* This creates the 'trial' instruction (nop) that filler will examine */
        if (__builtin_expect(a < b, 1)) {
            asm volatile("nop" ::: "memory");  /* trial instruction */
            /* Target label L1: */
            L1:
            /* Candidate for delay slot: simple reg-to-reg operation */
            /* Uses different registers than branch condition (a,b) */
            e = f + 1;  /* next_trial candidate - simple, no trap */
            /* Continue with other operations */
            c = d + 2;
        }
        
        /* BRANCH 2: Predictable not-taken with varying nop count */
        /* Creates different 'slots_filled' scenarios */
        if (__builtin_expect(c > d, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            L2:
            f = e + 3;  /* Another candidate instruction */
            d = c - 1;
        }
        
        /* BRANCH 3: Mixed predictability with multiple nops */
        /* Forces filler to evaluate multiple trial positions */
        if (__builtin_expect((i % 3) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            L3:
            a = b + i;  /* Simple arithmetic, different registers */
            result += a;
        }
        
        /* BRANCH 4: Complex condition but simple target */
        /* Ensures branch is simplejump_p eligible */
        if (__builtin_expect((a ^ b) > (c & d), 0)) {
            asm volatile("nop" ::: "memory");
            L4:
            /* Multiple independent operations after label */
            b = a + 5;    /* First op - delay slot candidate */
            e = f * 2;    /* Second op - stays after delay slot fill */
            result += b;
        }
        
        /* BRANCH 5: Near 50/50 branch with minimal filler */
        /* Tests slots_to_fill != slots_filled condition */
        if (__builtin_expect((counter & 1) == 0, 0)) {
            /* Single nop - filler may try to move L5's first instruction here */
            asm volatile("nop" ::: "memory");
            L5:
            /* Perfect delay slot candidate: uses untouched registers */
            register int t1 REG5 = f;
            register int t2 REG6 = 10;
            c = t1 + t2;  /* All register ops, no memory, no trap */
            d = c + 1;
        }
        
        /* Modify variables to change future branch behavior */
        a = (a + 1) % 100;
        b = (b - 1) % 100;
        c = (c + i) % 50;
        d = (d + 2) % 50;
        
        /* Prevent optimization of loop body */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d));
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (a=%d, b=%d, c=%d, d=%d, e=%d, f=%d)\n", 
           result, a, b, c, d, e, f);
    
    return result != 0 ? 0 : 1;
}

/* Additional function to create more branch contexts */
int helper(int x, int y) {
    register int r1 REG1 = x;
    register int r2 REG2 = y;
    register int r3 REG3 = 0;
    
    /* Nested branches with different patterns */
    if (__builtin_expect(r1 > r2, 0)) {
        asm volatile("nop" ::: "memory");
        H1:
        r3 = r1 - r2;  /* Simple subtraction - good candidate */
        r1 = r3 * 2;
    } else {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        H2:
        r3 = r1 + r2;  /* Simple addition */
        r2 = r3 / 2;   /* Division but with positive numbers - safe */
    }
    
    return r3;
}
