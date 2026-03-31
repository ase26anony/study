/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver
 * during register allocation, specifically to exercise the 
 * dump_fixup_edge function with new_exit_index and new_entry_index.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of mcf debugging by defining MCF_DEBUG if not already defined */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with complex register pressure to trigger fixup graph creation */
__attribute__((noinline))
static void test_ira_conflict(int iterations) {
    /* Create many variables with overlapping live ranges */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize all variables to create many definitions */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across loop iterations */
        a += b; b += c; c += d; d += e; e += f;
        f += g; g += h; h += i; i += j; j += k;
        
        /* Inner loop with more overlapping live ranges */
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
            k = l + m;
            l = m + n;
            m = n + o;
            n = o + p;
            o = p + q;
            p = q + r;
            q = r + s;
            r = s + t;
            s = t + a;
            t = a + b;
            
            /* Force register pressure with inline asm that clobbers many registers */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mov r2, %2\n\t"
                "mov r3, %3\n\t"
                "add r0, r0, r1\n\t"
                "add r2, r2, r3\n\t"
                "mul r0, r0, r2\n\t"
                : 
                : "r" (k), "r" (l), "r" (m), "r" (n)
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
        }
        
        /* More computations to extend live ranges */
        a = b * c;
        b = c * d;
        c = d * e;
        d = e * f;
        e = f * g;
        
        /* Another asm statement clobbering different registers */
        asm volatile (
            "mov r4, %0\n\t"
            "mov r5, %1\n\t"
            "mov r6, %2\n\t"
            "mov r7, %3\n\t"
            "sub r4, r4, r5\n\t"
            "sub r6, r6, r7\n\t"
            : 
            : "r" (g), "r" (h), "r" (i), "r" (j)
            : "r4", "r5", "r6", "r7", "cc", "memory"
        );
    }
    
    /* Final use of all variables to prevent optimization */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Second test function with different register pressure pattern */
__attribute__((noinline))
static void test_ira_conflict2(void) {
    /* Create a scenario with more pseudo-registers than physical registers */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    int v26 = 26, v27 = 27, v28 = 28, v29 = 29, v30 = 30;
    
    /* Complex dependency chain */
    for (int i = 0; i < 100; i++) {
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v10;
        v9 = v10 + v11;
        v10 = v11 + v12;
        v11 = v12 + v13;
        v12 = v13 + v14;
        v13 = v14 + v15;
        v14 = v15 + v16;
        v15 = v16 + v17;
        v16 = v17 + v18;
        v17 = v18 + v19;
        v18 = v19 + v20;
        v19 = v20 + v21;
        v20 = v21 + v22;
        v21 = v22 + v23;
        v22 = v23 + v24;
        v23 = v24 + v25;
        v24 = v25 + v26;
        v25 = v26 + v27;
        v26 = v27 + v28;
        v27 = v28 + v29;
        v28 = v29 + v30;
        v29 = v30 + v1;
        v30 = v1 + v2;
        
        /* Force spilling with a large clobber list */
        asm volatile (
            "/* Clobber many registers */\n\t"
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"
            "mov r4, %4\n\t"
            "mov r5, %5\n\t"
            "mov r6, %6\n\t"
            "mov r7, %7\n\t"
            "mov r8, %8\n\t"
            "mov r9, %9\n\t"
            : 
            : "r" (v1), "r" (v2), "r" (v3), "r" (v4), 
              "r" (v5), "r" (v6), "r" (v7), "r" (v8),
              "r" (v9), "r" (v10)
            : "r0", "r1", "r2", "r3", "r4", "r5", 
              "r6", "r7", "r8", "r9", "cc", "memory"
        );
    }
    
    /* Prevent dead code elimination */
    volatile int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    (void)sum;
}

/* Third test: Function with conditional flow creating imbalance */
__attribute__((noinline))
static void test_ira_imbalance(int flag) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    /* Different execution paths create different register demands */
    if (flag) {
        /* Path A: Many computations */
        for (int i = 0; i < 50; i++) {
            x1 = x2 * x3;
            x2 = x3 * x4;
            x3 = x4 * x5;
            x4 = x5 * x6;
            x5 = x6 * x7;
            x6 = x7 * x8;
            x7 = x8 * x9;
            x8 = x9 * x10;
            x9 = x10 * x1;
            x10 = x1 * x2;
        }
    } else {
        /* Path B: Different computation pattern */
        for (int i = 0; i < 50; i++) {
            x1 = x10 - x9;
            x2 = x9 - x8;
            x3 = x8 - x7;
            x4 = x7 - x6;
            x5 = x6 - x5;
            x6 = x5 - x4;
            x7 = x4 - x3;
            x8 = x3 - x2;
            x9 = x2 - x1;
            x10 = x1 - x10;
        }
    }
    
    /* Merge point: all variables live again */
    volatile int total = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    
    /* Force register pressure at merge point */
    asm volatile (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        "mov r3, %3\n\t"
        "mov r4, %4\n\t"
        "mov r5, %5\n\t"
        : 
        : "r" (x1), "r" (x2), "r" (x3), "r" (x4), "r" (x5), "r" (x6)
        : "r0", "r1", "r2", "r3", "r4", "r5", "cc", "memory"
    );
    
    (void)total;
}

/* Main function to drive different test scenarios */
int main(void) {
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    
    /* Test 1: High register pressure with loops */
    test_ira_conflict(100);
    
    /* Test 2: Even more variables */
    test_ira_conflict2();
    
    /* Test 3: Control flow imbalance */
    for (int i = 0; i < 10; i++) {
        test_ira_imbalance(i & 1);
    }
    
    return 0;
}
