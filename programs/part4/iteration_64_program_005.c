/* test-mcf-debug.c
 * 
 * This test is designed to trigger the debug dumping code in GCC's
 * min-cost flow solver (mcf.cc) when compiled with a GCC built with
 * internal checking enabled (--enable-checking).
 * 
 * The uncovered lines print special node labels (NEW_EXIT, NEW_ENTRY, etc.)
 * in dump_fixup_edge. To reach them, we need to create a function with
 * such high register pressure that IRA's priority allocator builds a
 * fixup graph requiring artificial source/sink nodes.
 */

/* Force the priority register allocator for this function */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
high_pressure_function(void)
{
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with distinct values to prevent optimization */
    v0  = 0;
    v1  = 1;
    v2  = 2;
    v3  = 3;
    v4  = 4;
    v5  = 5;
    v6  = 6;
    v7  = 7;
    v8  = 8;
    v9  = 9;
    v10 = 10;
    v11 = 11;
    v12 = 12;
    v13 = 13;
    v14 = 14;
    v15 = 15;
    
    /* Clobber many ARM registers to increase perceived pressure */
    asm volatile("" : : : "memory",
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12");
    
    /* Complex control flow with different subsets of variables live */
    /* Block 1: Use first 8 variables */
    if (v0 > 0) {
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        /* Keep them live across block boundary */
        asm volatile("" : "+r"(v0), "+r"(v1), "+r"(v2), "+r"(v3),
                         "+r"(v4), "+r"(v5), "+r"(v6), "+r"(v7));
    } else {
        /* Alternative path using different variables */
        v10 = v11 + v12;
        v13 = v14 * v15;
        asm volatile("" : "+r"(v8), "+r"(v9), "+r"(v10), "+r"(v11),
                         "+r"(v12), "+r"(v13), "+r"(v14), "+r"(v15));
    }
    
    /* Block 2: Mix variables from both paths */
    v0 = v1 + v10;
    v2 = v3 + v11;
    v4 = v5 + v12;
    v6 = v7 + v13;
    v8 = v9 + v14;
    
    /* Another clobber to force spills */
    asm volatile("" : : : "memory",
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12");
    
    /* Nested conditional to create more complex CFG */
    switch (v0 & 3) {
        case 0:
            v1 = v2 * v3;
            v4 = v5 - v6;
            break;
        case 1:
            v7 = v8 * v9;
            v10 = v11 - v12;
            break;
        case 2:
            v13 = v14 * v15;
            v0 = v1 - v2;
            break;
        default:
            v3 = v4 * v5;
            v6 = v7 - v8;
            break;
    }
    
    /* Final use of all variables to keep them live until the end */
    asm volatile("" : "+r"(v0), "+r"(v1), "+r"(v2), "+r"(v3),
                     "+r"(v4), "+r"(v5), "+r"(v6), "+r"(v7),
                     "+r"(v8), "+r"(v9), "+r"(v10), "+r"(v11),
                     "+r"(v12), "+r"(v13), "+r"(v14), "+r"(v15));
}

/* Simple main to make the file compilable */
int main(void)
{
    return 0;
}
