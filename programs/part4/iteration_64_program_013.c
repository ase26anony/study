/* test-mcf-debug.c
 * 
 * This test aims to trigger the debug dumping code in GCC's min-cost flow solver
 * (mcf.cc) when compiled with a GCC built with internal checking enabled
 * (--enable-checking), which defines MCF_DEBUG.
 * 
 * The uncovered lines print special node labels in fixup graphs:
 *   ENTRY, ENTRY'', EXIT, EXIT'', NEW_EXIT, NEW_ENTRY
 * 
 * To trigger these, we need to create a function with such high register pressure
 * that IRA's priority allocator builds a fixup graph requiring artificial
 * source/sink nodes (new_exit_index, new_entry_index).
 */

/* Force the priority register allocator for this function */
#ifdef __GNUC__
#define PRIORITY_ALLOC __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define PRIORITY_ALLOC
#endif

/* Target ARM to limit available registers */
#ifdef __GNUC__
#define ARM_TARGET __attribute__((target("arch=armv7-a")))
#else
#define ARM_TARGET
#endif

/* Combine attributes */
#define HIGH_PRESSURE_FUNC ARM_TARGET PRIORITY_ALLOC

/* Volatile assembly to clobber many registers on ARM */
#define CLOBBER_MANY_REGS \
    asm volatile("" : : : "memory", \
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
        "r8", "r9", "r10", "r11", "r12", "r14")

/* The high-pressure function that should trigger complex fixup graph construction */
HIGH_PRESSURE_FUNC
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with volatile values to prevent constant propagation */
    volatile int seed = 42;
    v0 = seed + 1;
    v1 = seed + 2;
    v2 = seed + 3;
    v3 = seed + 4;
    v4 = seed + 5;
    v5 = seed + 6;
    v6 = seed + 7;
    v7 = seed + 8;
    v8 = seed + 9;
    v9 = seed + 10;
    v10 = seed + 11;
    v11 = seed + 12;
    v12 = seed + 13;
    v13 = seed + 14;
    v14 = seed + 15;
    v15 = seed + 16;
    
    /* Complex control flow to create intersecting live ranges */
    /* First basic block: use all variables */
    v0 = v1 + v2;
    v3 = v4 * v5;
    v6 = v7 - v8;
    v9 = v10 / (v11 ? v11 : 1);
    v12 = v13 | v14;
    v15 = v0 ^ v3;
    
    CLOBBER_MANY_REGS; /* Force spill/reload points */
    
    /* Branch point 1 - different subsets live in each path */
    if (v0 > 0) {
        /* Path A: keep v0-v7 live, kill v8-v15 */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v0 - v1;
        /* But also use some "killed" vars to extend liveness */
        v8 = v9 + 1;  /* v9 becomes live again */
        v10 = v11 * 2; /* v11 becomes live again */
    } else {
        /* Path B: keep v8-v15 live, kill v0-v7 */
        v9 = v10 + v11;
        v12 = v13 * v14;
        v15 = v8 - v9;
        /* Extend liveness of some "killed" vars */
        v0 = v1 + 1;  /* v1 becomes live again */
        v2 = v3 * 2;  /* v3 becomes live again */
    }
    
    CLOBBER_MANY_REGS;
    
    /* Another branch to create more complex CFG */
    switch (v0 & 3) {
        case 0:
            v4 = v5 + v6 + v7;
            v8 = v9 * v10;
            break;
        case 1:
            v5 = v6 - v7 - v8;
            v9 = v10 / (v11 ? v11 : 1);
            break;
        case 2:
            v6 = v7 | v8 | v9;
            v10 = v11 ^ v12;
            break;
        default:
            v7 = v8 & v9 & v10;
            v11 = v12 + v13;
            break;
    }
    
    CLOBBER_MANY_REGS;
    
    /* Loop to increase pressure further */
    for (int i = 0; i < 3; i++) {
        /* Use many variables inside loop */
        v0 = v0 + v1 + i;
        v2 = v2 * v3 * (i + 1);
        v4 = v4 - v5 - i;
        v6 = v6 | v7 | i;
        v8 = v8 ^ v9 ^ i;
        v10 = v10 + v11 + (i * 2);
        v12 = v12 * v13 * (i + 2);
        v14 = v14 - v15 - i;
    }
    
    CLOBBER_MANY_REGS;
    
    /* Final computations using all variables to ensure they're live at end */
    v0 = v1 + v2 + v3 + v4;
    v5 = v6 * v7 * v8 * v9;
    v10 = v11 - v12 - v13 - v14;
    v15 = v0 ^ v5 ^ v10;
    
    /* Use result to prevent dead code elimination */
    volatile int *sink = (volatile int *)0x1000;
    *sink = v15;
}

/* Secondary function with different pressure pattern */
HIGH_PRESSURE_FUNC
void another_high_pressure_function(void) {
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p;
    
    volatile int init = 123;
    a = init; b = init + 1; c = init + 2; d = init + 3;
    e = init + 4; f = init + 5; g = init + 6; h = init + 7;
    i = init + 8; j = init + 9; k = init + 10; l = init + 11;
    m = init + 12; n = init + 13; o = init + 14; p = init + 15;
    
    /* Nested conditionals for complex CFG */
    if (a > b) {
        if (c > d) {
            e = f + g + h;
            i = j * k * l;
        } else {
            m = n - o - p;
            a = b * c * d;
        }
        CLOBBER_MANY_REGS;
        
        for (int x = 0; x < 4; x++) {
            e = e + f + x;
            g = g * h * (x + 1);
            i = i - j - x;
            k = k | l | x;
        }
    } else {
        if (m > n) {
            o = p + a + b;
            c = d * e * f;
        } else {
            g = h - i - j;
            k = l * m * n;
        }
        
        /* Parallel computations to increase live range intersections */
        o = o + p + a + b + c + d;
        p = p * a * b * c * d * e;
    }
    
    CLOBBER_MANY_REGS;
    
    /* Force all variables to be used at the end */
    volatile int result = a + b + c + d + e + f + g + h + 
                         i + j + k + l + m + n + o + p;
    (void)result;
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* The actual execution doesn't matter for coverage.
     * Coverage happens at compile-time when GCC processes
     * the high-pressure functions. */
    return 0;
}
