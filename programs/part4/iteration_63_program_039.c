/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the debug dumping code
 * that prints special node labels like "NEW_EXIT" and "NEW_ENTRY".
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * 
 * The -DMCF_DEBUG flag is crucial to enable the debug printing code.
 */

/* Force inclusion of MCF debugging by defining the macro */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function designed to create complex register pressure and overlapping
 * live ranges that will force IRA to build a complex conflict graph
 * requiring fixup edges with NEW_EXIT/NEW_ENTRY nodes.
 */
int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int result = 0;
    
    /* Initialize all variables with different values to prevent optimization */
    a = iterations * 1;
    b = iterations * 2;
    c = iterations * 3;
    d = iterations * 4;
    e = iterations * 5;
    f = iterations * 6;
    g = iterations * 7;
    h = iterations * 8;
    i = iterations * 9;
    j = iterations * 10;
    k = iterations * 11;
    l = iterations * 12;
    m = iterations * 13;
    n = iterations * 14;
    o = iterations * 15;
    p = iterations * 16;
    q = iterations * 17;
    r = iterations * 18;
    s = iterations * 19;
    t = iterations * 20;
    u = iterations * 21;
    v = iterations * 22;
    w = iterations * 23;
    x = iterations * 24;
    y = iterations * 25;
    z = iterations * 26;
    
    /* Nested loops to create complex control flow and overlapping live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the entire outer loop */
        a += outer;
        b += a;
        c += b;
        
        /* Inner loop with its own set of live variables */
        for (int inner = 0; inner < 10; inner++) {
            /* More computations creating dependencies */
            d = a + b + inner;
            e = c + d + inner;
            f = e + d + inner;
            g = f + e + inner;
            h = g + f + inner;
            
            /* Volatile asm to clobber many registers and increase pressure */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
            
            /* More computations keeping many variables live */
            i = h + g + inner;
            j = i + h + inner;
            k = j + i + inner;
            l = k + j + inner;
            m = l + k + inner;
            
            /* Another volatile asm to force spills */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5"
            );
            
            n = m + l + inner;
            o = n + m + inner;
            p = o + n + inner;
            q = p + o + inner;
            r = q + p + inner;
        }
        
        /* More computations in outer loop */
        s = r + q + outer;
        t = s + r + outer;
        u = t + s + outer;
        v = u + t + outer;
        w = v + u + outer;
        x = w + v + outer;
        y = x + w + outer;
        z = y + x + outer;
        
        /* Accumulate result from all variables */
        result += a + b + c + d + e + f + g + h + i + j + 
                  k + l + m + n + o + p + q + r + s + t + 
                  u + v + w + x + y + z;
    }
    
    /* Final computation using all variables */
    result = result + a - b + c - d + e - f + g - h + i - j + 
             k - l + m - n + o - p + q - r + s - t + 
             u - v + w - x + y - z;
    
    return result;
}

/* Second test function with different variable usage pattern
 * to explore different conflict graph configurations
 */
int test_ira_conflict2(int seed) {
    /* Declare another set of variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize with seed-dependent values */
    v1 = seed;
    v2 = seed * 2;
    v3 = seed * 3;
    v4 = seed * 4;
    v5 = seed * 5;
    v6 = seed * 6;
    v7 = seed * 7;
    v8 = seed * 8;
    v9 = seed * 9;
    v10 = seed * 10;
    v11 = seed * 11;
    v12 = seed * 12;
    v13 = seed * 13;
    v14 = seed * 14;
    v15 = seed * 15;
    v16 = seed * 16;
    v17 = seed * 17;
    v18 = seed * 18;
    v19 = seed * 19;
    v20 = seed * 20;
    
    /* Complex conditional flow to create irregular live ranges */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            v1 += v2;
            v3 += v4;
            v5 += v6;
            
            /* Asm that clobbers specific registers */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                : 
                : "r" (v1), "r" (v2)
                : "r0", "r1", "memory"
            );
        } else if (i % 3 == 1) {
            v7 += v8;
            v9 += v10;
            v11 += v12;
            
            asm volatile (
                "mov r2, %0\n\t"
                "mov r3, %1\n\t"
                : 
                : "r" (v7), "r" (v8)
                : "r2", "r3", "memory"
            );
        } else {
            v13 += v14;
            v15 += v16;
            v17 += v18;
            
            asm volatile (
                "mov r4, %0\n\t"
                "mov r5, %1\n\t"
                : 
                : "r" (v13), "r" (v14)
                : "r4", "r5", "memory"
            );
        }
        
        /* All variables used together periodically */
        if (i % 10 == 0) {
            v19 = v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17;
            v20 = v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18;
            
            /* Major register pressure point */
            asm volatile (
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
        }
    }
    
    /* Final use of all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Third test: Function with parameters and return values
 * to create calling convention pressure
 */
int test_ira_conflict3(int p1, int p2, int p3, int p4, int p5,
                       int p6, int p7, int p8, int p9, int p10) {
    /* Local variables that interact with parameters */
    int l1 = p1 + p2;
    int l2 = p3 + p4;
    int l3 = p5 + p6;
    int l4 = p7 + p8;
    int l5 = p9 + p10;
    
    int l6, l7, l8, l9, l10;
    int l11, l12, l13, l14, l15;
    
    /* Unrolled loop to increase register pressure */
    for (int i = 0; i < 50; i++) {
        /* Phase 1: Use parameters and first set of locals */
        l6 = l1 + p1 + i;
        l7 = l2 + p2 + i;
        l8 = l3 + p3 + i;
        l9 = l4 + p4 + i;
        l10 = l5 + p5 + i;
        
        /* Phase 2: Chain computations */
        l11 = l6 + l7;
        l12 = l8 + l9;
        l13 = l10 + l1;
        l14 = l2 + l3;
        l15 = l4 + l5;
        
        /* Phase 3: Mix everything */
        l1 = l11 + l12 + i;
        l2 = l13 + l14 + i;
        l3 = l15 + l6 + i;
        l4 = l7 + l8 + i;
        l5 = l9 + l10 + i;
        
        /* Heavy asm clobber to force register shuffling */
        if (i % 5 == 0) {
            asm volatile (
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                "nop\n\t" "nop\n\t" "nop\n\t" "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
        }
    }
    
    /* Return value using all variables */
    return l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10 +
           l11 + l12 + l13 + l14 + l15 + p1 + p2 + p3 + p4 + p5 +
           p6 + p7 + p8 + p9 + p10;
}

/* Main function that calls all test cases with different parameters
 * to explore various conflict graph configurations
 */
int main() {
    int total = 0;
    
    /* Call first test multiple times with different iteration counts
     * to create different graph sizes
     */
    for (int i = 1; i <= 5; i++) {
        total += test_ira_conflict(i * 10);
    }
    
    /* Call second test with different seeds */
    for (int i = 0; i < 10; i++) {
        total += test_ira_conflict2(i * 100);
    }
    
    /* Call third test with many parameters */
    total += test_ira_conflict3(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    total += test_ira_conflict3(10, 20, 30, 40, 50, 60, 70, 80, 90, 100);
    total += test_ira_conflict3(100, 200, 300, 400, 500, 600, 700, 800, 900, 1000);
    
    /* Return something to prevent dead code elimination */
    return total % 256;
}
