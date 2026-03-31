/* test-mcf-debug.c
 * 
 * This test is designed to trigger the debug dumping code in GCC's
 * min-cost flow solver (mcf.cc) when compiled with a GCC built with
 * internal checking enabled (--enable-checking), which defines MCF_DEBUG.
 * 
 * The uncovered lines are in dump_fixup_edge, which prints special node
 * labels like "NEW_EXIT" and "NEW_ENTRY" when the fixup graph contains
 * artificial source/sink nodes added during network transformation.
 * 
 * To trigger this:
 * 1. Create extreme register pressure with many simultaneously live variables
 * 2. Force use of the priority-based register allocator (which uses MCF)
 * 3. Target a register-poor architecture (ARM)
 * 4. Use volatile assembly to clobber registers and prevent optimization
 * 5. Structure control flow to create complex liveness patterns
 * 
 * Coverage occurs at compile-time when GCC's IRA builds and dumps the fixup graph.
 */

/* Force use of priority-based allocator for this function */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
high_pressure_function(void)
{
    /* Declare many integer variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with distinct values to prevent coalescing */
    v0 = 0;
    v1 = 1;
    v2 = 2;
    v3 = 3;
    v4 = 4;
    v5 = 5;
    v6 = 6;
    v7 = 7;
    v8 = 8;
    v9 = 9;
    v10 = 10;
    v11 = 11;
    v12 = 12;
    v13 = 13;
    v14 = 14;
    v15 = 15;
    
    /* Clobber many ARM registers to increase perceived pressure.
     * This tells GCC these registers are modified, forcing more spills. */
    asm volatile("" : : : "memory",
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12");
    
    /* Complex control flow with different variable usage patterns
     * to create intersecting live ranges across basic blocks */
    if (v0 > 0) {
        /* Block A: Use first subset of variables */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 / v9;
        
        /* Make all variables live by using them */
        v10 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        
        /* Another clobber to split live ranges */
        asm volatile("" : : : "memory");
        
        if (v1 > 5) {
            /* Block B: Use different subset */
            v11 = v12 * v13;
            v14 = v11 - v15;
            v0 = v14 + v2;
            
            /* Use all variables again */
            v15 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14;
        } else {
            /* Block C: Yet another subset */
            v3 = v4 * v5;
            v6 = v7 + v8;
            v9 = v10 - v11;
            
            /* Use all variables */
            v12 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        }
        
        /* Block D: Merge point with more computations */
        v13 = v0 * v1 + v2 * v3 - v4 * v5 + v6 * v7 - v8 * v9 + v10 * v11 - v12 * v13 + v14 * v15;
    } else {
        /* Block E: Alternative path with different usage */
        v2 = v3 * v4;
        v5 = v6 + v7;
        v8 = v9 - v10;
        v11 = v12 * v13;
        
        /* Use all variables */
        v14 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
        
        if (v2 < 10) {
            /* Block F: Nested with more operations */
            v15 = v0 - v1 + v2 - v3 + v4 - v5 + v6 - v7 + v8 - v9 + v10 - v11 + v12 - v13 + v14;
            
            /* Final clobber */
            asm volatile("" : : : "memory",
                         "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
    }
    
    /* Final use of all variables to keep them live until the end */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(v0), "+r"(v1), "+r"(v2), "+r"(v3),
                       "+r"(v4), "+r"(v5), "+r"(v6), "+r"(v7),
                       "+r"(v8), "+r"(v9), "+r"(v10), "+r"(v11),
                       "+r"(v12), "+r"(v13), "+r"(v14), "+r"(v15));
}

/* Secondary function with different pressure pattern to increase
 * chances of triggering the fixup graph transformation */
void __attribute__((optimize("O3", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
another_pressure_function(int x)
{
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    a0 = x;
    a1 = x + 1;
    a2 = x + 2;
    a3 = x + 3;
    a4 = x + 4;
    a5 = x + 5;
    a6 = x + 6;
    a7 = x + 7;
    a8 = x + 8;
    a9 = x + 9;
    a10 = x + 10;
    a11 = x + 11;
    a12 = x + 12;
    a13 = x + 13;
    a14 = x + 14;
    
    /* Switch statement creates multiple basic blocks with
     * different variable liveness patterns */
    switch (x & 7) {
        case 0:
            a0 = a1 + a2;
            a3 = a4 * a5;
            break;
        case 1:
            a6 = a7 - a8;
            a9 = a10 / a11;
            break;
        case 2:
            a12 = a13 | a14;
            a0 = a1 ^ a2;
            break;
        case 3:
            a3 = a4 & a5;
            a6 = a7 << 2;
            break;
        case 4:
            a8 = a9 >> 1;
            a10 = a11 + a12;
            break;
        case 5:
            a13 = a14 * a0;
            a1 = a2 - a3;
            break;
        case 6:
            a4 = a5 | a6;
            a7 = a8 ^ a9;
            break;
        default:
            a10 = a11 & a12;
            a13 = a14 << 3;
            break;
    }
    
    /* Loop to increase register pressure further */
    for (int i = 0; i < 3; i++) {
        a0 = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14;
        a1 = a0 - a2;
        a2 = a1 * a3;
        
        /* Clobber between loop iterations */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4");
    }
    
    /* Final use */
    asm volatile("" : "+r"(a0), "+r"(a1), "+r"(a2), "+r"(a3),
                       "+r"(a4), "+r"(a5), "+r"(a6), "+r"(a7),
                       "+r"(a8), "+r"(a9), "+r"(a10), "+r"(a11),
                       "+r"(a12), "+r"(a13), "+r"(a14));
}

/* Main function exists only to make the file compilable.
 * The coverage occurs during compilation, not at runtime. */
int main(void)
{
    high_pressure_function();
    another_pressure_function(42);
    return 0;
}
