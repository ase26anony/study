/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * Min-Cost Flow solver (mcf.cc) by creating register allocation
 * scenarios that require fixup graph construction with special nodes.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * 
 * For ARM targets (limited registers): gcc -O2 -march=armv7-a -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of IRA debugging */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with many overlapping live ranges to create complex conflict graph */
int test_ira_conflict_1(void) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize all variables with different values */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < 100; outer++) {
        /* Many variables live across inner loop */
        for (int inner = 0; inner < 50; inner++) {
            /* Complex computation keeping many variables live */
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + g;
            f = g + h;
            g = h + i;
            h = i + j;
            i = j + k;
            j = k + l;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            asm volatile ("" : : "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
            
            /* More computations */
            k = l + m;
            l = m + n;
            m = n + o;
            n = o + p;
            o = p + q;
            
            asm volatile ("" : : "r"(k), "r"(l), "r"(m), "r"(n), "r"(o));
            
            p = q + r;
            q = r + s;
            r = s + t;
            s = t + a;
            t = a + b;
            
            asm volatile ("" : : "r"(p), "r"(q), "r"(r), "r"(s), "r"(t));
            
            /* Accumulate result */
            result += a + b + c + d + e + f + g + h + i + j +
                     k + l + m + n + o + p + q + r + s + t;
        }
        
        /* Force spilling/reloading between outer loop iterations */
        asm volatile ("# Force register pressure" : : : 
                     "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    }
    
    return result;
}

/* Function with unbalanced definitions/uses to create supply-demand imbalance */
int test_ira_conflict_2(int param1, int param2, int param3, int param4, int param5,
                       int param6, int param7, int param8, int param9, int param10) {
    /* Create many temporary variables from parameters */
    int t1 = param1 * 2;
    int t2 = param2 * 3;
    int t3 = param3 * 4;
    int t4 = param4 * 5;
    int t5 = param5 * 6;
    int t6 = param6 * 7;
    int t7 = param7 * 8;
    int t8 = param8 * 9;
    int t9 = param9 * 10;
    int t10 = param10 * 11;
    
    /* Chain computations to create data dependencies */
    for (int i = 0; i < 1000; i++) {
        /* Many uses of each variable before redefinition */
        t1 = t1 + t2 + t3;
        t2 = t2 + t3 + t4;
        t3 = t3 + t4 + t5;
        t4 = t4 + t5 + t6;
        t5 = t5 + t6 + t7;
        t6 = t6 + t7 + t8;
        t7 = t7 + t8 + t9;
        t8 = t8 + t9 + t10;
        t9 = t9 + t10 + t1;
        t10 = t10 + t1 + t2;
        
        /* Force all variables to be live simultaneously */
        asm volatile ("" : : 
                     "r"(t1), "r"(t2), "r"(t3), "r"(t4), "r"(t5),
                     "r"(t6), "r"(t7), "r"(t8), "r"(t9), "r"(t10));
    }
    
    /* Return uses all temporaries */
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Function with switch statement to create complex control flow */
int test_ira_conflict_3(int selector) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int y1 = 6, y2 = 7, y3 = 8, y4 = 9, y5 = 10;
    int z1 = 11, z2 = 12, z3 = 13, z4 = 14, z5 = 15;
    
    /* Complex control flow with many live variables */
    switch (selector) {
        case 0:
            x1 = y1 + z1;
            /* fall through */
        case 1:
            x2 = y2 + z2;
            x3 = y3 + z3;
            break;
        case 2:
            x4 = y4 + z4;
            x5 = y5 + z5;
            /* fall through */
        case 3:
            y1 = x1 + z1;
            y2 = x2 + z2;
            y3 = x3 + z3;
            y4 = x4 + z4;
            y5 = x5 + z5;
            break;
        case 4:
            z1 = x1 + y1;
            z2 = x2 + y2;
            z3 = x3 + y3;
            z4 = x4 + y4;
            z5 = x5 + y5;
            break;
        default:
            x1 = y1 = z1 = selector;
            x2 = y2 = z2 = selector * 2;
            x3 = y3 = z3 = selector * 3;
            x4 = y4 = z4 = selector * 4;
            x5 = y5 = z5 = selector * 5;
    }
    
    /* All variables live at return */
    asm volatile ("" : : 
                 "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5),
                 "r"(y1), "r"(y2), "r"(y3), "r"(y4), "r"(y5),
                 "r"(z1), "r"(z2), "r"(z3), "r"(z4), "r"(z5));
    
    return x1 + x2 + x3 + x4 + x5 + y1 + y2 + y3 + y4 + y5 + z1 + z2 + z3 + z4 + z5;
}

/* Function with recursive calls to increase register pressure */
int test_ira_conflict_4(int depth, int acc) {
    int local1 = acc + 1;
    int local2 = acc + 2;
    int local3 = acc + 3;
    int local4 = acc + 4;
    int local5 = acc + 5;
    
    if (depth <= 0) {
        return local1 + local2 + local3 + local4 + local5;
    }
    
    /* Recursive call with all locals live across call */
    int result = test_ira_conflict_4(depth - 1, acc + local1);
    
    /* More computations after recursion */
    local1 = result + local2;
    local2 = result + local3;
    local3 = result + local4;
    local4 = result + local5;
    local5 = result + local1;
    
    asm volatile ("" : : 
                 "r"(local1), "r"(local2), "r"(local3), 
                 "r"(local4), "r"(local5), "r"(result));
    
    return local1 + local2 + local3 + local4 + local5 + result;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    
    /* Call each test function multiple times with different parameters
     * to explore different register allocation scenarios */
    for (int i = 0; i < 10; i++) {
        total += test_ira_conflict_1();
        total += test_ira_conflict_2(i, i+1, i+2, i+3, i+4, i+5, i+6, i+7, i+8, i+9);
        total += test_ira_conflict_3(i % 5);
        total += test_ira_conflict_4(3, i);
    }
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r"(total));
    
    return total % 256;  /* Return small value to avoid overflow */
}
