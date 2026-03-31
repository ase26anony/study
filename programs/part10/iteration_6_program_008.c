/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>
#include <stdlib.h>

/* Force register allocation for MIPS/SPARC-like architectures */
#ifdef __mips__
#define REG1 asm("$t0")
#define REG2 asm("$t1")
#define REG3 asm("$t2")
#define REG4 asm("$t3")
#define REG5 asm("$t4")
#define REG6 asm("$t5")
#else
/* Generic register hints for RISC architectures */
#define REG1
#define REG2
#define REG3
#define REG4
#define REG5
#define REG6
#endif

/* Volatile counter to prevent loop unrolling */
static volatile int global_counter = 1000;

/* Simple trap-free arithmetic operations for delay slot candidates */
static inline int safe_add(int a, int b) {
    return a + b;  /* No trap possible for integer addition */
}

static inline int safe_mul(int a, int b) {
    return a * b;  /* Multiplication won't trap for small integers */
}

int main(void) {
    /* Declare variables with register hints to avoid resource conflicts */
    register int a REG1 = 0;
    register int b REG2 = 1;
    register int c REG3 = 2;
    register int d REG4 = 3;
    register int e REG5 = 4;
    register int f REG6 = 5;
    
    /* Result accumulator to keep computations live */
    int result = 0;
    
    /* Volatile iteration count to preserve branch structure */
    volatile int iterations = 100;
    
    /* Loop with multiple conditional branches targeting different labels */
    for (int i = 0; i < iterations; i++) {
        /* Vary branch conditions to create different execution paths */
        int condition = (i & 1) ? a > b : b < c;
        
        /* BRANCH 1: Single nop before target, simple arithmetic at label */
        if (__builtin_expect(condition, 0)) {
            /* This nop creates a filler instruction that delay slot filler may replace */
            asm volatile("nop" ::: "memory");
            goto target_label_1;
        }
        
        /* Some intermediate computation using different registers */
        d = safe_add(e, f);
        
        /* BRANCH 2: Multiple nops to create different slot filling scenarios */
        if (__builtin_expect(a != b, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label_2;
        }
        
        /* Continue with more code to prevent fall-through optimization */
        c = safe_add(a, 1);
        
        /* Skip to avoid executing label code unintentionally */
        goto continue_loop;
        
    target_label_1:
        /* DELAY SLOT CANDIDATE 1: Simple, trap-free, register-only operation
         * Uses registers not involved in branch condition (REG3, REG4, REG5)
         * This should be eligible for delay slot filling */
        c = d + e;  /* Simple addition, no function call, no memory access */
        
        /* Follow-up computation to keep value live */
        result += c;
        goto continue_loop;
        
    target_label_2:
        /* DELAY SLOT CANDIDATE 2: Another simple operation
         * Uses different registers than branch condition */
        f = a + 3;  /* Immediate constant addition, trap-free */
        
        /* More computations to prevent dead code elimination */
        result += f * 2;
        
    continue_loop:
        /* Update variables to change branch outcomes */
        a = safe_add(a, i);
        b = safe_add(b, 1);
        
        /* Prevent optimization of loop counter */
        if (global_counter > 0) {
            iterations = (iterations > 1) ? iterations - 1 : 100;
        }
        
        /* Additional branch with different structure */
        if (__builtin_expect((i % 3) == 0, 0)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            goto target_label_3;
        }
        
        /* More code to separate labels */
        e = safe_mul(d, 2);
        goto loop_end;
        
    target_label_3:
        /* DELAY SLOT CANDIDATE 3: Another independent operation */
        d = c + 7;  /* Simple addition with constant */
        result += d;
        
    loop_end:
        /* Ensure all variables are used to prevent optimization */
        volatile int temp = a + b + c + d + e + f;
        (void)temp;
    }
    
    /* Nested loop with different branch pattern */
    {
        register int x REG1 = 10;
        register int y REG2 = 20;
        register int z REG3 = 0;
        
        for (int j = 0; j < 50; j++) {
            /* Branch with predictable pattern */
            if (__builtin_expect(x < y, 1)) {
                /* Variable number of nops to test slots_to_fill logic */
                for (int k = 0; k < (j % 3); k++) {
                    asm volatile("nop" ::: "memory");
                }
                goto nested_label;
            }
            
            /* Alternative path */
            y = safe_add(y, 1);
            continue;
            
        nested_label:
            /* Candidate instruction: register move operation */
            z = x;  /* Simple register-to-register move, no traps */
            
            /* Update variables */
            x = safe_add(x, j);
            y = safe_sub(y, 1);
        }
        result += z;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Helper function to avoid inlining issues */
static int safe_sub(int a, int b) {
    return a - b;  /* Subtraction won't trap */
}
