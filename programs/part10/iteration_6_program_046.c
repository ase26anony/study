/* delay_slot_test.c - Target GCC's reorg.cc delay slot filler */
#include <stdio.h>
#include <stdlib.h>

/* Force specific register usage for MIPS/SPARC architectures */
#ifdef __mips__
#define REG1 asm("$t0")
#define REG2 asm("$t1")
#define REG3 asm("$t2")
#define REG4 asm("$t3")
#define REG5 asm("$t4")
#elif __sparc__
#define REG1 asm("%l0")
#define REG2 asm("%l1")
#define REG3 asm("%l2")
#define REG4 asm("%l3")
#define REG5 asm("%l4")
#else
/* Generic register hints */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#endif

int main() {
    /* Use register variables to avoid memory operations that might trap */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    int result = 0;
    
    /* Loop with multiple branches to give delay slot filler many attempts */
    for (int i = 0; i < iterations; i++) {
        /* BRANCH 1: Predictable taken branch with nop filler */
        if (__builtin_expect(a < b, 1)) {
            /* Multiple nops to create filler opportunities */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label1;
        }
        asm volatile("nop" ::: "memory");
        
    target_label1:
        /* ELIGIBLE DELAY SLOT CANDIDATE:
           Simple register operation, no traps, no resource conflicts */
        d = e + 1;  /* Uses different registers than branch condition (a,b) */
        
        /* BRANCH 2: Predictable not-taken branch with different structure */
        if (__builtin_expect(c > d, 0)) {
            asm volatile("nop" ::: "memory");
            goto target_label2;
        }
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        
    target_label2:
        /* Another eligible candidate using different registers */
        a = b + c;  /* Simple arithmetic, no memory access */
        
        /* BRANCH 3: Variable outcome to prevent optimization */
        if (__builtin_expect((i & 1) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label3;
        }
        
    target_label3:
        /* Candidate that sets a register not used in upcoming condition */
        e = d + 2;
        
        /* BRANCH 4: Complex condition but simple target instruction */
        if (__builtin_expect((a + b) > (c - d), 1)) {
            asm volatile("nop" ::: "memory");
            goto target_label4;
        }
        asm volatile("nop" ::: "memory");
        
    target_label4:
        /* Final candidate - register move operation */
        c = a;
        
        /* Update variables to change branch outcomes */
        a = (a + 1) & 0xFF;
        b = (b * 3) & 0xFF;
        c = (c + i) & 0xFF;
        d = (d - 1) & 0xFF;
        e = (e ^ i) & 0xFF;
        
        result += a + b + c + d + e;
    }
    
    printf("Result: %d\n", result);
    return result != 0;
}
