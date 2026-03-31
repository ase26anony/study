/* Compile with: gcc -O2 -fearly-remat -fno-rename-registers -fno-tree-pre -fdump-rtl-early_remat -o remat_test remat_test.c */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int a, volatile int b, 
                                 volatile int c, volatile int d,
                                 volatile int iter_count)
{
    volatile int result = 0;
    
    /* Force many pseudo registers with independent calculations */
    for (volatile int i = 0; i < iter_count; i++) {
        /* Declare many local variables to increase register pressure */
        int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
        int r11, r12, r13, r14, r15, r16, r17, r18, r19, r20;
        int r21, r22, r23, r24, r25, r26, r27, r28, r29, r30;
        
        /* Complex expression that will be reused - potential rematerialization candidate */
        int common_expr = (a * b) + (c << 2) - d;
        
        /* First block of independent calculations */
        r1 = a + b;
        r2 = b * c;
        r3 = c - d;
        r4 = d << 3;
        r5 = a ^ b;
        r6 = b | c;
        r7 = c & d;
        r8 = a * 7;
        r9 = b * 13;
        r10 = c * 17;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* Copy the common expression multiple times - creating register-to-register moves */
        r11 = common_expr;  /* This copy might trigger rematerialization */
        r12 = common_expr;  /* Another copy of the same value */
        r13 = common_expr;  /* Yet another copy */
        
        /* More independent calculations */
        r14 = r1 * r2;
        r15 = r3 + r4;
        r16 = r5 ^ r6;
        r17 = r7 | r8;
        r18 = r9 - r10;
        r19 = r11 * 2;
        r20 = r12 + 5;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Conditional branch to split basic blocks */
        volatile int condition = a & 1;
        if (condition) {
            /* Branch 1: More calculations using the common expression */
            r21 = common_expr * 3;  /* Reuse of common_expr */
            r22 = r21 + r14;
            r23 = r15 - r16;
            r24 = r17 ^ r18;
            r25 = r19 | r20;
            
            /* More copies of common_expr in this branch */
            r26 = common_expr;
            r27 = common_expr + 1;
            
            result += r22 + r23 + r24 + r25 + r26 + r27;
        } else {
            /* Branch 2: Different calculations but still using common_expr */
            r21 = common_expr / 2;  /* Another reuse */
            r22 = r14 - r15;
            r23 = r16 * r17;
            r24 = r18 ^ r19;
            r25 = r20 | r21;
            
            /* Even more copies */
            r28 = common_expr;
            r29 = common_expr - 1;
            r30 = common_expr * common_expr;
            
            result += r22 + r23 + r24 + r25 + r28 + r29 + r30;
        }
        
        /* Force dependency chain to prevent dead code elimination */
        a = (a + 1) & 0xFF;
        b = (b * 3) & 0xFF;
        c = (c + 5) & 0xFF;
        d = (d * 7) & 0xFF;
        
        /* Final compiler barrier in loop */
        __asm__ volatile ("" : : : "memory");
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
    volatile int iterations = 100; /* Enough iterations to create pressure */
    
    printf("Starting computation with: %d, %d, %d, %d\n", v1, v2, v3, v4);
    
    int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
