/* test_mcf_coverage.c
 * 
 * This test program creates a complex register allocation scenario
 * designed to trigger the min-cost flow solver's fixup graph construction
 * in GCC's IRA, specifically to exercise the dump_fixup_edge function
 * with n == fixup_graph->new_exit_index and n == fixup_graph->new_entry_index.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of MCF debugging */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with extreme register pressure to force complex conflict graph */
__attribute__((noinline))
int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int sum = 0;
    
    /* Initialize all variables with different values to prevent optimization */
    v1 = iterations * 1; v2 = iterations * 2; v3 = iterations * 3;
    v4 = iterations * 4; v5 = iterations * 5; v6 = iterations * 6;
    v7 = iterations * 7; v8 = iterations * 8; v9 = iterations * 9;
    v10 = iterations * 10; v11 = iterations * 11; v12 = iterations * 12;
    v13 = iterations * 13; v14 = iterations * 14; v15 = iterations * 15;
    v16 = iterations * 16; v17 = iterations * 17; v18 = iterations * 18;
    v19 = iterations * 19; v20 = iterations * 20; v21 = iterations * 21;
    v22 = iterations * 22; v23 = iterations * 23; v24 = iterations * 24;
    v25 = iterations * 25; v26 = iterations * 26; v27 = iterations * 27;
    v28 = iterations * 28; v29 = iterations * 29; v30 = iterations * 30;
    
    /* Nested loops to create complex liveness patterns */
    for (int i = 0; i < iterations; i++) {
        /* Make many variables live across loop iterations */
        v1 += i; v2 += v1; v3 += v2; v4 += v3; v5 += v4;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 5; j++) {
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile (
                "/* Clobber many registers */"
                :
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14"
            );
            
            /* Complex computation keeping many variables live */
            v6 = v1 + v2 + j;
            v7 = v3 + v4 + j;
            v8 = v5 + v6 + j;
            v9 = v7 + v8 + j;
            v10 = v9 + v1 + j;
            
            v11 = v6 * 2 - v7;
            v12 = v8 * 3 - v9;
            v13 = v10 * 4 - v11;
            v14 = v11 * 5 - v12;
            v15 = v12 * 6 - v13;
            
            /* More overlapping live ranges */
            v16 = v13 + v14 + v15;
            v17 = v16 - v1 - v2;
            v18 = v17 * v3 / (v4 + 1);
            v19 = v18 ^ v5 ^ v6;
            v20 = v19 | v7 | v8;
            
            v21 = v9 & v10 & v11;
            v22 = v12 ^ v13 ^ v14;
            v23 = v15 | v16 | v17;
            v24 = v18 + v19 + v20;
            v25 = v21 * v22 * v23;
            
            v26 = v24 - v25 + v1;
            v27 = v2 * v3 - v26;
            v28 = v4 + v5 + v27;
            v29 = v6 * v7 / (v28 + 1);
            v30 = v8 ^ v9 ^ v29;
            
            sum += v30;
        }
        
        /* Another asm statement that uses many input registers */
        asm volatile (
            "/* Use many input registers */"
            : "=r" (v1), "=r" (v2), "=r" (v3), "=r" (v4), "=r" (v5)
            : "0" (v1), "1" (v2), "2" (v3), "3" (v4), "4" (v5),
              "r" (v6), "r" (v7), "r" (v8), "r" (v9), "r" (v10)
            : "memory"
        );
    }
    
    /* Final computation using all variables to ensure they're live until here */
    sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    sum += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    return sum;
}

/* Second test function with different pattern to explore more graph configurations */
__attribute__((noinline))
int test_ira_conflict2(int seed) {
    /* Variables with partial overlapping live ranges */
    int a = seed, b = seed * 2, c = seed * 3, d = seed * 4, e = seed * 5;
    int f, g, h, i, j, k, l, m, n, o;
    
    /* Conditional blocks create different control flow paths */
    if (seed & 1) {
        f = a + b;
        g = c + d;
        
        /* Loop with variables that become dead at different points */
        for (int x = 0; x < 10; x++) {
            h = f + g + x;
            i = a * b - x;
            
            /* Force spill/reload pattern */
            asm volatile (
                "/* Force register pressure */"
                : 
                : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
                  "r" (f), "r" (g), "r" (h), "r" (i)
                : "memory"
            );
            
            j = h * i;
            k = j >> 2;
            
            /* Make some variables dead early */
            if (x > 5) {
                /* a, b, c no longer needed in this branch */
                l = k + e + d;
                m = l * f;
            } else {
                l = k + a + b + c;
                m = l * d;
            }
            
            n = m + g;
            o = n ^ h;
            
            a += o;  /* a reused with new value */
        }
    } else {
        f = b + c;
        g = d + e;
        
        /* Different computation pattern */
        for (int x = 0; x < 8; x++) {
            h = f - g + x;
            i = b * c + x;
            j = h | i;
            k = j & f;
            
            asm volatile (
                "/* Different clobber set */"
                :
                :
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
            );
            
            l = k * 7;
            m = l / (g + 1);
            n = m ^ h;
            o = n - i;
            
            b += o;  /* b reused */
        }
    }
    
    /* Final use creates web of overlapping live ranges */
    return a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
}

/* Third test: Function with parameters forcing specific register allocation */
__attribute__((noinline))
int test_ira_conflict3(int p1, int p2, int p3, int p4, int p5,
                       int p6, int p7, int p8, int p9, int p10) {
    /* Many parameters already in registers create initial pressure */
    int t1 = p1 * p2;
    int t2 = p3 * p4;
    int t3 = p5 * p6;
    int t4 = p7 * p8;
    int t5 = p9 * p10;
    
    /* Unrolled loop to increase basic block size */
    t1 += p2; t2 += p3; t3 += p4; t4 += p5; t5 += p6;
    t1 += p7; t2 += p8; t3 += p9; t4 += p10; t5 += p1;
    t1 += p3; t2 += p4; t3 += p5; t4 += p6; t5 += p7;
    t1 += p8; t2 += p9; t3 += p10; t4 += p1; t5 += p2;
    
    /* Volatile asm that clobbers caller-saved registers */
    asm volatile (
        "/* Massive clobber to force spills */"
        :
        : "r" (t1), "r" (t2), "r" (t3), "r" (t4), "r" (t5),
          "r" (p1), "r" (p2), "r" (p3), "r" (p4), "r" (p5),
          "r" (p6), "r" (p7), "r" (p8), "r" (p9), "r" (p10)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r14", "memory"
    );
    
    return t1 + t2 + t3 + t4 + t5 + p1 + p2 + p3 + p4 + p5 +
           p6 + p7 + p8 + p9 + p10;
}

/* Main function that calls test functions with different parameters
 * to explore various conflict graph configurations */
int main() {
    int result = 0;
    
    /* Call with different iteration counts to create graphs of different sizes */
    for (int i = 1; i <= 5; i++) {
        result ^= test_ira_conflict(i);  /* Vary graph size */
        result ^= test_ira_conflict2(i * 10);
        result ^= test_ira_conflict3(i, i+1, i+2, i+3, i+4,
                                    i+5, i+6, i+7, i+8, i+9);
    }
    
    /* Prevent optimization of entire program */
    asm volatile ("" : : "r" (result));
    
    return result;
}

/* Additional global variable to affect allocation */
volatile int global_counter = 0;

/* Helper function that's called from test functions to add more complexity */
__attribute__((noinline))
int helper_func(int x, int y) {
    int a = x, b = y;
    
    /* Small loop with register pressure */
    for (int i = 0; i < 3; i++) {
        a = (a << 3) | (b >> 5);
        b = (b << 5) | (a >> 3);
        
        /* Access volatile global to prevent optimizations */
        global_counter++;
    }
    
    /* Conditional to create control flow merge point */
    if (global_counter & 1) {
        return a - b;
    } else {
        return b - a;
    }
}
