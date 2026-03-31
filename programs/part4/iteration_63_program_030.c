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

/* Function with high register pressure and complex live ranges */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
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
    
    /* Nested loops to create complex live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the inner loop */
        int temp1 = a + b + c + d;
        int temp2 = e + f + g + h;
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 10; inner++) {
            /* Force all variables to be used and modified */
            a = b + inner;
            b = c + inner;
            c = d + inner;
            d = e + inner;
            e = f + inner;
            f = g + inner;
            g = h + inner;
            h = i + inner;
            i = j + inner;
            j = k + inner;
            k = l + inner;
            l = m + inner;
            m = n + inner;
            n = o + inner;
            o = p + inner;
            p = q + inner;
            q = r + inner;
            r = s + inner;
            s = t + inner;
            t = u + inner;
            u = v + inner;
            v = w + inner;
            w = x + inner;
            x = y + inner;
            y = z + inner;
            z = temp1 + temp2 + inner;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile (
                "/* Clobber many registers */"
                :
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14"
            );
            
            /* Accumulate result to prevent dead code elimination */
            result += a + b + c + d + e + f + g + h + i + j +
                     k + l + m + n + o + p + q + r + s + t +
                     u + v + w + x + y + z;
        }
        
        /* More computations to extend live ranges */
        int temp3 = a * b * c * d;
        int temp4 = e * f * g * h;
        
        /* Another asm statement that uses many input registers */
        asm volatile (
            "add %0, %0, %1\n\t"
            "add %0, %0, %2\n\t"
            "add %0, %0, %3"
            : "+r" (result)
            : "r" (temp3), "r" (temp4), "r" (outer)
            : "cc"
        );
    }
    
    /* Final computation using all variables */
    result = result + a - b + c - d + e - f + g - h + i - j +
             k - l + m - n + o - p + q - r + s - t +
             u - v + w - x + y - z;
    
    return result;
}

/* Second test function with different variable count to potentially
 * create different fixup graph sizes */
__attribute__((noinline))
static int test_ira_conflict2(int seed) {
    /* Use exactly 10 variables to potentially create a specific
     * graph size that might lead to new_exit_index being printed */
    int v1 = seed * 1;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    
    int sum = 0;
    
    /* Complex control flow to create imbalance in def-use chains */
    for (int i = 0; i < 100; i++) {
        if (i & 1) {
            v1 = v2 + v3;
            v3 = v4 + v5;
            v5 = v6 + v7;
            v7 = v8 + v9;
            v9 = v10 + i;
            
            /* More uses than definitions in this path */
            sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        } else {
            v2 = v1 + i;
            v4 = v3 + i;
            v6 = v5 + i;
            v8 = v7 + i;
            v10 = v9 + i;
            
            /* Different pattern of uses */
            sum += v1 * v2 + v3 * v4 + v5 * v6 + v7 * v8 + v9 * v10;
        }
        
        /* Switch statement to create more complex CFG */
        switch (i % 5) {
            case 0: v1 = v10; break;
            case 1: v2 = v9; break;
            case 2: v3 = v8; break;
            case 3: v4 = v7; break;
            case 4: v5 = v6; break;
        }
    }
    
    return sum;
}

/* Third test: Function with parameter passing to create
 * additional register pressure at entry/exit */
__attribute__((noinline))
static int test_ira_conflict3(int p1, int p2, int p3, int p4, int p5,
                              int p6, int p7, int p8, int p9, int p10) {
    /* All parameters are live initially */
    int sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;
    
    /* Create many local variables that overlap with parameters */
    int l1 = sum * 2;
    int l2 = sum * 3;
    int l3 = sum * 4;
    int l4 = sum * 5;
    int l5 = sum * 6;
    int l6 = sum * 7;
    int l7 = sum * 8;
    int l8 = sum * 9;
    int l9 = sum * 10;
    int l10 = sum * 11;
    
    /* Loop where all variables are live */
    for (int i = 0; i < 50; i++) {
        /* Rotate values to create data dependencies */
        int temp = p1;
        p1 = p2; p2 = p3; p3 = p4; p4 = p5; p5 = p6;
        p6 = p7; p7 = p8; p8 = p9; p9 = p10; p10 = temp;
        
        temp = l1;
        l1 = l2; l2 = l3; l3 = l4; l4 = l5; l5 = l6;
        l6 = l7; l7 = l8; l8 = l9; l9 = l10; l10 = temp;
        
        /* Use all variables in computation */
        sum += p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10 +
               l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10;
    }
    
    /* Return expression using all live variables */
    return sum + p1 - p2 + p3 - p4 + p5 - p6 + p7 - p8 + p9 - p10 +
           l1 - l2 + l3 - l4 + l5 - l6 + l7 - l8 + l9 - l10;
}

int main(void) {
    int total = 0;
    
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    for (int run = 0; run < 10; run++) {
        /* First test: Many variables, nested loops, asm clobbers */
        total += test_ira_conflict(run + 1);
        
        /* Second test: Specific number of variables (10) */
        total += test_ira_conflict2(run * 7);
        
        /* Third test: Many parameters + locals */
        total += test_ira_conflict3(run, run*2, run*3, run*4, run*5,
                                   run*6, run*7, run*8, run*9, run*10);
        
        /* Prevent optimization of loop */
        asm volatile ("" : : "r" (total));
    }
    
    printf("Result: %d\n", total);
    return 0;
}
