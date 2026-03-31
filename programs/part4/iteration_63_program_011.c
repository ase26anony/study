/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * min-cost flow solver (mcf.cc) by creating register allocation
 * scenarios that force IRA to build complex fixup graphs.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force MCF_DEBUG to be defined if not already defined by command line */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with many overlapping live ranges to create complex conflict graph */
int test_ira_conflict_1(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize all variables with different values */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex control flow and overlapping live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the loop */
        a += outer;
        b += a;
        c += b;
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 10; inner++) {
            /* Use many variables in computation to keep them live */
            d = a + b + inner;
            e = c + d + inner;
            f = d + e + inner;
            g = e + f + inner;
            h = f + g + inner;
            
            /* More computations to increase register pressure */
            i = g + h + inner;
            j = h + i + inner;
            k = i + j + inner;
            l = j + k + inner;
            m = k + l + inner;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile ("# Force register clobbering %0 %1 %2 %3 %4"
                         : /* no outputs */
                         : "r" (d), "r" (e), "r" (f), "r" (g), "r" (h)
                         : "memory");
            
            /* More computations after asm */
            n = l + m + inner;
            o = m + n + inner;
            p = n + o + inner;
            q = o + p + inner;
            r = p + q + inner;
            s = q + r + inner;
            t = r + s + inner;
            
            result += t;
        }
        
        /* Use all variables again to keep them live across loop iterations */
        result += a + b + c + d + e + f + g + h + i + j;
        result += k + l + m + n + o + p + q + r + s + t;
        
        /* Another asm statement that clobbers many registers */
        asm volatile ("# More clobbering %0 %1 %2 %3 %4 %5 %6 %7"
                     : /* no outputs */
                     : "r" (a), "r" (b), "r" (c), "r" (d),
                       "r" (e), "r" (f), "r" (g), "r" (h)
                     : "memory");
    }
    
    return result;
}

/* Function with different variable usage pattern to create different graph */
int test_ira_conflict_2(int seed) {
    /* Variables with different scopes and lifetimes */
    int v1 = seed, v2 = seed * 2, v3 = seed * 3;
    int v4, v5, v6, v7, v8, v9, v10;
    int sum = 0;
    
    /* Conditional blocks create different control flow paths */
    if (seed > 100) {
        v4 = v1 + v2;
        v5 = v2 + v3;
        v6 = v3 + v4;
        
        /* Loop with variables that become live at different times */
        for (int i = 0; i < 5; i++) {
            v7 = v4 + i;
            v8 = v5 + i;
            v9 = v6 + i;
            v10 = v7 + v8 + v9;
            
            /* Use asm to force specific register allocation patterns */
            asm volatile ("# Pattern A %0 %1 %2"
                         : /* no outputs */
                         : "r" (v7), "r" (v8), "r" (v9)
                         : "memory");
            
            sum += v10;
        }
    } else {
        v4 = v1 - v2;
        v5 = v2 - v3;
        v6 = v3 - v4;
        
        /* Different loop structure */
        for (int i = 0; i < 8; i++) {
            v7 = v4 * i;
            v8 = v5 * i;
            v9 = v6 * i;
            
            /* Different asm clobber pattern */
            asm volatile ("# Pattern B %0 %1 %2 %3 %4"
                         : /* no outputs */
                         : "r" (v4), "r" (v5), "r" (v6), "r" (v7), "r" (v8)
                         : "memory");
            
            sum += v7 + v8 + v9;
        }
    }
    
    /* Final computation using all variables */
    return sum + v1 + v2 + v3 + v4 + v5 + v6;
}

/* Function designed to create supply/demand imbalance for fixup edges */
int test_ira_fixup_edges(int base) {
    /* Create many definitions and uses of variables */
    int x1 = base, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    int y1, y2, y3, y4, y5, y6, y7, y8, y9, y10;
    
    /* Chain of computations creating data dependencies */
    x2 = x1 * 2;
    x3 = x2 + x1;
    x4 = x3 - x2;
    x5 = x4 * x3;
    x6 = x5 / (x4 + 1);
    x7 = x6 % (x5 + 1);
    x8 = x7 << 2;
    x9 = x8 >> 1;
    x10 = x9 ^ x8;
    
    /* Parallel computations to increase register pressure */
    y1 = base + 1;
    y2 = y1 * 3;
    y3 = y2 + y1;
    y4 = y3 - y2;
    y5 = y4 * y3;
    y6 = y5 / (y4 + 1);
    y7 = y6 % (y5 + 1);
    y8 = y7 << 1;
    y9 = y8 >> 2;
    y10 = y9 ^ y8;
    
    /* Complex expression using all variables multiple times */
    int result = 0;
    for (int i = 0; i < 3; i++) {
        /* Use all x variables */
        result += x1 + x2 + x3 + x4 + x5;
        result += x6 + x7 + x8 + x9 + x10;
        
        /* Use all y variables */
        result += y1 + y2 + y3 + y4 + y5;
        result += y6 + y7 + y8 + y9 + y10;
        
        /* Modify some variables to create new live ranges */
        x1 += i; x2 -= i; x3 *= (i + 1);
        y1 += i; y2 -= i; y3 *= (i + 1);
        
        /* Volatile asm that clobbers many registers */
        asm volatile ("# Fixup test %0 %1 %2 %3 %4 %5 %6 %7 %8 %9"
                     : /* no outputs */
                     : "r" (x1), "r" (x2), "r" (x3), "r" (x4), "r" (x5),
                       "r" (y1), "r" (y2), "r" (y3), "r" (y4), "r" (y5)
                     : "memory");
    }
    
    return result;
}

/* Function with switch statement to create complex control flow */
int test_ira_control_flow(int mode) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result = 0;
    
    switch (mode % 4) {
        case 0:
            a = b + c;
            d = e + f;
            g = h + i;
            result = a + d + g;
            /* Keep many variables live */
            asm volatile ("# Case 0 %0 %1 %2 %3"
                         : /* no outputs */
                         : "r" (a), "r" (b), "r" (c), "r" (d)
                         : "memory");
            break;
        case 1:
            b = c + d;
            e = f + g;
            h = i + j;
            result = b + e + h;
            asm volatile ("# Case 1 %0 %1 %2 %3 %4"
                         : /* no outputs */
                         : "r" (b), "r" (c), "r" (d), "r" (e), "r" (f)
                         : "memory");
            break;
        case 2:
            c = d + e;
            f = g + h;
            i = j + a;
            result = c + f + i;
            asm volatile ("# Case 2 %0 %1 %2 %3 %4 %5"
                         : /* no outputs */
                         : "r" (c), "r" (d), "r" (e), "r" (f), "r" (g), "r" (h)
                         : "memory");
            break;
        case 3:
            /* Use all variables */
            result = a + b + c + d + e + f + g + h + i + j;
            asm volatile ("# Case 3 %0 %1 %2 %3 %4 %5 %6 %7 %8 %9"
                         : /* no outputs */
                         : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e),
                           "r" (f), "r" (g), "r" (h), "r" (i), "r" (j)
                         : "memory");
            break;
    }
    
    return result;
}

/* Main function that exercises all test cases with different parameters */
int main() {
    int total = 0;
    
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    total += test_ira_conflict_1(5);
    total += test_ira_conflict_1(3);  /* Different iteration count */
    
    total += test_ira_conflict_2(50);
    total += test_ira_conflict_2(150); /* Different branch taken */
    
    total += test_ira_fixup_edges(10);
    total += test_ira_fixup_edges(20); /* Different base value */
    
    total += test_ira_control_flow(0);
    total += test_ira_control_flow(1);
    total += test_ira_control_flow(2);
    total += test_ira_control_flow(3); /* All switch cases */
    
    /* Return something based on computations to prevent optimization */
    return total % 256;
}
