/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse -fearly-remat -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -da -o remat_test remat_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iter_count) {
    volatile int result = 0;
    
    /* Force many pseudo registers with independent computations */
    for (volatile int i = 0; i < iter_count; i++) {
        /* Declare many local variables to create register pressure */
        int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
        int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
        
        /* Complex sub-expression that will be reused multiple times */
        /* This creates rematerialization candidates */
        int common_subexpr = (a * b) + (c << 2) - d;
        
        /* First sequence of independent computations */
        r0 = a + b + common_subexpr;
        r1 = b * c - common_subexpr;
        r2 = (c ^ d) | common_subexpr;
        r3 = (a << 3) + common_subexpr;
        r4 = (b >> 1) * common_subexpr;
        r5 = common_subexpr + (d * 7);
        r6 = (a & b) ^ common_subexpr;
        r7 = common_subexpr - (c | d);
        r8 = (b + d) * common_subexpr;
        r9 = common_subexpr / (a + 1);
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* More computations using the same common subexpression */
        r10 = common_subexpr * 2 + r0;
        r11 = common_subexpr + r1 * 3;
        r12 = (common_subexpr << 1) | r2;
        r13 = r3 - common_subexpr;
        r14 = common_subexpr ^ r4;
        r15 = (r5 + common_subexpr) * 2;
        r16 = common_subexpr & r6;
        r17 = r7 | common_subexpr;
        r18 = common_subexpr + r8 / 2;
        r19 = r9 * common_subexpr;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Control flow split based on volatile condition */
        volatile int condition = a & 1;
        if (condition) {
            /* Branch with more computations using common_subexpr */
            r20 = common_subexpr + (r10 * r11);
            r21 = (r12 ^ common_subexpr) + r13;
            r22 = common_subexpr * r14 - r15;
            r23 = (r16 | common_subexpr) & r17;
            r24 = common_subexpr + r18 * r19;
            
            /* More distinct computations */
            r25 = (a * r20) + (b * r21);
            r26 = (c ^ r22) | (d & r23);
            r27 = (r24 << 2) + common_subexpr;
            r28 = (r25 * r26) / (common_subexpr + 1);
            r29 = (r27 ^ r28) + common_subexpr;
            
            result += r20 + r21 + r22 + r23 + r24 + 
                     r25 + r26 + r27 + r28 + r29;
        } else {
            /* Alternative branch with different computations */
            r20 = common_subexpr * r10 - r11;
            r21 = (r12 & common_subexpr) | r13;
            r22 = common_subexpr + r14 * r15;
            r23 = (r16 ^ common_subexpr) - r17;
            r24 = common_subexpr / (r18 + r19);
            
            /* Force more register pressure */
            int t0 = (b * r20) + common_subexpr;
            int t1 = (c | r21) ^ common_subexpr;
            int t2 = common_subexpr * (d + r22);
            int t3 = (r23 << 1) + common_subexpr;
            int t4 = (r24 & common_subexpr) * 3;
            
            /* Use all variables to keep them live */
            result += r20 - r21 + r22 * r23 - r24 +
                     t0 + t1 + t2 + t3 + t4;
        }
        
        /* Final compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Use all variables in final computation to prevent dead code elimination */
        int final_sum = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                       r10 + r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19;
        
        result += final_sum & 0xFF;  /* Prevent overflow */
        
        /* Modify inputs slightly to prevent loop unrolling */
        a = (a + 1) & 0x7F;
        b = (b * 3) & 0x7F;
        c = (c ^ i) & 0x7F;
        d = (d - 1) & 0x7F;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile inputs to prevent constant propagation */
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int iterations = 10;  /* Small enough to not timeout, large enough for pressure */
    
    printf("Starting computation with inputs: %d, %d, %d, %d\n", 
           v1, v2, v3, v4);
    
    int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
