/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -fdump-rtl-all -da -o remat_test remat_test.c */

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
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int common_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        /* First block of independent computations */
        r0 = a + b + common_expr;
        r1 = b * c - common_expr;
        r2 = (a << 3) | (b & 0xF);
        r3 = common_expr * 2 + r0;
        r4 = (c * d) ^ common_expr;
        r5 = (a + d) * common_expr;
        r6 = b << 4;
        r7 = common_expr + r6;
        r8 = (c * 3) + common_expr;
        r9 = (d & 0xAA) | common_expr;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* More computations using the same common expression */
        r10 = common_expr - a;
        r11 = (b * common_expr) >> 2;
        r12 = common_expr + (c * d);
        r13 = (a ^ b) & common_expr;
        r14 = common_expr * common_expr;
        r15 = (common_expr << 1) + r10;
        r16 = d + common_expr * 3;
        r17 = (common_expr & 0xFF) + r11;
        r18 = common_expr / (b + 1);
        r19 = (common_expr | 0xFFFF) - r12;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic blocks */
        volatile int cond = a & 1;
        if (cond) {
            /* Branch 1: More computations with common_expr */
            r20 = common_expr + r0 + r1;
            r21 = (common_expr * r2) >> 1;
            r22 = r3 ^ common_expr;
            r23 = common_expr - r4;
            r24 = (r5 & common_expr) | r6;
            r25 = common_expr * 7 + r7;
            r26 = (r8 << 2) + common_expr;
            r27 = r9 & ~common_expr;
            r28 = common_expr + (r10 * 2);
            r29 = (r11 | common_expr) ^ r12;
            
            /* Use volatile to prevent dead code elimination */
            result += r20 + r21 + r22 + r23 + r24 + 
                     r25 + r26 + r27 + r28 + r29;
        } else {
            /* Branch 2: Alternative computations still using common_expr */
            r20 = common_expr * 11;
            r21 = (common_expr + r13) & 0xFF;
            r22 = r14 - common_expr;
            r23 = (common_expr | r15) ^ 0xDEAD;
            r24 = common_expr + r16 * 2;
            r25 = (r17 & common_expr) + 0xBEEF;
            r26 = common_expr << (d & 3);
            r27 = r18 + common_expr / 2;
            r28 = (common_expr ^ r19) | 0xCAFE;
            r29 = common_expr * common_expr % 1000;
            
            result += r20 ^ r21 ^ r22 ^ r23 ^ r24 ^ 
                     r25 ^ r26 ^ r27 ^ r28 ^ r29;
        }
        
        /* Final compiler barrier in loop */
        __asm__ volatile ("" : : : "memory");
        
        /* Force reuse of common_expr across loop iterations */
        a = (a + common_expr) & 0x7FFF;
        b = (b ^ common_expr) & 0x7FFF;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile inputs to prevent constant propagation */
    volatile int v1 = rand() % 1000 + 1;
    volatile int v2 = rand() % 1000 + 1;
    volatile int v3 = rand() % 1000 + 1;
    volatile int v4 = rand() % 1000 + 1;
    volatile int iterations = 10; /* Small enough to not timeout, large enough for pressure */
    
    printf("Inputs: %d, %d, %d, %d, iterations: %d\n", 
           v1, v2, v3, v4, iterations);
    
    int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
