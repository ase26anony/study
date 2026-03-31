/* test_mcf_coverage.c
 * 
 * This test creates a function with extreme register pressure and complex
 * liveness patterns to trigger the min-cost flow solver's fixup graph
 * construction in GCC's IRA. When compiled with -DMCF_DEBUG, this should
 * cause dump_fixup_edge to be called with n == fixup_graph->new_exit_index
 * and n == fixup_graph->new_entry_index.
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize all variables with different values to prevent optimization */
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
    
    /* Nested loops with many live variables across iterations */
    for (int i = 0; i < iterations; i++) {
        /* All variables are live here due to volatile accesses */
        asm volatile ("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4));
        asm volatile ("" : : "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9));
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 3; j++) {
            /* More volatile asm to keep variables live and clobber registers */
            asm volatile ("# Clobber many registers" 
                         : 
                         : "r"(v10), "r"(v11), "r"(v12), "r"(v13), "r"(v14),
                           "r"(v15), "r"(v16), "r"(v17), "r"(v18), "r"(v19)
                         : "memory", "cc");
            
            /* Complex computation keeping many variables live */
            v20 = v0 + v1 + v2;
            v21 = v3 + v4 + v5;
            v22 = v6 + v7 + v8;
            v23 = v9 + v10 + v11;
            v24 = v12 + v13 + v14;
            
            /* More asm to prevent optimization and increase pressure */
            asm volatile ("# More clobbering" 
                         : 
                         : "r"(v20), "r"(v21), "r"(v22), "r"(v23), "r"(v24),
                           "r"(v25), "r"(v26), "r"(v27), "r"(v28), "r"(v29)
                         : "memory");
        }
        
        /* Switch-like structure to create complex control flow */
        switch (i % 4) {
            case 0:
                v25 = v0 + v10 + v20;
                asm volatile ("" : : "r"(v25));
                break;
            case 1:
                v26 = v1 + v11 + v21;
                asm volatile ("" : : "r"(v26));
                break;
            case 2:
                v27 = v2 + v12 + v22;
                asm volatile ("" : : "r"(v27));
                break;
            case 3:
                v28 = v3 + v13 + v23;
                asm volatile ("" : : "r"(v28));
                break;
        }
        
        /* Final computation using all variables */
        v29 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
              v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
              v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28;
        
        /* Force v29 to be used */
        asm volatile ("" : : "r"(v29) : "memory");
    }
    
    /* Use all variables one more time to extend liveness */
    asm volatile ("# Final use" 
                 : 
                 : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                   "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
                   "r"(v10), "r"(v11), "r"(v12), "r"(v13), "r"(v14),
                   "r"(v15), "r"(v16), "r"(v17), "r"(v18), "r"(v19),
                   "r"(v20), "r"(v21), "r"(v22), "r"(v23), "r"(v24),
                   "r"(v25), "r"(v26), "r"(v27), "r"(v28), "r"(v29)
                 : "memory");
}

/* Second test function with different pattern to explore more graph shapes */
__attribute__((noinline))
void test_ira_conflict2(int seed) {
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Chain of dependencies creating long live ranges */
    a = seed;
    b = a * 2;
    c = b + a;
    d = c * 3;
    e = d - b;
    f = e + c;
    g = f * a;
    h = g - d;
    i = h + e;
    j = i * b;
    k = j - f;
    l = k + g;
    m = l * c;
    n = m - h;
    o = n + i;
    p = o * d;
    q = p - j;
    r = q + k;
    s = r * e;
    t = s - l;
    
    /* Unrolled loop with all variables live */
    for (int x = 0; x < 8; x++) {
        asm volatile ("# Chain computation %0" : : "r"(a + x));
        asm volatile ("# Chain computation %0" : : "r"(b + x));
        asm volatile ("# Chain computation %0" : : "r"(c + x));
        asm volatile ("# Chain computation %0" : : "r"(d + x));
        
        /* Conditional that keeps many variables live */
        if (x & 1) {
            a = b + c;
            e = f + g;
            i = j + k;
            m = n + o;
            q = r + s;
        } else {
            b = a + d;
            f = e + h;
            j = i + l;
            n = m + p;
            r = q + t;
        }
        
        /* More asm to prevent optimization */
        asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                         "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
                         "r"(k), "r"(l), "r"(m), "r"(n), "r"(o),
                         "r"(p), "r"(q), "r"(r), "r"(s), "r"(t));
    }
}

/* Third test: Function with parameters forcing caller-save register pressure */
__attribute__((noinline))
int test_ira_conflict3(int p1, int p2, int p3, int p4, int p5,
                       int p6, int p7, int p8, int p9, int p10) {
    volatile int l1 = p1 * p2;
    volatile int l2 = p3 * p4;
    volatile int l3 = p5 * p6;
    volatile int l4 = p7 * p8;
    volatile int l5 = p9 * p10;
    
    volatile int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* All parameters and locals must be live here */
        asm volatile ("# Mix params %0 %1 %2" : : "r"(p1 + i), "r"(p2 + i), "r"(l1));
        asm volatile ("# Mix params %0 %1 %2" : : "r"(p3 + i), "r"(p4 + i), "r"(l2));
        asm volatile ("# Mix params %0 %1 %2" : : "r"(p5 + i), "r"(p6 + i), "r"(l3));
        asm volatile ("# Mix params %0 %1 %2" : : "r"(p7 + i), "r"(p8 + i), "r"(l4));
        asm volatile ("# Mix params %0 %1 %2" : : "r"(p9 + i), "r"(p10 + i), "r"(l5));
        
        sum += p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10 +
               l1 + l2 + l3 + l4 + l5;
    }
    
    return sum;
}

int main() {
    printf("Starting IRA/MCF coverage test...\n");
    
    /* Call test functions with different parameters to explore
     * different conflict graph shapes and sizes */
    for (int i = 0; i < 5; i++) {
        test_ira_conflict(i + 2);
        test_ira_conflict2(i * 10);
        
        /* Pass many parameters to test_ira_conflict3 */
        int result = test_ira_conflict3(
            i * 1, i * 2, i * 3, i * 4, i * 5,
            i * 6, i * 7, i * 8, i * 9, i * 10
        );
        
        printf("Iteration %d, result = %d\n", i, result);
    }
    
    printf("Test completed.\n");
    return 0;
}
