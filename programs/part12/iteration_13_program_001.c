/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iter_count)
{
    /* Force many pseudo registers with independent calculations */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
    int r30, r31, r32, r33, r34, r35, r36, r37, r38, r39;
    
    volatile int control = 0;
    int result = 0;
    
    for (volatile int i = 0; i < iter_count; i++) {
        /* Complex expression that will be reused - candidate for rematerialization */
        int common_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        /* Force many independent calculations to create register pressure */
        r0 = a + b;
        r1 = b * c;
        r2 = c - d;
        r3 = d ^ a;
        r4 = a << 3;
        r5 = b >> 2;
        r6 = c & 0x0F;
        r7 = d | 0xAA;
        r8 = a * 7;
        r9 = b + 11;
        
        /* Reuse the common expression - creates register-to-register moves */
        r10 = common_expr;  /* This copy may trigger rematerialization */
        r11 = common_expr + r0;
        r12 = common_expr - r1;
        r13 = common_expr * r2;
        
        /* Compiler barrier - prevents reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* More independent calculations */
        r14 = r3 * r4;
        r15 = r5 ^ r6;
        r16 = r7 & r8;
        r17 = r9 | r10;
        r18 = r11 << 1;
        r19 = r12 >> 2;
        r20 = r13 & 0x55;
        r21 = r14 * 3;
        r22 = r15 + 5;
        r23 = r16 - 7;
        
        /* Another reuse of the common expression */
        r24 = common_expr;  /* Another copy - potential rematerialization */
        r25 = common_expr ^ r17;
        r26 = common_expr & r18;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Control flow split to complicate live ranges */
        if (control) {
            /* Use all variables in this branch */
            r27 = r19 * r20;
            r28 = r21 ^ r22;
            r29 = r23 | r24;
            r30 = r25 & r26;
            r31 = r27 + r28;
            r32 = r29 - r30;
            r33 = r31 * r32;
            
            /* Yet another reuse */
            r34 = common_expr;  /* Another copy candidate */
            r35 = common_expr + r33;
            
            result += r35;
        } else {
            /* Alternative calculations in else branch */
            r27 = r19 + r20;
            r28 = r21 - r22;
            r29 = r23 ^ r24;
            r30 = r25 | r26;
            r31 = r27 * r28;
            r32 = r29 & r30;
            r33 = r31 ^ r32;
            
            /* Reuse again */
            r34 = common_expr;  /* Copy for rematerialization */
            r35 = common_expr - r33;
            
            result -= r35;
        }
        
        __asm__ volatile ("" : : : "memory");
        
        /* Final set of calculations using all variables */
        r36 = r0 * r1 + r2;
        r37 = r3 - r4 ^ r5;
        r38 = r6 & r7 | r8;
        r39 = r9 + r10 - r11;
        
        /* Mix results to prevent dead code elimination */
        result ^= r36 + r37 - r38 * r39;
        
        /* Modify control variable to affect branch prediction */
        control = (control + 1) & 1;
        
        /* Modify inputs slightly each iteration */
        a = (a + 1) & 0xFF;
        b = (b * 3) & 0xFF;
        c = (c - 1) & 0xFF;
        d = (d ^ i) & 0xFF;
    }
    
    return result;
}

int main(void)
{
    srand(time(NULL));
    
    /* Use volatile to prevent constant propagation */
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int iterations = 1000;  /* Force loop execution */
    
    printf("Starting high-pressure computation...\n");
    
    /* Call the high register pressure function */
    volatile int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
