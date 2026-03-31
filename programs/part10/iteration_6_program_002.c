/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register usage for MIPS/SPARC architectures */
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

/* Volatile counter to prevent loop unrolling */
static volatile int iterations = 100;

/* Simple trap-free arithmetic operations for delay slot candidates */
static inline int safe_add(int a, int b) {
    return a + b;  /* Never traps for integers */
}

static inline int safe_mul(int a, int b) {
    return a * b;  /* Safe for small values */
}

int main(void) {
    /* Declare variables with register hints to control allocation */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    int result = 0;
    int loop_count = iterations;
    
    /* Main loop to create multiple branch opportunities */
    for (int i = 0; i < loop_count; i++) {
        /* Vary branch conditions to create different paths */
        int condition = i & 0x3;
        
        /* BRANCH 1: Simple conditional with predictable outcome */
        if (__builtin_expect((a > b), 0)) {
            /* Insert nop as potential delay slot filler */
            asm volatile("nop" ::: "memory");
target_label_1:
            /* Candidate for delay slot: simple, trap-free, uses different registers */
            c = d + 1;  /* Uses REG3 = REG4 + 1, independent of branch condition */
        } else {
            a = safe_add(a, 1);
        }
        
        /* BRANCH 2: Different condition, different register set */
        if (__builtin_expect((c < d), 1)) {
            /* Multiple nops to create different slot filling scenarios */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_2:
            /* Another candidate: different operation, different registers */
            e = f * 2;  /* REG5 = REG6 * 2, trap-free */
        } else {
            b = safe_add(b, 2);
        }
        
        /* BRANCH 3: More complex condition but still simple candidate */
        if (__builtin_expect((a + b) > (c - d), condition & 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label_3:
            /* Register move operation - very simple candidate */
            f = e;  /* REG6 = REG5, just a register copy */
        } else {
            d = safe_mul(d, 3);
        }
        
        /* BRANCH 4: Nested conditions to create deeper analysis */
        if (__builtin_expect((i % 5) == 0, 0)) {
            if (__builtin_expect(e != f, 1)) {
                asm volatile("nop" ::: "memory");
target_label_4:
                /* Arithmetic with immediate - safe and simple */
                a = b + 5;  /* REG1 = REG2 + 5 */
            }
        }
        
        /* Update variables to change branch behavior */
        a = safe_add(a, i);
        b = safe_add(b, condition);
        c = safe_mul(c, (i % 3) + 1);
        
        /* Accumulate result to keep computation live */
        result += a + b + c + d + e + f;
        
        /* Force memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
    }
    
    /* Additional test case with computed goto to create jump_to_label_p */
    void* labels[] = { &&label_a, &&label_b, &&label_c };
    
    for (int j = 0; j < 3; j++) {
        /* Create a simple jump instruction */
        if (__builtin_expect(j == 1, 0)) {
            goto *labels[j];
        }
        
        continue;
        
    label_a:
        /* Simple instruction after label - delay slot candidate */
        d = e + 2;
        continue;
        
    label_b:
        f = a - 1;
        continue;
        
    label_c:
        c = b * 3;
        continue;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Helper function to ensure certain patterns aren't optimized away */
__attribute__((noinline))
static int preserve_branches(int x, int y) {
    /* Create more branch opportunities */
    if (__builtin_expect(x > y, 0)) {
        asm volatile("nop" ::: "memory");
        return x + y;
    }
    return x - y;
}
