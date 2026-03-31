/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for MIPS/SPARC */
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
/* Generic register hints */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

/* Volatile to prevent optimization */
static volatile int g_iterations = 100;

/* Simple trap-free arithmetic operations */
static inline int safe_add(int a, int b) {
    return a + b;  /* No overflow check - compiler knows it's trap-free */
}

static inline int safe_mul(int a, int b) {
    return a * b;  /* For MIPS/SPARC, integer multiply doesn't trap */
}

int main(void) {
    /* Declare registers with specific allocation hints */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    /* Result accumulator */
    int result = 0;
    
    /* Volatile counter to prevent loop unrolling */
    volatile int counter = g_iterations;
    
    /* Create multiple branches with different patterns */
    for (int i = 0; i < counter; i++) {
        /* Pattern 1: Branch with nop filler before target */
        if (__builtin_expect(a > b, 0)) {
            /* Insert nop that delay slot filler might replace */
            asm volatile("nop" ::: "memory");
target_label_1:
            /* Candidate for delay slot: simple, independent operation */
            e = f + 1;  /* Uses different registers than branch condition */
        } else {
            /* Fall-through path */
            a = safe_add(a, i);
        }
        
        /* Pattern 2: Different branch condition, different filler count */
        if (__builtin_expect(c < d, 1)) {
            /* Two nops - filler will try multiple trials */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_2:
            /* Another candidate - register move operation */
            f = e;  /* Simple move, no traps */
        } else {
            c = safe_add(c, 2);
        }
        
        /* Pattern 3: More complex but still trap-free */
        if (__builtin_expect((a & 0xF) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_3:
            /* Safe multiplication - no division (which could trap) */
            d = safe_mul(e, 2);
        }
        
        /* Pattern 4: Nested conditions to create more opportunities */
        if (__builtin_expect(b != 0, 1)) {
            if (__builtin_expect(a % 3 == 0, 0)) {  /* Modulo by constant is safe */
                asm volatile("nop" ::: "memory");
target_label_4:
                /* Immediate operation - very simple */
                f = 42;
            }
        }
        
        /* Update variables to change branch patterns */
        a = safe_add(a, b);
        b = safe_add(b, 1);
        c = safe_add(c, a);
        d = safe_add(d, b);
        
        /* Mix in some bit operations (trap-free) */
        e = (e << 1) | 1;
        f = f ^ (i & 0xFF);
        
        /* Accumulate result to keep computations live */
        result += (a + b + c + d + e + f) & 0xFF;
    }
    
    /* Additional test case: Switch statement with labels */
    switch (result & 0x3) {
        case 0:
            if (__builtin_expect(e > 100, 0)) {
                asm volatile("nop" ::: "memory");
switch_target_0:
                a = b + c;  /* Another delay slot candidate */
            }
            break;
        case 1:
            if (__builtin_expect(f < 50, 1)) {
                asm volatile("nop" ::: "memory");
                asm volatile("nop" ::: "memory");
switch_target_1:
                d = e - 1;  /* Subtraction is trap-free */
            }
            break;
    }
    
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}

/* Helper function to create more label opportunities */
static void branch_pattern(int x, int y, int *out) {
    /* Force spill/reload to create more RTL opportunities */
    register int r1 REG1 = x;
    register int r2 REG2 = y;
    register int r3 REG3 = 0;
    
    if (__builtin_expect(r1 > r2, 0)) {
        asm volatile("nop" ::: "memory");
helper_label:
        /* Independent operation using different register set */
        r3 = r1 & 0xFF;  /* Bitwise AND - trap-free */
    }
    
    *out = r3;
}
