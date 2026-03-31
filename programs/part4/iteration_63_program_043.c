/* test_mcf_coverage.c
 * 
 * This test is designed to trigger the uncovered lines in GCC's mcf.cc
 * Specifically, the dump_fixup_edge function's branches for:
 * - fixup_graph->new_exit_index
 * - fixup_graph->new_entry_index
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with volatile asm to prevent optimization */
#define CLOBBER_MANY_REGS() \
    __asm__ volatile ("" : : : \
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
        "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")

/* Function that creates complex register pressure with many overlapping live ranges */
static int __attribute__((noinline)) 
test_ira_conflict_complex(int iterations) 
{
    /* Declare many variables that will have overlapping live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    int sum = 0;
    
    /* Initialize all variables with different values to prevent CSE */
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
    
    /* Nested loops to create complex liveness patterns */
    for (int i = 0; i < iterations; i++) {
        /* Make many variables live across loop iterations */
        v0 += i;
        v1 += v0;
        v2 += v1;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 3; j++) {
            /* Use many variables in computation */
            v3 = v0 + v1 + j;
            v4 = v1 + v2 + j;
            v5 = v2 + v3 + j;
            v6 = v3 + v4 + j;
            v7 = v4 + v5 + j;
            v8 = v5 + v6 + j;
            v9 = v6 + v7 + j;
            v10 = v7 + v8 + j;
            
            /* Force register pressure with clobber */
            CLOBBER_MANY_REGS();
            
            /* More computations keeping variables live */
            v11 = v8 + v9 + v10;
            v12 = v9 + v10 + v11;
            v13 = v10 + v11 + v12;
            v14 = v11 + v12 + v13;
            v15 = v12 + v13 + v14;
            v16 = v13 + v14 + v15;
            v17 = v14 + v15 + v16;
            v18 = v15 + v16 + v17;
            v19 = v16 + v17 + v18;
            
            /* Accumulate to sum to prevent dead code elimination */
            sum += v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                   v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
        }
        
        /* More computations between outer loop iterations */
        v0 = v19 - v0;
        v1 = v0 - v1;
        v2 = v1 - v2;
        
        /* Another clobber to force spill/reload */
        CLOBBER_MANY_REGS();
    }
    
    /* Final computation using all variables */
    return sum + v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
           v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
}

/* Alternative function with different live range patterns */
static int __attribute__((noinline))
test_ira_conflict_alternate(int seed) 
{
    /* Variables with carefully crafted live/dead patterns */
    int a = seed * 1, b = seed * 2, c = seed * 3, d = seed * 4;
    int e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Create definition-use chains that cross basic blocks */
    if (seed > 100) {
        e = a + b;
        f = b + c;
        g = c + d;
        h = d + a;
    } else {
        e = a - b;
        f = b - c;
        g = c - d;
        h = d - a;
    }
    
    /* Loop with variables that become live at different points */
    for (int x = 0; x < 5; x++) {
        i = e + x;
        j = f + x;
        k = g + x;
        l = h + x;
        
        /* Conditional that creates divergent live ranges */
        if (x & 1) {
            m = i + j;
            n = j + k;
            o = k + l;
            p = l + i;
        } else {
            m = i - j;
            n = j - k;
            o = k - l;
            p = l - i;
        }
        
        /* Use all variables in a complex expression */
        q = m + n + o + p;
        r = q * 2 - seed;
        s = r / 3 + x;
        t = s ^ (seed & 0xFF);
        
        /* Clobber to force register shuffling */
        CLOBBER_MANY_REGS();
    }
    
    /* Return using many live variables at function exit */
    return a + b + c + d + e + f + g + h + i + j + 
           k + l + m + n + o + p + q + r + s + t;
}

/* Function designed to create imbalanced supply/demand for pseudo-registers */
static int __attribute__((noinline))
test_ira_imbalance(int base)
{
    /* Many uses but few definitions - creates demand > supply */
    int x1 = base * 1;
    int x2 = base * 2;
    int x3 = base * 3;
    int x4 = base * 4;
    int x5 = base * 5;
    
    int result = 0;
    
    /* Chain of uses without redefinition */
    for (int i = 0; i < 10; i++) {
        /* Each iteration uses x1-x5 many times but doesn't redefine them */
        result += x1 * i;
        result += x2 * (i + 1);
        result += x3 * (i + 2);
        result += x4 * (i + 3);
        result += x5 * (i + 4);
        
        /* Intermediate computations create temporary live ranges */
        int t1 = result + x1;
        int t2 = result + x2;
        int t3 = result + x3;
        int t4 = result + x4;
        int t5 = result + x5;
        
        result = t1 + t2 + t3 + t4 + t5;
        
        /* Force spills with clobber */
        CLOBBER_MANY_REGS();
    }
    
    /* Final redefinition */
    x1 = result;
    x2 = result * 2;
    x3 = result * 3;
    x4 = result * 4;
    x5 = result * 5;
    
    return x1 + x2 + x3 + x4 + x5;
}

/* Main function that exercises different patterns */
int main(int argc, char **argv)
{
    int result = 0;
    
    /* Test different iteration counts to create different graph sizes */
    int test_cases[] = {5, 10, 15, 20, 25};
    
    for (int tc = 0; tc < 5; tc++) {
        /* Each test creates different conflict graph characteristics */
        result += test_ira_conflict_complex(test_cases[tc]);
        result += test_ira_conflict_alternate(test_cases[tc] * 10);
        result += test_ira_imbalance(test_cases[tc] * 5);
    }
    
    printf("Result: %d\n", result);  /* Prevent optimization */
    return result > 0 ? 0 : 1;
}

/* Additional test with switch statement for more control flow complexity */
static int __attribute__((noinline))
test_ira_switch(int val)
{
    int a = val, b = val * 2, c = val * 3, d = val * 4;
    int e = 0;
    
    switch (val % 5) {
        case 0:
            e = a + b;
            CLOBBER_MANY_REGS();
            break;
        case 1:
            e = b + c;
            CLOBBER_MANY_REGS();
            break;
        case 2:
            e = c + d;
            CLOBBER_MANY_REGS();
            break;
        case 3:
            e = d + a;
            CLOBBER_MANY_REGS();
            break;
        case 4:
            e = a + b + c + d;
            CLOBBER_MANY_REGS();
            break;
    }
    
    /* Loop with switch inside creates complex live ranges */
    for (int i = 0; i < 8; i++) {
        switch (i % 3) {
            case 0: a += e; break;
            case 1: b += a; break;
            case 2: c += b; d += c; break;
        }
    }
    
    return a + b + c + d + e;
}
