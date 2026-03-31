/* test_mcf_coverage.c
 * 
 * This test is designed to trigger the uncovered lines in mcf.cc's dump_fixup_edge
 * function by creating register allocation scenarios that require the min-cost
 * flow solver to create fixup graphs with NEW_EXIT and NEW_ENTRY nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or for more aggressive optimization: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of MCF debugging code */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

/* Function with many overlapping live ranges to create complex conflict graph */
__attribute__((noinline))
static int test_complex_live_ranges(int iterations) {
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
            /* Force many variables to be live simultaneously */
            d = a + b + inner;
            e = b + c + inner;
            f = c + d + inner;
            g = d + e + inner;
            h = e + f + inner;
            i = f + g + inner;
            j = g + h + inner;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile ("# Force register clobbering" : : : 
                "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            
            /* More computations keeping variables live */
            k = h + i + inner;
            l = i + j + inner;
            m = j + k + inner;
            n = k + l + inner;
            o = l + m + inner;
            p = m + n + inner;
            q = n + o + inner;
            r = o + p + inner;
            s = p + q + inner;
            t = q + r + inner;
            
            /* Accumulate result using all variables */
            result += a + b + c + d + e + f + g + h + i + j +
                     k + l + m + n + o + p + q + r + s + t;
        }
        
        /* More computations between loops */
        a = b + c;
        b = c + d;
        c = d + e;
        
        /* Another asm statement with different clobbers */
        asm volatile ("# More register pressure" : : : 
            "cc", "r0", "r1", "r2", "r3", "r4", "r5");
    }
    
    /* Final computation using all variables */
    result += a * b + c * d + e * f + g * h + i * j +
             k * l + m * n + o * p + q * r + s * t;
    
    return result;
}

/* Function with unbalanced definitions/uses to create supply/demand imbalance */
__attribute__((noinline))
static int test_unbalanced_flow(int seed) {
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    int y1, y2, y3, y4, y5, y6, y7, y8, y9, y10;
    int total = 0;
    
    /* Create many definitions in one basic block */
    x1 = seed + 1; x2 = seed + 2; x3 = seed + 3; x4 = seed + 4; x5 = seed + 5;
    x6 = seed + 6; x7 = seed + 7; x8 = seed + 8; x9 = seed + 9; x10 = seed + 10;
    
    /* Many uses in different paths - creates complex flow network */
    if (seed % 2) {
        y1 = x1 * 2; y2 = x2 * 3; y3 = x3 * 4; y4 = x4 * 5; y5 = x5 * 6;
        total += y1 + y2 + y3 + y4 + y5;
        
        /* More computations */
        y6 = x6 * 7; y7 = x7 * 8; y8 = x8 * 9; y9 = x9 * 10; y10 = x10 * 11;
        total += y6 + y7 + y8 + y9 + y10;
        
        /* Clobber registers */
        asm volatile ("# Path A clobber" : : : "memory", "r0", "r1", "r2", "r3");
    } else {
        y1 = x1 + 100; y2 = x2 + 200; y3 = x3 + 300; y4 = x4 + 400; y5 = x5 + 500;
        total += y1 + y2 + y3 + y4 + y5;
        
        /* Different computation pattern */
        y6 = x6 - 10; y7 = x7 - 20; y8 = x8 - 30; y9 = x9 - 40; y10 = x10 - 50;
        total += y6 + y7 + y8 + y9 + y10;
        
        /* Different clobber set */
        asm volatile ("# Path B clobber" : : : "memory", "r4", "r5", "r6", "r7");
    }
    
    /* All variables used again - creates more overlapping live ranges */
    total += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    total += y1 + y2 + y3 + y4 + y5 + y6 + y7 + y8 + y9 + y10;
    
    return total;
}

/* Function with switch statement to create complex control flow graph */
__attribute__((noinline))
static int test_switch_flow(int value) {
    int r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0;
    int r6 = 0, r7 = 0, r8 = 0, r9 = 0, r10 = 0;
    
    switch (value % 5) {
        case 0:
            r1 = value * 2;
            r2 = value * 3;
            asm volatile ("# Case 0" : : : "r0", "r1", "r2");
            break;
        case 1:
            r3 = value + 100;
            r4 = value + 200;
            r5 = value + 300;
            asm volatile ("# Case 1" : : : "r3", "r4", "r5");
            break;
        case 2:
            r6 = value - 50;
            r7 = value - 60;
            r8 = value - 70;
            r9 = value - 80;
            asm volatile ("# Case 2" : : : "r6", "r7", "r8", "r9");
            break;
        case 3:
            r1 = value / 2;
            r3 = value / 3;
            r5 = value / 4;
            r7 = value / 5;
            r9 = value / 6;
            asm volatile ("# Case 3" : : : "r1", "r3", "r5", "r7", "r9");
            break;
        case 4:
            r2 = value << 1;
            r4 = value << 2;
            r6 = value << 3;
            r8 = value << 4;
            r10 = value << 5;
            asm volatile ("# Case 4" : : : "r2", "r4", "r6", "r8", "r10");
            break;
    }
    
    /* Use all registers in computation */
    int result = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    
    /* Force another clobber */
    asm volatile ("# Final switch clobber" : : : 
        "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    
    return result;
}

/* Main test driver that exercises different scenarios */
int main(int argc, char **argv) {
    int total = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Test 1: Complex live ranges with nested loops */
    printf("Test 1: Complex live ranges...\n");
    for (int i = 0; i < 3; i++) {
        total += test_complex_live_ranges(5);
    }
    
    /* Test 2: Unbalanced flow patterns */
    printf("Test 2: Unbalanced flow...\n");
    for (int i = 0; i < 10; i++) {
        total += test_unbalanced_flow(i);
    }
    
    /* Test 3: Switch-based control flow */
    printf("Test 3: Switch flow...\n");
    for (int i = 0; i < 20; i++) {
        total += test_switch_flow(i);
    }
    
    /* Test 4: Mix of all patterns */
    printf("Test 4: Combined patterns...\n");
    for (int i = 0; i < 5; i++) {
        total += test_complex_live_ranges(2);
        total += test_unbalanced_flow(i * 7);
        total += test_switch_flow(i * 3);
    }
    
    printf("Total result: %d\n", total);
    printf("Test completed. Check compiler output for MCF debug messages.\n");
    
    return total > 0 ? 0 : 1;
}
