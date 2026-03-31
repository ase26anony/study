#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static volatile int high_pressure_compute(volatile int a, volatile int b, 
                                          volatile int c, volatile int d,
                                          volatile int iterations) {
    volatile int result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        /* Declare many local variables to create register pressure */
        int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
        int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
        
        /* Complex expression that will be reused - candidate for rematerialization */
        int complex_expr = (a * b) + (c << 2) - (d & 0xFF);
        
        /* First set of independent computations using complex_expr */
        r0 = complex_expr + i;
        r1 = complex_expr - i;
        r2 = complex_expr * i;
        r3 = complex_expr & i;
        r4 = complex_expr | i;
        
        /* Compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* More computations with different operations */
        r5 = (a + b) * r0;
        r6 = (c - d) + r1;
        r7 = (a ^ b) | r2;
        r8 = (c & d) ^ r3;
        r9 = (a | b) & r4;
        
        /* Reuse complex_expr again - forcing potential register copy */
        r10 = complex_expr + r5;
        r11 = complex_expr - r6;
        r12 = complex_expr * r7;
        r13 = complex_expr & r8;
        r14 = complex_expr | r9;
        
        /* Compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic blocks */
        volatile int condition = a & 1;
        if (condition) {
            /* Another reuse of complex_expr in different basic block */
            r15 = complex_expr + r10;
            r16 = complex_expr - r11;
            r17 = complex_expr * r12;
            r18 = complex_expr & r13;
            r19 = complex_expr | r14;
            
            /* More computations in this branch */
            r20 = r15 * r16;
            r21 = r17 + r18;
            r22 = r19 ^ r20;
            r23 = r21 & r22;
            
            /* Yet another reuse */
            r24 = complex_expr + r23;
            r25 = complex_expr - r23;
        } else {
            /* Alternative computations in else branch */
            r15 = r10 * r11;
            r16 = r12 + r13;
            r17 = r14 ^ r15;
            r18 = r16 & r17;
            
            /* Reuse complex_expr here too */
            r24 = complex_expr * r18;
            r25 = complex_expr & r18;
        }
        
        /* Compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Merge point - more computations using values from both branches */
        r26 = r24 + r25;
        r27 = r24 - r25;
        r28 = r24 * r25;
        r29 = r24 & r25;
        
        /* Final reuse of complex_expr at merge point */
        int final1 = complex_expr + r26;
        int final2 = complex_expr - r27;
        int final3 = complex_expr * r28;
        int final4 = complex_expr & r29;
        
        /* Accumulate result to prevent dead code elimination */
        result += final1 + final2 + final3 + final4;
        
        /* Modify inputs slightly for next iteration */
        a ^= 0x5A5A;
        b += 0x1111;
        c -= 0x2222;
        d |= 0x3333;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile to prevent constant propagation */
    volatile int v1 = rand() % 1000 + 1;
    volatile int v2 = rand() % 1000 + 1;
    volatile int v3 = rand() % 1000 + 1;
    volatile int v4 = rand() % 1000 + 1;
    volatile int iterations = 100; /* Enough iterations but not too many */
    
    printf("Starting computation with v1=%d, v2=%d, v3=%d, v4=%d, iter=%d\n",
           v1, v2, v3, v4, iterations);
    
    volatile int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
