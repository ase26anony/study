/* test-mcf-debug.c
 * 
 * This test is designed to trigger the debug dumping code in GCC's
 * min-cost flow solver (mcf.cc) when compiled with a GCC built with
 * internal checking enabled (--enable-checking), which defines MCF_DEBUG.
 * 
 * The function `high_pressure_func` creates extreme register pressure
 * through many simultaneously live integer variables across complex
 * control flow, forcing IRA to build a fixup graph with artificial
 * source/sink nodes (new_entry_index, new_exit_index).
 * 
 * Compile with a debug-built GCC (e.g., configured with --enable-checking):
 *   gcc-debug -O2 -c test-mcf-debug.c -o test.o
 * 
 * For ARM targets (fewer registers, more pressure):
 *   gcc-debug -O2 -march=armv7-a -c test-mcf-debug.c -o test.o
 * 
 * Coverage target: mcf.cc lines 151-162 in dump_fixup_edge
 */

/* Force the priority register allocator algorithm which uses MCF */
#define USE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for fewer general-purpose registers (higher pressure) */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Function with extreme register pressure */
USE_PRIORITY_IRA ARM_TARGET
void high_pressure_func(void) {
    /* Declare many integer variables that will be simultaneously live */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with distinct values to prevent optimization */
    v0 = 0;  v1 = 1;  v2 = 2;  v3 = 3;  v4 = 4;  v5 = 5;  v6 = 6;  v7 = 7;
    v8 = 8;  v9 = 9;  v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15;
    
    /* Volatile assembly to clobber many registers and prevent optimizations */
    /* For ARM: clobber r0-r12, lr (r14) - leaving only sp (r13) and pc */
    asm volatile(""
        : /* no outputs */
        : /* no inputs */
        : "memory", 
          "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
          "r8", "r9", "r10", "r11", "r12", "r14"  /* lr */
    );
    
    /* Complex control flow with different subsets of variables live */
    /* This creates many basic blocks and complex liveness patterns */
    if (v0 > 0) {
        /* Block A: Use v0-v7 heavily */
        v1 = v0 + v2;
        v3 = v1 * v4;
        v5 = v3 - v6;
        v7 = v5 / (v2 + 1);
        
        /* Keep all v0-v15 live by using them in computations */
        v8 = v7 + v0;
        v9 = v8 * v1;
        v10 = v9 - v2;
        v11 = v10 + v3;
        v12 = v11 * v4;
        v13 = v12 - v5;
        v14 = v13 + v6;
        v15 = v14 * v7;
        
        /* Another volatile barrier to split live ranges */
        asm volatile("" ::: "memory");
    } else {
        /* Block B: Different pattern using v8-v15 more heavily */
        v8 = v15 - v14;
        v9 = v8 * v13;
        v10 = v9 + v12;
        v11 = v10 - v7;
        
        /* Mix with v0-v7 to keep them all live */
        v0 = v11 + v6;
        v1 = v0 * v5;
        v2 = v1 - v4;
        v3 = v2 + v9;
        v4 = v3 * v8;
        v5 = v4 - v10;
        v6 = v5 + v11;
        v7 = v6 * v12;
        
        asm volatile("" ::: "memory");
    }
    
    /* Nested conditional to create more control flow complexity */
    for (int i = 0; i < 3; i++) {
        switch (v0 & 3) {
            case 0:
                v1 = v2 + v3;
                v4 = v5 - v6;
                v7 = v8 * v9;
                v10 = v11 / (v12 + 1);
                break;
            case 1:
                v2 = v3 + v4;
                v5 = v6 - v7;
                v8 = v9 * v10;
                v11 = v12 / (v13 + 1);
                break;
            case 2:
                v3 = v4 + v5;
                v6 = v7 - v8;
                v9 = v10 * v11;
                v12 = v13 / (v14 + 1);
                break;
            default:
                v4 = v5 + v6;
                v7 = v8 - v9;
                v10 = v11 * v12;
                v13 = v14 / (v15 + 1);
                break;
        }
        
        /* Force all variables to be live across loop iterations */
        v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
             v9 + v10 + v11 + v12 + v13 + v14 + v15;
        
        /* Another volatile barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Final computation using all variables to ensure they're live at exit */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
                 v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Use result in a way compiler can't eliminate */
    asm volatile("" : "+r" (result) : : "memory");
}

/* Second function with different pressure pattern to increase chances */
USE_PRIORITY_IRA ARM_TARGET
void another_pressure_func(int x) {
    int a0 = x + 0, a1 = x + 1, a2 = x + 2, a3 = x + 3;
    int a4 = x + 4, a5 = x + 5, a6 = x + 6, a7 = x + 7;
    int a8 = x + 8, a9 = x + 9, a10 = x + 10, a11 = x + 11;
    int a12 = x + 12, a13 = x + 13, a14 = x + 14, a15 = x + 15;
    
    /* Deeply nested conditionals */
    if (x > 0) {
        if (x > 10) {
            if (x > 20) {
                a0 = a1 + a2;
                a3 = a4 * a5;
                a6 = a7 - a8;
                a9 = a10 / (a11 + 1);
            } else {
                a1 = a2 + a3;
                a4 = a5 * a6;
                a7 = a8 - a9;
                a10 = a11 / (a12 + 1);
            }
        } else {
            a2 = a3 + a4;
            a5 = a6 * a7;
            a8 = a9 - a10;
            a11 = a12 / (a13 + 1);
        }
    }
    
    /* Loop with many live variables */
    for (int i = 0; i < 4; i++) {
        a0 = a0 + a1;
        a1 = a1 + a2;
        a2 = a2 + a3;
        a3 = a3 + a4;
        a4 = a4 + a5;
        a5 = a5 + a6;
        a6 = a6 + a7;
        a7 = a7 + a8;
        
        asm volatile("" ::: "memory");
    }
    
    /* Force all to be used */
    asm volatile("" 
        : "+r" (a0), "+r" (a1), "+r" (a2), "+r" (a3),
          "+r" (a4), "+r" (a5), "+r" (a6), "+r" (a7),
          "+r" (a8), "+r" (a9), "+r" (a10), "+r" (a11),
          "+r" (a12), "+r" (a13), "+r" (a14), "+r" (a15)
        :
        : "memory");
}

/* Main function exists only to make the file compilable */
int main(void) {
    high_pressure_func();
    another_pressure_func(5);
    return 0;
}
