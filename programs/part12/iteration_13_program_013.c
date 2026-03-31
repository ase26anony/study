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
    
    /* Force many pseudo registers with complex expressions */
    for (volatile int i = 0; i < iter_count; i++) {
        /* Declare many local variables to create register pressure */
        int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
        int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
        int r20, r21, r22, r23, r24, r25, r26, r27, r28, r29;
        
        /* Complex expression that will be reused - rematerialization candidate */
        int common_expr = (a * b) + (c << 2) - d;
        
        /* First sequence of independent computations */
        r0 = a + b;
        r1 = b * c;
        r2 = c - d;
        r3 = d << 3;
        r4 = a ^ b;
        r5 = b | c;
        r6 = c & d;
        r7 = ~a;
        r8 = a * 7;
        r9 = b / 3;
        
        /* Compiler barrier to prevent reordering */
        __asm__ volatile ("" : : : "memory");
        
        /* Reuse the common expression multiple times - creates register copies */
        r10 = common_expr + r0;
        r11 = common_expr - r1;
        r12 = common_expr * r2;
        r13 = common_expr ^ r3;
        r14 = common_expr | r4;
        
        /* More independent computations */
        r15 = r0 * r1 + r2;
        r16 = r3 - r4 * r5;
        r17 = (r6 << 2) + r7;
        r18 = r8 ^ r9;
        r19 = (r10 & r11) | r12;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic blocks */
        volatile int cond = a > b;
        if (cond) {
            /* Another reuse of the common expression */
            r20 = common_expr + r13;
            r21 = common_expr - r14;
            r22 = common_expr * r15;
            
            /* More computations in this branch */
            r23 = r16 + r17 * 2;
            r24 = r18 ^ r19;
            r25 = (r20 << 1) + r21;
            
            /* Compiler barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Yet another reuse */
            r26 = common_expr + r22;
            r27 = common_expr - r23;
            
            result += r24 + r25 + r26 + r27;
        } else {
            /* Alternative path with different computations */
            r28 = common_expr * 3;
            r29 = common_expr / 2;
            
            /* More independent expressions */
            int t0 = r13 * r14 + r15;
            int t1 = r16 - r17 ^ r18;
            int t2 = r19 << 4;
            int t3 = (r28 & 0xFF) | r29;
            int t4 = t0 * t1 - t2;
            int t5 = t3 ^ t4;
            
            /* Compiler barrier */
            __asm__ volatile ("" : : : "memory");
            
            /* Final reuse of common expression */
            int t6 = common_expr + t5;
            int t7 = common_expr - t4;
            int t8 = common_expr * t3;
            
            result += t6 + t7 + t8;
        }
        
        /* Modify inputs slightly to prevent complete loop unrolling */
        a = a ^ 1;
        b = b + 1;
        
        /* Final compiler barrier in loop */
        __asm__ volatile ("" : : : "memory");
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile to prevent constant propagation */
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int iterations = 10; /* Small enough to not overflow */
    
    printf("Starting computation with: %d, %d, %d, %d\n", 
           v1, v2, v3, v4);
    
    int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
