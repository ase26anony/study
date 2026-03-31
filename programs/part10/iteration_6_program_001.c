/* delay_slot_test.c
 * Designed to trigger GCC's delay slot filler logic in reorg.cc lines 2135-2149
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>
#include <stdlib.h>

/* Force variables into specific registers to avoid resource conflicts */
#ifdef __mips__
#define REG1 "t0"
#define REG2 "t1"
#define REG3 "t2"
#define REG4 "t3"
#define REG5 "t4"
#define REG6 "t5"
#elif __sparc__
#define REG1 "l0"
#define REG2 "l1"
#define REG3 "l2"
#define REG4 "l3"
#define REG5 "l4"
#define REG6 "l5"
#else
/* Generic register hints - compiler will allocate appropriately */
#define REG1 "r0"
#define REG2 "r1"
#define REG3 "r2"
#define REG4 "r3"
#define REG5 "r4"
#define REG6 "r5"
#endif

/* Simple trap-free arithmetic operations for delay slot candidates */
static inline int safe_add(int a, int b) {
    return a + b;  /* Never traps for integers */
}

static inline int safe_mul(int a, int b) {
    return a * b;  /* Safe for small values */
}

int main(void) {
    /* Use register variables to control allocation and avoid spills */
    register int cond_a asm(REG1) = 0;
    register int cond_b asm(REG2) = 100;
    register int candidate_x asm(REG3) = 1;
    register int candidate_y asm(REG4) = 2;
    register int result1 asm(REG5) = 0;
    register int result2 asm(REG6) = 0;
    
    /* Volatile to prevent loop unrolling and preserve branch structure */
    volatile int iterations = 100;
    volatile int counter = 0;
    
    /* Main loop with multiple branches for delay slot filling attempts */
    for (int i = 0; i < iterations; i++) {
        /* Vary the branch condition to create different execution paths */
        cond_a = i % 50;
        cond_b = 25 + (i % 25);
        
        /* BRANCH 1: Simple conditional with predictable pattern */
        if (__builtin_expect(cond_a > cond_b, 0)) {
            /* Insert nop filler that delay slot filler may replace */
            asm volatile("nop" ::: "memory");
            /* Target label for branch - instruction here is delay slot candidate */
target_label1:
            /* Simple, trap-free, non-conflicting operation */
            result1 = safe_add(candidate_x, 1);
            /* Ensure this doesn't become a jump or sequence */
            candidate_x = result1;
        } else {
            /* Alternative path to balance branch prediction */
            result1 = safe_add(candidate_x, 2);
        }
        
        /* BRANCH 2: Different condition, different registers */
        if (__builtin_expect(candidate_y < 1000, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");  /* Two nops for multiple slot attempts */
target_label2:
            /* Another independent, safe operation */
            result2 = safe_mul(candidate_y, 3);
            candidate_y = result2 + 1;
        }
        
        /* BRANCH 3: Nested condition for complex flow */
        if (__builtin_expect((cond_a * 2) > cond_b, 0)) {
            asm volatile("nop" ::: "memory");
target_label3:
            /* Register move operation - very simple candidate */
            int temp = candidate_x;
            candidate_x = candidate_y;
            candidate_y = temp;
        }
        
        /* BRANCH 4: Loop-dependent condition */
        if (__builtin_expect(counter % 10 == 0, 1)) {
            /* Multiple nops to create different filling scenarios */
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
target_label4:
            /* Simple arithmetic with immediate - won't trap */
            result1 = result1 + 5;
            result2 = result2 - 3;
        }
        
        /* Update variables to change future branch behavior */
        counter++;
        candidate_x = (candidate_x * 3) % 1000;
        candidate_y = (candidate_y + 7) % 1000;
        
        /* Prevent optimization of loop body */
        asm volatile("" : "+r"(cond_a), "+r"(cond_b), 
                          "+r"(candidate_x), "+r"(candidate_y),
                          "+r"(result1), "+r"(result2));
    }
    
    /* Use results to prevent dead code elimination */
    int final_result = result1 + result2 + candidate_x + candidate_y;
    
    printf("Final result: %d\n", final_result);
    printf("Counter: %d\n", counter);
    
    /* Additional test case: switch statement with labels */
    {
        register int switch_var asm(REG1) = final_result % 4;
        
        switch (switch_var) {
            case 0:
                asm volatile("nop" ::: "memory");
case_label0:
                result1 = safe_add(result1, 10);
                break;
            case 1:
                asm volatile("nop" ::: "memory");
case_label1:
                result2 = safe_add(result2, 20);
                break;
            case 2:
                asm volatile("nop" ::: "memory");
case_label2:
                candidate_x = safe_mul(candidate_x, 2);
                break;
            default:
                asm volatile("nop" ::: "memory");
case_label3:
                candidate_y = safe_mul(candidate_y, 2);
                break;
        }
    }
    
    return final_result > 0 ? 0 : 1;
}

/* Helper function to create more complex control flow */
static int recursive_branch(int n, int depth) {
    if (depth <= 0) return n;
    
    /* Conditional with inline asm nop */
    if (__builtin_expect(n % 2 == 0, 1)) {
        asm volatile("nop" ::: "memory");
recursive_label:
        /* Simple operation at branch target */
        int temp = n * 3 + 1;
        return recursive_branch(temp, depth - 1);
    } else {
        asm volatile("nop" ::: "memory");
        return recursive_branch(n / 2, depth - 1);
    }
}
