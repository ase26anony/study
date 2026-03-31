/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-web -fno-gcse -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -fdump-rtl-all -da -o remat_test remat_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int v1, volatile int v2, 
                                 volatile int v3, volatile int v4,
                                 volatile int iterations) {
    volatile int result = 0;
    
    /* Loop to create multiple basic blocks */
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create register pressure */
        int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
        int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int common_expr = (v1 * v2) + (v3 << 2) - v4;
        
        /* Force many independent computations to create register pressure */
        r0 = v1 + v2;
        r1 = v2 * v3;
        r2 = v3 - v4;
        r3 = v4 ^ v1;
        r4 = v1 | v2;
        r5 = v2 & v3;
        r6 = v3 + v4;
        r7 = v4 - v1;
        r8 = v1 ^ v2;
        r9 = v2 | v3;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* Reuse the common expression multiple times with different operations */
        r10 = common_expr + r0;
        r11 = common_expr - r1;
        r12 = common_expr * r2;
        r13 = common_expr & r3;
        r14 = common_expr | r4;
        
        /* More independent computations */
        r15 = r0 * r1 + r2;
        r16 = r3 - r4 ^ r5;
        r17 = r6 & r7 | r8;
        r18 = r9 + r10 - r11;
        r19 = r12 * r13 / 2;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic blocks */
        volatile int condition = v1 & 1;
        if (condition) {
            /* Another use of the common expression in the true branch */
            r20 = common_expr + r14;
            r21 = common_expr - r15;
            r22 = common_expr * r16;
            
            /* More computations in this branch */
            r23 = r17 + r18 + r19;
            r24 = r20 * r21 - r22;
            r25 = r23 ^ r24;
            r26 = r25 & common_expr;
            
            __asm__ volatile ("" : : : "memory");
            
            /* Yet another reuse of common expression */
            r27 = common_expr + r26;
            r28 = common_expr * 3;
            r29 = r27 - r28;
            
            result += r29;
        } else {
            /* Different computations in false branch, still using common_expr */
            r20 = common_expr >> 1;
            r21 = common_expr << 2;
            r22 = common_expr ^ 0xFF;
            
            r23 = r13 + r14 + r15;
            r24 = r16 * r17 - r18;
            r25 = r19 ^ r20;
            r26 = r21 & r22;
            
            __asm__ volatile ("" : : : "memory");
            
            /* More uses of common expression */
            r27 = common_expr | r26;
            r28 = common_expr & r25;
            r29 = r27 - r28;
            
            result -= r29;
        }
        
        /* Post-loop computations using all variables */
        int final1 = r0 + r1 + r2 + r3 + r4;
        int final2 = r5 + r6 + r7 + r8 + r9;
        int final3 = r10 + r11 + r12 + r13 + r14;
        int final4 = r15 + r16 + r17 + r18 + r19;
        int final5 = r20 + r21 + r22 + r23 + r24;
        int final6 = r25 + r26 + r27 + r28 + r29;
        
        /* One last use of common expression */
        int final_expr = common_expr + final1 - final2 + final3 - final4 + final5 - final6;
        
        result += final_expr;
        
        __asm__ volatile ("" : : : "memory");
        
        /* Modify volatile inputs slightly to prevent complete loop unrolling */
        v1 = v1 ^ (i & 0xFF);
        v2 = v2 + (i % 7);
    }
    
    return result;
}

/* Another high-pressure function to increase overall register pressure */
__attribute__((noinline, noipa))
static int secondary_pressure(volatile int a, volatile int b, volatile int c) {
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    int x11, x12, x13, x14, x15, x16, x17, x18, x19, x20;
    
    /* Another common expression candidate */
    int common2 = (a * b) + (c << 3) - (a ^ b);
    
    x1 = a + b;
    x2 = b * c;
    x3 = c - a;
    x4 = a ^ b;
    x5 = b | c;
    x6 = c & a;
    x7 = a + b + c;
    x8 = b - c * a;
    x9 = c ^ a ^ b;
    x10 = a | b | c;
    
    __asm__ volatile ("" : : : "memory");
    
    /* Multiple uses of common2 */
    x11 = common2 + x1;
    x12 = common2 - x2;
    x13 = common2 * x3;
    x14 = common2 & x4;
    x15 = common2 | x5;
    x16 = common2 ^ x6;
    x17 = common2 + x7;
    x18 = common2 - x8;
    x19 = common2 * x9;
    x20 = common2 & x10;
    
    __asm__ volatile ("" : : : "memory");
    
    return x11 + x12 + x13 + x14 + x15 + x16 + x17 + x18 + x19 + x20;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile to prevent constant propagation */
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int iterations = 10 + (rand() % 5);  /* Small loop to avoid timeout */
    
    printf("Starting computation with v1=%d, v2=%d, v3=%d, v4=%d, iterations=%d\n",
           v1, v2, v3, v4, iterations);
    
    /* Call the high-pressure function */
    int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    /* Call secondary function to add more global register pressure */
    int result2 = secondary_pressure(v1, v2, v3);
    
    printf("Result1: %d, Result2: %d, Total: %d\n", result, result2, result + result2);
    
    return 0;
}
