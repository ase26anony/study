/* Program to trigger GCC's delay slot filling logic (reorg.cc lines 2135-2149) */
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
/* Generic register hints for other RISC architectures */
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
    register int d REG4 = 1;
    register int e REG5 = 2;
    register int f REG6 = 3;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    volatile int counter = 0;
    
    int result = 0;
    
    /* Loop to create multiple delay slot filling opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Vary branch outcomes to exercise different paths */
        counter++;
        
        /* BRANCH 1: Likely taken branch with nop filler */
        /* This creates a trial instruction (nop) that might be replaced */
        if (__builtin_expect(a < b, 1)) {
            asm volatile("nop" ::: "memory");
            /* Target label with simple arithmetic instruction */
            /* This is the next_trial candidate for delay slot filling */
            target1:
                /* Simple register-to-register operation, no trap possible */
                c = d + e;  /* Candidate for delay slot */
            
            /* Continue with more operations */
            f = c + 1;
        }
        
        /* BRANCH 2: Unlikely taken branch with different nop count */
        /* Creates different slots_to_fill vs slots_filled scenario */
        if (__builtin_expect(counter % 3 == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target2:
                /* Another candidate - independent register operation */
                d = e + f;
            
            a = d * 2;
        }
        
        /* BRANCH 3: Mixed predictability with minimal nops */
        if (__builtin_expect(i % 2 == 0, 0)) {
            /* Single nop creates exactly one slot to fill */
            asm volatile("nop" ::: "memory");
            target3:
                /* Safe arithmetic with immediate constant */
                e = f + 5;  /* No division, no memory access */
            
            b = e - 1;
        }
        
        /* BRANCH 4: Complex condition but simple target */
        /* Uses different registers to avoid resource conflicts */
        register int x REG1 = i;
        register int y REG2 = iterations / 2;
        
        if (__builtin_expect(x > y && x < y * 2, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target4:
                /* Completely independent operation */
                /* Uses registers not involved in branch condition */
                register int tmp1 REG3 = 10;
                register int tmp2 REG4 = 20;
                f = tmp1 + tmp2;
            
            result += f;
        }
        
        /* Modify variables to change future branch outcomes */
        a += 1;
        b -= 1;
        if (i % 10 == 0) {
            d = e;
            e = f;
            f = a;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test case with nested branches */
    {
        register int p REG1 = 0;
        register int q REG2 = 10;
        register int r REG3 = 0;
        register int s REG4 = 0;
        
        for (int j = 0; j < 50; j++) {
            /* Branch with immediate label target */
            if (__builtin_expect(p < q, 1)) {
                asm volatile("nop" ::: "memory");
                immediate_target:
                    r = s + j;  /* Simple, trap-free operation */
                
                p++;
            } else {
                s = r;
            }
            
            /* Alternate branch to same target */
            if (__builtin_expect(j % 4 == 0, 0)) {
                asm volatile("nop" ::: "memory");
                goto immediate_target;
            }
        }
        
        printf("Nested result: %d\n", r + s);
    }
    
    return 0;
}
