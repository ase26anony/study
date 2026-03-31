/* test_mcf_coverage.c
 * 
 * This test creates a function with extreme register pressure to trigger
 * GCC's IRA min-cost flow solver to build a fixup graph with new_exit_index
 * and new_entry_index nodes. When compiled with -DMCF_DEBUG, this should
 * cause dump_fixup_edge to be called with n == fixup_graph->new_exit_index.
 */

/* Force MCF_DEBUG to be defined if not already */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

/* Function with extreme register pressure and complex live ranges */
__attribute__((noinline))
void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize with different values to prevent optimization */
    v0 = iterations * 1;
    v1 = iterations * 2;
    v2 = iterations * 3;
    v3 = iterations * 4;
    v4 = iterations * 5;
    v5 = iterations * 6;
    v6 = iterations * 7;
    v7 = iterations * 8;
    v8 = iterations * 9;
    v9 = iterations * 10;
    v10 = iterations * 11;
    v11 = iterations * 12;
    v12 = iterations * 13;
    v13 = iterations * 14;
    v14 = iterations * 15;
    v15 = iterations * 16;
    v16 = iterations * 17;
    v17 = iterations * 18;
    v18 = iterations * 19;
    v19 = iterations * 20;
    v20 = iterations * 21;
    v21 = iterations * 22;
    v22 = iterations * 23;
    v23 = iterations * 24;
    v24 = iterations * 25;
    v25 = iterations * 26;
    v26 = iterations * 27;
    v27 = iterations * 28;
    v28 = iterations * 29;
    v29 = iterations * 30;
    
    /* Nested loops to create complex live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Make many variables live across the loop */
        v0 += v1;
        v1 += v2;
        v2 += v3;
        v3 += v4;
        v4 += v5;
        v5 += v6;
        v6 += v7;
        v7 += v8;
        v8 += v9;
        v9 += v10;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 3; j++) {
            /* Complex computation keeping many values live */
            v10 = v0 + v1 + v2;
            v11 = v3 + v4 + v5;
            v12 = v6 + v7 + v8;
            v13 = v9 + v10 + v11;
            v14 = v12 + v13 + v0;
            v15 = v1 + v2 + v3;
            v16 = v4 + v5 + v6;
            v17 = v7 + v8 + v9;
            v18 = v10 + v11 + v12;
            v19 = v13 + v14 + v15;
            
            /* Use inline asm to clobber registers and increase pressure */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "add r0, r0, r1\n\t"
                "mov %0, r0\n\t"
                : "+r" (v20), "+r" (v21)
                :
                : "r0", "r1", "cc"
            );
            
            /* More computations to extend live ranges */
            v20 += v16;
            v21 += v17;
            v22 += v18;
            v23 += v19;
            v24 = v20 + v21 + v22;
            v25 = v23 + v24 + v0;
            v26 = v1 + v25 + v2;
            v27 = v3 + v26 + v4;
            v28 = v5 + v27 + v6;
            v29 = v7 + v28 + v8;
        }
        
        /* Another asm that clobbers many registers */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"
            "add r0, r0, r1\n\t"
            "add r2, r2, r3\n\t"
            "add r0, r0, r2\n\t"
            "mov %0, r0\n\t"
            : "+r" (v29), "+r" (v28), "+r" (v27), "+r" (v26)
            :
            : "r0", "r1", "r2", "r3", "cc"
        );
    }
    
    /* Final use of all variables to prevent dead code elimination */
    volatile int result = 
        v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
        v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
        v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    (void)result; /* Suppress unused warning */
}

/* Alternative function with different conflict pattern */
__attribute__((noinline))
void test_ira_conflict2(int seed) {
    /* Variables with alternating live/dead patterns */
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4;
    int e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Create a complex web of dependencies */
    for (int x = 0; x < 100; x++) {
        e = a + b;
        f = c + d;
        g = e + f;
        h = a + g;
        i = b + h;
        j = c + i;
        k = d + j;
        l = e + k;
        m = f + l;
        n = g + m;
        o = h + n;
        p = i + o;
        q = j + p;
        r = k + q;
        s = l + r;
        t = m + s;
        
        /* Force spilling by using all in a volatile asm */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"
            "mov r4, %4\n\t"
            "mov r5, %5\n\t"
            "add r0, r0, r1\n\t"
            "add r2, r2, r3\n\t"
            "add r4, r4, r5\n\t"
            "add r0, r0, r2\n\t"
            "add r0, r0, r4\n\t"
            "mov %0, r0\n\t"
            : "+r" (a), "+r" (b), "+r" (c), "+r" (d), "+r" (e), "+r" (f)
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "cc"
        );
        
        /* Rotate values to extend live ranges */
        a = b; b = c; c = d; d = e; e = f;
        f = g; g = h; h = i; i = j; j = k;
        k = l; l = m; m = n; n = o; o = p;
        p = q; q = r; r = s; s = t; t = a;
    }
}

/* Function with switch-case to create irregular control flow */
__attribute__((noinline))
int test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    switch (mode % 5) {
        case 0:
            x1 = x2 + x3;
            x4 = x5 + x6;
            x7 = x8 + x9;
            x10 = x1 + x4;
            break;
        case 1:
            x2 = x3 + x4;
            x5 = x6 + x7;
            x8 = x9 + x10;
            x1 = x2 + x5;
            break;
        case 2:
            x3 = x4 + x5;
            x6 = x7 + x8;
            x9 = x10 + x1;
            x2 = x3 + x6;
            break;
        case 3:
            x4 = x5 + x6;
            x7 = x8 + x9;
            x10 = x1 + x2;
            x3 = x4 + x7;
            break;
        case 4:
            x5 = x6 + x7;
            x8 = x9 + x10;
            x1 = x2 + x3;
            x4 = x5 + x8;
            break;
    }
    
    /* All variables live at this point */
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
}

int main() {
    /* Call test functions with different parameters to explore
     * different conflict graph configurations */
    for (int i = 0; i < 10; i++) {
        test_ira_conflict(i + 1);
        test_ira_conflict2(i * 100);
        
        /* Use result to prevent optimization */
        int r = test_ira_conflict3(i);
        printf("Iteration %d: %d\n", i, r);
    }
    
    return 0;
}
