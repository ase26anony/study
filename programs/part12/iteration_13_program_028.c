/* early_remat_trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Prevent inlining and interprocedural optimization */
__attribute__((noinline, noipa))
static int high_pressure_compute(volatile int v1, volatile int v2, 
                                 volatile int v3, volatile int v4,
                                 volatile int iterations) {
    volatile int result = 0;
    
    /* Loop with volatile iteration count to prevent unrolling */
    for (volatile int i = 0; i < iterations; i++) {
        /* Declare many local variables to create register pressure */
        int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
        int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
        int t21, t22, t23, t24, t25, t26, t27, t28, t29, t30;
        int t31, t32, t33, t34, t35, t36, t37, t38, t39, t40;
        
        /* Complex sub-expression that will be reused multiple times */
        /* This creates rematerialization candidates */
        int common_subexpr = (v1 * v2) + (v3 << 2) - v4;
        
        /* First group of independent arithmetic expressions */
        t1 = v1 + v2;
        t2 = v2 * v3;
        t3 = v3 - v4;
        t4 = v4 ^ v1;
        t5 = t1 * t2;
        t6 = t2 + t3;
        t7 = t3 | t4;
        t8 = t4 & t1;
        t9 = t5 << 2;
        t10 = t6 >> 1;
        
        /* Copy the common sub-expression multiple times */
        /* This creates register-to-register moves that remat might replace */
        t11 = common_subexpr;
        t12 = common_subexpr;
        t13 = common_subexpr;
        
        /* Compiler barrier to prevent reordering/coalescing */
        __asm__ volatile ("" : : : "memory");
        
        /* Second group with more complex operations */
        t14 = t9 * t10;
        t15 = t11 + t12;
        t16 = t13 ^ t14;
        t17 = t15 | t16;
        t18 = (t14 * 3) + (t15 << 1);
        t19 = t16 - t17;
        t20 = t18 ^ t19;
        t21 = t17 * 7;
        t22 = t18 / 3;
        t23 = t19 & 0xFF;
        
        /* More copies of the common sub-expression */
        t24 = common_subexpr;
        t25 = common_subexpr;
        
        /* Another compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Third group with bitwise operations */
        t26 = t20 ^ t21;
        t27 = t22 | t23;
        t28 = t24 + t25;
        t29 = t26 * t27;
        t30 = t28 - t29;
        t31 = (t29 << 3) | (t30 >> 2);
        t32 = t30 ^ 0xABCD;
        t33 = t31 + 12345;
        t34 = t32 * 6789;
        t35 = t33 | t34;
        
        /* Final copies of common sub-expression */
        t36 = common_subexpr;
        t37 = common_subexpr;
        t38 = common_subexpr;
        
        /* Conditional branch based on volatile variable */
        /* Splits basic blocks and complicates live ranges */
        if (v1 > v2) {
            t39 = t35 + t36;
            t40 = t37 * t38;
            result += t39 - t40;
        } else {
            t39 = t35 - t36;
            t40 = t37 / (t38 ? t38 : 1);
            result += t39 + t40;
        }
        
        /* More arithmetic to extend live ranges */
        t1 = t1 + t39;
        t2 = t2 * t40;
        t3 = t3 ^ result;
        t4 = t4 | t1;
        
        /* Final compiler barrier */
        __asm__ volatile ("" : : : "memory");
        
        /* Use all variables to prevent dead code elimination */
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        result += t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
        result += t21 + t22 + t23 + t24 + t25 + t26 + t27 + t28 + t29 + t30;
        result += t31 + t32 + t33 + t34 + t35 + t36 + t37 + t38 + t39 + t40;
    }
    
    return result;
}

int main(void) {
    srand(time(NULL));
    
    /* Use volatile variables to prevent constant propagation */
    volatile int v1 = rand() % 100 + 1;
    volatile int v2 = rand() % 100 + 1;
    volatile int v3 = rand() % 100 + 1;
    volatile int v4 = rand() % 100 + 1;
    volatile int iterations = 10; /* Small enough to run, large enough for pressure */
    
    printf("Starting high-pressure computation...\n");
    printf("Parameters: v1=%d, v2=%d, v3=%d, v4=%d, iterations=%d\n", 
           v1, v2, v3, v4, iterations);
    
    int result = high_pressure_compute(v1, v2, v3, v4, iterations);
    
    printf("Result: %d\n", result);
    
    return 0;
}
