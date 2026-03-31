/* Program to trigger GCC's delay slot filling logic for uncovered lines in reorg.cc */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage for MIPS/SPARC architectures */
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
/* Generic register hints for other RISC architectures */
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
    register int c REG3 = 0;
    register int d REG4 = 100;
    register int e REG5 = 0;
    register int f REG6 = 0;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    volatile int counter = 0;
    
    int result = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (counter = 0; counter < iterations; counter++) {
        /* Varying branch conditions to create different paths */
        if (__builtin_expect((a > b), 0)) {
            /* Branch 1 - unlikely path with nop filler */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            /* Target label with simple arithmetic instruction */
            /* This instruction should be eligible for delay slot */
            target_label_1:
            e = d + 1;  /* Simple, trap-free, register operation */
            
            /* Continue with loop logic */
            f = e * 2;
        } else {
            /* Likely path - different computation */
            c = a + b;
        }
        
        /* Another branch with different structure */
        if (__builtin_expect((c < d), 1)) {
            /* Single nop to create different slot filling scenario */
            asm volatile("nop" ::: "memory");
            target_label_2:
            f = e + 3;  /* Another simple candidate for delay slot */
            
            /* Additional independent operation */
            a = f - 1;
        }
        
        /* Third branch pattern with more nops */
        if (__builtin_expect((d > 50), 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_3:
            b = a + 5;  /* Simple arithmetic, no resource conflicts */
            
            /* Update variables to change future branch outcomes */
            d = d - 1;
        }
        
        /* Fourth branch - test with immediate values */
        if (__builtin_expect((counter & 1), 0)) {
            /* No nops here - different pattern for filler */
            target_label_4:
            e = b + 7;  /* Immediate addition, safe operation */
        }
        
        /* Mix of operations to create register pressure variations */
        a = (a + b) ^ c;
        b = b + 1;
        c = c + d;
        
        /* Accumulate result to keep computations live */
        result += a + b + c + d + e + f;
        
        /* Prevent optimization of loop variables */
        asm volatile("" : "+r"(a), "+r"(b), "+r"(c), 
                          "+r"(d), "+r"(e), "+r"(f));
    }
    
    /* Additional complex branch structure outside loop */
    register int x REG1 = 0;
    register int y REG2 = 1000;
    register int z REG3 = 0;
    
    /* Nested conditional for more delay slot opportunities */
    for (int i = 0; i < 10; i++) {
        if (__builtin_expect((x < y), 1)) {
            /* Multiple filler instructions */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_5:
            z = x + y;  /* Simple addition, no traps */
            
            /* Follow with another safe operation */
            x = z >> 1;
        } else {
            /* Alternative path */
            y = y - x;
        }
        
        /* Another branch immediately after */
        if (__builtin_expect((z > 500), 0)) {
            /* Single filler */
            asm volatile("nop" ::: "memory");
            target_label_6:
            x = y * 2;  /* Multiplication is safe with integers */
        }
        
        x++;
        y--;
        result += x + y + z;
    }
    
    /* Final computation to use all variables */
    result = result + (a * b) - (c / (d + 1)) + (e | f);
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
