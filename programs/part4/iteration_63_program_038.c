/* test_mcf_coverage.c
 * 
 * This program creates a complex register allocation scenario to trigger
 * the min-cost flow solver's fixup graph construction in GCC's IRA,
 * specifically aiming to exercise the dump_fixup_edge function with
 * new_exit_index and new_entry_index cases.
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    volatile int p = 16, q = 17, r = 18, s = 19, t = 20;
    
    /* Complex nested loops to create intricate live range overlaps */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across loop boundaries */
        a += outer;
        b *= (outer + 1);
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 5; inner++) {
            /* Force register pressure by using many variables */
            c = a + b + inner;
            d = c * outer - inner;
            e = d / (inner + 1) + a;
            
            /* More computations to extend live ranges */
            f = e + c + d;
            g = f * inner - outer;
            h = g + a + b + c;
            
            /* Use inline asm to clobber registers and increase pressure */
            asm volatile (
                "/* Clobber many registers */"
                :
                : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
                  "r"(f), "r"(g), "r"(h)
                : "memory", "cc"
            );
            
            /* More variables to increase graph complexity */
            i = h + f + g;
            j = i * outer + inner;
            k = j - a - b;
            
            /* Conditional to create control flow divergence */
            if (inner % 2 == 0) {
                l = k + i + j;
                m = l * 2 - outer;
            } else {
                l = k - i - j;
                m = l / 2 + outer;
            }
            
            /* Use all variables in a complex expression */
            n = m + l + k + j + i + h + g + f + e + d + c + b + a;
            
            /* More asm to force spills/reloads */
            asm volatile (
                "/* Force register shuffling */"
                : "+r"(n), "+r"(m), "+r"(l), "+r"(k)
                :
                : "memory", "cc"
            );
            
            /* Final computations */
            o = n * m - l + k;
            p = o + n + m;
            q = p / (inner + 2) + outer;
            r = q * p - o;
            s = r + q + p;
            t = s * outer + inner;
        }
        
        /* Use results to prevent optimization */
        asm volatile (
            "/* Use all results */"
            :
            : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e),
              "r"(f), "r"(g), "r"(h), "r"(i), "r"(j),
              "r"(k), "r"(l), "r"(m), "r"(n), "r"(o),
              "r"(p), "r"(q), "r"(r), "r"(s), "r"(t)
            : "memory"
        );
    }
    
    /* Prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j + 
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Another test function with different live range patterns */
__attribute__((noinline))
void test_ira_conflict2(int seed) {
    /* Variables with different lifetimes */
    int v1 = seed, v2 = seed * 2, v3 = seed * 3;
    int v4, v5, v6, v7, v8, v9, v10;
    
    /* Switch-like structure for varied control flow */
    for (int i = 0; i < 10; i++) {
        switch (i % 4) {
            case 0:
                v4 = v1 + v2;
                v5 = v3 * v4;
                /* Force long live range */
                v6 = v5 - v1;
                break;
            case 1:
                v7 = v2 + v3;
                v8 = v7 / (v1 + 1);
                v6 = v8 * 2;  /* v6 live across cases */
                break;
            case 2:
                v9 = v6 + v3 + v2;
                v10 = v9 * v1;
                v4 = v10 - v6;  /* v4 live across cases */
                break;
            case 3:
                /* Use all variables */
                v1 = v4 + v5 + v6 + v7 + v8 + v9 + v10;
                v2 = v1 * i;
                v3 = v2 - seed;
                break;
        }
        
        /* Inline asm that clobbers specific registers */
        asm volatile (
            "/* Selective clobbering */"
            : "+r"(v1), "+r"(v2), "+r"(v3)
            :
            : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "memory", "cc"
        );
    }
}

/* Function with artificial register pressure via many parameters */
__attribute__((noinline, regparm(0)))
int test_many_params(int p1, int p2, int p3, int p4, int p5,
                     int p6, int p7, int p8, int p9, int p10) {
    /* All parameters are live initially */
    int sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;
    
    /* Create many temporary variables */
    int t1 = sum * p1, t2 = sum * p2, t3 = sum * p3;
    int t4 = sum * p4, t5 = sum * p5, t6 = sum * p6;
    int t7 = sum * p7, t8 = sum * p8, t9 = sum * p9;
    int t10 = sum * p10;
    
    /* Complex loop with all variables live */
    for (int i = 0; i < 100; i++) {
        t1 += p1 + i;
        t2 += p2 + t1;
        t3 += p3 + t2;
        t4 += p4 + t3;
        t5 += p5 + t4;
        t6 += p6 + t5;
        t7 += p7 + t6;
        t8 += p8 + t7;
        t9 += p9 + t8;
        t10 += p10 + t9;
        
        /* Force register pressure with asm */
        asm volatile (
            "/* Massive clobber */"
            : "+r"(t1), "+r"(t2), "+r"(t3), "+r"(t4), "+r"(t5),
              "+r"(t6), "+r"(t7), "+r"(t8), "+r"(t9), "+r"(t10)
            :
            : "memory", "cc"
        );
    }
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Main function to trigger different IRA scenarios */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    
    /* Call test functions multiple times with different parameters
     * to explore different register allocation scenarios */
    for (int run = 0; run < 3; run++) {
        test_ira_conflict(iterations + run);
        test_ira_conflict2(iterations * (run + 1));
        
        /* Test with many parameters */
        int result = test_many_params(
            run * 1, run * 2, run * 3, run * 4, run * 5,
            run * 6, run * 7, run * 8, run * 9, run * 10
        );
        
        /* Use result to prevent optimization */
        asm volatile ("" : : "r"(result) : "memory");
    }
    
    return 0;
}
