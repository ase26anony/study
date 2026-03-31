/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger the uncovered lines in GCC's
 * mcf.cc dump_fixup_edge function, specifically the branches that print
 * "NEW_EXIT" and "NEW_ENTRY" labels.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
static void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize with different values to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops with many live variables across loop boundaries */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live at this point */
        a += outer;
        b += a;
        
        /* Inner loop with complex live range overlaps */
        for (int inner = 0; inner < 10; inner++) {
            /* Many variables used in computation */
            c = a + b + inner;
            d = b + c + outer;
            e = c + d + inner;
            f = d + e + outer;
            g = e + f + inner;
            h = f + g + outer;
            
            /* Force register pressure with inline asm that clobbers registers */
            asm volatile (
                "# Force register clobbering\n"
                "movl $0, %%eax\n"
                "movl $0, %%ebx\n"
                "movl $0, %%ecx\n"
                "movl $0, %%edx\n"
                "movl $0, %%esi\n"
                "movl $0, %%edi\n"
                :
                :
                : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
            );
            
            /* More computations to extend live ranges */
            i = g + h + inner;
            j = h + i + outer;
            k = i + j + inner;
            l = j + k + outer;
        }
        
        /* Another computation block with different variable combinations */
        m = k + l + outer;
        n = l + m + a;
        o = m + n + b;
        p = n + o + c;
        q = o + p + d;
        r = p + q + e;
        s = q + r + f;
        t = r + s + g;
        
        /* Another asm block to increase pressure */
        asm volatile (
            "# More register pressure\n"
            "movl $0, %%r8d\n"
            "movl $0, %%r9d\n"
            "movl $0, %%r10d\n"
            "movl $0, %%r11d\n"
            :
            :
            : "r8", "r9", "r10", "r11", "memory"
        );
    }
    
    /* Use all variables at the end to ensure they're not optimized away */
    volatile int result = a + b + c + d + e + f + g + h + i + j + 
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Test function with switch statement to create complex CFG */
__attribute__((noinline))
static void test_complex_cfg(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    int x11 = 11, x12 = 12, x13 = 13, x14 = 14, x15 = 15;
    
    /* Complex switch creates many basic blocks */
    switch (mode % 8) {
        case 0:
            x1 = x2 + x3;
            x4 = x5 + x6;
            break;
        case 1:
            x2 = x3 + x4;
            x5 = x6 + x7;
            break;
        case 2:
            x3 = x4 + x5;
            x6 = x7 + x8;
            break;
        case 3:
            x4 = x5 + x6;
            x7 = x8 + x9;
            break;
        case 4:
            x5 = x6 + x7;
            x8 = x9 + x10;
            break;
        case 5:
            x6 = x7 + x8;
            x9 = x10 + x11;
            break;
        case 6:
            x7 = x8 + x9;
            x10 = x11 + x12;
            break;
        case 7:
            x8 = x9 + x10;
            x11 = x12 + x13;
            break;
    }
    
    /* All variables used in final computation */
    volatile int sum = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10 + 
                      x11 + x12 + x13 + x14 + x15;
    (void)sum;
}

/* Function with parameters to force register allocation of arguments */
__attribute__((noinline))
static int test_many_params(int p1, int p2, int p3, int p4, int p5,
                           int p6, int p7, int p8, int p9, int p10) {
    /* Create many temporary variables that overlap with parameters */
    int t1 = p1 + p2;
    int t2 = p3 + p4;
    int t3 = p5 + p6;
    int t4 = p7 + p8;
    int t5 = p9 + p10;
    
    int t6 = t1 + t2;
    int t7 = t3 + t4;
    int t8 = t5 + t1;
    int t9 = t2 + t3;
    int t10 = t4 + t5;
    
    /* Loop to extend live ranges */
    for (int i = 0; i < 5; i++) {
        t1 += i;
        t2 += t1;
        t3 += t2;
        t4 += t3;
        t5 += t4;
        
        /* Inline asm to clobber caller-saved registers */
        asm volatile (
            "# Clobber caller-saved regs\n"
            "movl $0, %%eax\n"
            "movl $0, %%ecx\n"
            "movl $0, %%edx\n"
            :
            :
            : "eax", "ecx", "edx", "memory"
        );
    }
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Main function that exercises different scenarios */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Test 1: Complex conflict graph with many overlapping live ranges */
    test_ira_conflict(iterations);
    
    /* Test 2: Complex control flow graph */
    for (int i = 0; i < iterations / 10; i++) {
        test_complex_cfg(i);
    }
    
    /* Test 3: Many parameters forcing specific register allocation */
    int result = 0;
    for (int i = 0; i < iterations / 20; i++) {
        result += test_many_params(i, i+1, i+2, i+3, i+4,
                                  i+5, i+6, i+7, i+8, i+9);
    }
    
    printf("Result: %d\n", result);
    return 0;
}
