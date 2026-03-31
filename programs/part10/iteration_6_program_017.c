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

/* Volatile counter to prevent loop unrolling */
static volatile int global_counter = 1000;

int main(void) {
    /* Declare registers for branch condition - separate from candidate ops */
    register int cond_a REG1 = 0;
    register int cond_b REG2 = 1;
    
    /* Registers for delay slot candidate instructions */
    register int cand_x REG3 = 100;
    register int cand_y REG4 = 200;
    register int cand_z REG5 = 0;
    register int cand_w REG6 = 0;
    
    /* Result accumulator */
    int result = 0;
    
    /* Force multiple filling attempts by varying nop counts */
    int i;
    for (i = 0; i < global_counter; i++) {
        /* Pattern 1: Branch with 1 nop before label */
        if (__builtin_expect(cond_a > cond_b, 0)) {
            asm volatile("nop" ::: "memory");
            /* Target label for branch */
            target_label_1:
            /* ELIGIBLE CANDIDATE: Simple trap-free register operation */
            cand_z = cand_x + 1;  /* Uses different regs than branch condition */
        } else {
            asm volatile("nop" ::: "memory");
        }
        
        /* Pattern 2: Branch with 2 nops, different registers */
        if (__builtin_expect(cond_b <= cond_a, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_2:
            /* Another eligible candidate */
            cand_w = cand_y - 2;  /* Different operation, different regs */
        }
        
        /* Pattern 3: Branch with no nops */
        if (__builtin_expect((cond_a & 0x1) == 0, 0)) {
            target_label_3:
            /* Simple move operation - good candidate */
            cand_x = cand_y;
        }
        
        /* Pattern 4: More complex but still trap-free */
        if (__builtin_expect(cond_b != 0, 1)) {
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            asm volatile("nop" ::: "memory");
            target_label_4:
            /* Bitwise operation - no traps */
            cand_z = cand_x & 0xFF;
        }
        
        /* Update condition variables to change branch behavior */
        cond_a = (cond_a * 3 + 1) & 0xFF;
        cond_b = (cond_b * 5 - 2) & 0xFF;
        
        /* Use results to keep computations live */
        result += cand_z + cand_w;
        
        /* Prevent optimization of delay slot candidates */
        asm volatile("" : "+r"(cand_x), "+r"(cand_y));
    }
    
    /* Additional test cases with different register sets */
    {
        register int r1 REG1 = 10;
        register int r2 REG2 = 20;
        register int r3 REG3 = 0;
        register int r4 REG4 = 0;
        
        /* Nested branches to create more opportunities */
        for (int j = 0; j < 100; j++) {
            if (__builtin_expect(r1 < r2, 0)) {
                asm volatile("nop" ::: "memory");
                target_label_5:
                /* Multiplication by constant - safe if no overflow trap */
                r3 = r4 * 3;
            }
            
            if (__builtin_expect(r2 > r1, 1)) {
                asm volatile("nop" ::: "memory");
                target_label_6:
                /* Shift operation - trap-free */
                r4 = r3 >> 2;
            }
            
            r1 = (r1 + j) & 0x7F;
            r2 = (r2 - j) & 0x7F;
        }
        
        result += r3 + r4;
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Helper function to create more branch contexts */
static void branch_helper(int a, int b) {
    register int x REG3 = a;
    register int y REG4 = b;
    register int tmp REG5 = 0;
    
    /* Multiple branches in sequence */
    if (__builtin_expect(x > y, 0)) {
        asm volatile("nop" ::: "memory");
        helper_label_1:
        tmp = x + 5;  /* Simple addition candidate */
    }
    
    if (__builtin_expect(y < x, 1)) {
        asm volatile("nop" ::: "memory");
        helper_label_2:
        tmp = y - 3;  /* Simple subtraction candidate */
    }
    
    /* Use result */
    asm volatile("" : "+r"(tmp));
}
