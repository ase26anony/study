/* delay_slot_test.c - Target GCC's delay slot filler for MIPS/SPARC */
#include <stdio.h>
#include <stdlib.h>

/* Force register usage to avoid resource conflicts */
#define REG1 asm("$t0")
#define REG2 asm("$t1")
#define REG3 asm("$t2")
#define REG4 asm("$t3")
#define REG5 asm("$t4")
#define REG6 asm("$t5")

/* Helper to create predictable branch patterns */
#define LIKELY(x) __builtin_expect((x), 1)
#define UNLIKELY(x) __builtin_expect((x), 0)

int main(void) {
    /* Use register variables to control allocation */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    /* Volatile to prevent loop unrolling */
    volatile int iterations = 100;
    int result = 0;
    
    /* Loop with multiple branch patterns to trigger delay slot filling */
    for (int i = 0; i < iterations; i++) {
        /* Pattern 1: Branch with single nop before target */
        if (UNLIKELY(a > b)) {
            /* This branch should have delay slot filling attempted */
            asm volatile("nop" ::: "memory");
            goto target1;
        }
        /* Filler to prevent fall-through optimization */
        c = d + e;
        
    target1:
        /* Candidate for delay slot: simple register operation, no traps */
        f = a + 1;  /* Uses different regs than branch condition */
        
        /* Pattern 2: Different condition, multiple nops */
        if (LIKELY(c < d)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target2;
        }
        e = f - a;
        
    target2:
        /* Another delay slot candidate */
        b = c + 2;
        
        /* Pattern 3: Nested conditions to create complex flow */
        if (UNLIKELY((a & 1) == 0)) {
            if (LIKELY(b > 0)) {
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
                goto target3;
            }
        }
        d = e * 2;
        
    target3:
        /* Safe arithmetic - no division (could trap) */
        a = b + c;
        
        /* Pattern 4: Reverse condition */
        if (UNLIKELY(f == 0)) {
            asm volatile("nop" ::: "memory");
            goto target4;
        } else {
            asm volatile("nop" ::: "memory");
            goto target5;
        }
        
    target4:
        /* Independent operation using fresh registers */
        e = d + 3;
        goto continue_loop;
        
    target5:
        /* Another independent operation */
        c = f + 4;
        
    continue_loop:
        /* Update variables to change branch patterns */
        a = (a + 1) & 0xF;      /* Keep small to avoid overflow */
        b = (b * 3 + i) & 0xF;
        c = (c ^ d) & 0xF;
        d = (d + i) & 0xF;
        result += (a + b + c + d + e + f) & 1;
        
        /* Memory barrier to prevent reordering across iterations */
        asm volatile("" ::: "memory");
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    return result != 0;
}

/* Additional function to create more opportunities */
void helper_func(int x, int y) {
    register int r1 REG1 = x;
    register int r2 REG2 = y;
    register int r3 REG3 = 0;
    register int r4 REG4 = 0;
    
    /* Create branch with simple instruction at target */
    if (UNLIKELY(r1 > 100)) {
        asm volatile("nop" ::: "memory");
        asm volatile("nop" ::: "memory");
        goto helper_target;
    }
    
    r3 = r1 + r2;
    
helper_target:
    /* Perfect delay slot candidate: uses different registers,
       simple operation, no side effects */
    r4 = r2 + 5;
    
    /* Force usage to prevent optimization */
    asm volatile("" : "+r"(r3), "+r"(r4) ::);
}

/* Compile with:
   For MIPS: gcc -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish delay_slot_test.c
   For SPARC: gcc -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns delay_slot_test.c
   Generic: gcc -O2 -fdump-rtl-all -fno-gcse -fno-crossjumping delay_slot_test.c
*/
