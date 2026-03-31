/* reorg_delay_slot_test.c
 * Target: GCC's delay slot filler (reorg.cc lines 2135-2149)
 * Compile with: -O2 -march=mips32 -mabi=32 -fdump-rtl-dfinish
 * or: -O3 -mcpu=v9 -fdump-rtl-reorg -fno-schedule-insns -fno-schedule-insns2
 */

#include <stdio.h>
#include <stdlib.h>

/* Force variables into specific registers to avoid resource conflicts */
#ifdef __mips__
register int cond_a asm("$2");
register int cond_b asm("$3");
register int slot_cand1 asm("$4");
register int slot_cand2 asm("$5");
register int temp1 asm("$6");
register int temp2 asm("$7");
register int loop_counter asm("$8");
#elif __sparc__
register int cond_a asm("%l0");
register int cond_b asm("%l1");
register int slot_cand1 asm("%l2");
register int slot_cand2 asm("%l3");
register int temp1 asm("%l4");
register int temp2 asm("%l5");
register int loop_counter asm("%l6");
#else
/* Generic registers - compiler will choose */
int cond_a, cond_b, slot_cand1, slot_cand2, temp1, temp2, loop_counter;
#endif

/* Volatile to prevent optimization */
volatile int iterations = 100;
volatile int seed = 42;

int main(void) {
    /* Initialize variables with distinct values */
    cond_a = 100;
    cond_b = 50;
    slot_cand1 = 1;
    slot_cand2 = 2;
    temp1 = 10;
    temp2 = 20;
    loop_counter = 0;
    
    /* Force multiple delay slot filling attempts */
    while (__builtin_expect(loop_counter < iterations, 1)) {
        /* Varying number of nops to create different filling scenarios */
        switch (loop_counter % 4) {
            case 0:
                /* Branch with 1 nop before target */
                if (__builtin_expect(cond_a > cond_b, 0)) {
                    asm volatile("nop" :::);
                    goto target_label0;
                }
                asm volatile("nop" :::);
                asm volatile("nop" :::);
                /* Candidate instruction after label - simple arithmetic */
                target_label0:
                slot_cand1 = temp1 + 1;  /* Independent of branch condition */
                break;
                
            case 1:
                /* Branch with 2 nops before target */
                if (__builtin_expect(cond_a < cond_b, 1)) {
                    asm volatile("nop" :::);
                    asm volatile("nop" :::);
                    goto target_label1;
                }
                asm volatile("nop" :::);
                /* Candidate instruction after label */
                target_label1:
                slot_cand2 = temp2 - 1;  /* Different operation, different regs */
                break;
                
            case 2:
                /* Branch with no nops before target */
                if (__builtin_expect(cond_a == cond_b, 0)) {
                    goto target_label2;
                }
                asm volatile("nop" :::);
                asm volatile("nop" :::);
                asm volatile("nop" :::);
                target_label2:
                temp1 = slot_cand1 * 2;  /* Safe multiplication (no overflow trap) */
                break;
                
            case 3:
                /* More complex pattern with multiple candidates */
                if (__builtin_expect((cond_a & 1) == 0, 1)) {
                    asm volatile("nop" :::);
                    goto target_label3a;
                }
                /* Intermediate nops to create filler opportunities */
                asm volatile("nop" :::);
                target_label3a:
                temp2 = slot_cand2 + 3;  /* First candidate */
                
                /* Second branch to same target area */
                if (__builtin_expect((cond_b & 1) == 1, 0)) {
                    goto target_label3b;
                }
                asm volatile("nop" :::);
                target_label3b:
                slot_cand1 = temp1 | 0xFF;  /* Bitwise operation - trap-free */
                break;
        }
        
        /* Update branch condition variables to change branch outcomes */
        cond_a = (cond_a * 1103515245 + 12345) & 0x7fffffff;
        cond_b = (cond_b * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Mix up the candidate registers to prevent optimization */
        if (loop_counter % 7 == 0) {
            temp1 = slot_cand1 ^ slot_cand2;
            temp2 = slot_cand1 + slot_cand2;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(loop_counter));
        loop_counter++;
    }
    
    /* Use results to prevent dead code elimination */
    int result = slot_cand1 + slot_cand2 + temp1 + temp2;
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}

/* Helper function to create additional delay slot opportunities */
static int helper_func(int x, int y) {
    int local1 = x, local2 = y;
    
    /* Another branch with delay slot candidate */
    if (__builtin_expect(local1 > local2, 0)) {
        asm volatile("nop" :::);
        goto helper_label;
    }
    asm volatile("nop" :::);
    
    helper_label:
    /* Simple move/arithmetic that could fill delay slot */
    int local3 = local1 + 5;  /* Uses different regs than main loop */
    
    return local3;
}
