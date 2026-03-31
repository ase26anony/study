/* test_mcf_coverage.c
 * 
 * This test is designed to trigger the uncovered lines in mcf.cc's dump_fixup_edge
 * function by creating register allocation scenarios that force IRA to build
 * complex fixup graphs with NEW_EXIT and NEW_ENTRY nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Function with many overlapping live ranges to create complex conflict graph */
__attribute__((noinline))
static int test_complex_live_ranges(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    int sum = 0;
    
    /* Initialize all variables with different values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15;
    v16 = 16; v17 = 17; v18 = 18; v19 = 19; v20 = 20;
    v21 = 21; v22 = 22; v23 = 23; v24 = 24; v25 = 25;
    v26 = 26; v27 = 27; v28 = 28; v29 = 29; v30 = 30;
    
    /* Nested loops to create complex liveness patterns */
    for (int i = 0; i < iterations; i++) {
        /* All variables are live here - creates maximum register pressure */
        sum += v1 + v2 + v3 + v4 + v5;
        
        /* Volatile asm to clobber many registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : : : "memory"
        );
        
        for (int j = 0; j < 3; j++) {
            /* More overlapping live ranges */
            int temp1 = v6 + v7 + v8 + v9 + v10;
            int temp2 = v11 + v12 + v13 + v14 + v15;
            
            /* Complex computation keeping many variables live */
            v1 = v1 * 2 - v16;
            v2 = v2 * 3 - v17;
            v3 = v3 * 4 - v18;
            v4 = v4 * 5 - v19;
            v5 = v5 * 6 - v20;
            
            sum += temp1 + temp2;
            
            /* Another volatile asm to force spills */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                      "r8", "r9", "r10", "r11", "r12", "r14"
            );
        }
        
        /* Use remaining variables */
        sum += v21 + v22 + v23 + v24 + v25;
        sum += v26 + v27 + v28 + v29 + v30;
        
        /* Rotate values to create data dependencies */
        int tmp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = tmp;
        tmp = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = tmp;
    }
    
    /* Final computation using all variables */
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Function with imbalanced register usage to trigger fixup edges */
__attribute__((noinline))
static int test_imbalanced_flow(int seed) {
    /* Variables with different usage patterns to create supply/demand imbalance */
    int def_only[5];    /* Only defined, never used */
    int use_only[5];    /* Only used, never defined here */
    int many_uses[3];   /* Defined once, used many times */
    int many_defs[3];   /* Defined many times, used once */
    
    /* Initialize use_only from seed */
    for (int i = 0; i < 5; i++) {
        use_only[i] = seed + i;
    }
    
    /* Define def_only variables */
    for (int i = 0; i < 5; i++) {
        def_only[i] = seed * i;
        /* Never use def_only[i] - creates excess supply */
    }
    
    /* Variable with single definition but many uses */
    many_uses[0] = seed * 2;
    many_uses[1] = seed * 3;
    many_uses[2] = seed * 4;
    
    int result = 0;
    
    /* Create many uses of the same variables */
    for (int i = 0; i < 10; i++) {
        result += many_uses[0] * i;
        result += many_uses[1] * (i + 1);
        result += many_uses[2] * (i + 2);
        
        /* Use the use_only variables */
        result += use_only[i % 5];
        
        /* Many definitions of same variable */
        many_defs[0] = i * 10;
        many_defs[1] = i * 20;
        many_defs[2] = i * 30;
        
        /* Volatile asm to complicate allocation */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : : : "memory", "r0", "r1", "r2", "r3"
        );
    }
    
    /* Finally use the many_defs variables once */
    result += many_defs[0] + many_defs[1] + many_defs[2];
    
    return result;
}

/* Function with switch statement to create complex control flow */
__attribute__((noinline))
static int test_control_flow(int mode) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result = 0;
    
    /* Switch creates multiple control flow paths */
    switch (mode % 5) {
        case 0:
            result = a + b + c;
            /* Keep many variables live across asm */
            asm volatile ("nop\n\t" : : : "memory", "r0", "r1", "r2", "r3", "r4");
            result += d + e + f;
            break;
        case 1:
            result = g + h + i + j;
            asm volatile ("nop\n\t" : : : "memory", "r5", "r6", "r7", "r8");
            result += a + b;
            break;
        case 2:
            result = c + d + e + f + g;
            asm volatile ("nop\n\t" : : : "memory", "r9", "r10", "r11", "r12");
            result += h + i + j;
            break;
        case 3:
            /* Use all variables */
            result = a + b + c + d + e + f + g + h + i + j;
            asm volatile ("nop\n\t" : : : "memory", "r0", "r1", "r2", "r3", "r4",
                          "r5", "r6", "r7", "r8", "r9", "r10");
            break;
        case 4:
            /* Complex computation with all variables live */
            result = (a * b) + (c * d) + (e * f) + (g * h) + (i * j);
            asm volatile ("nop\n\t" : : : "memory", "r0", "r1", "r2", "r3", "r4",
                          "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12");
            break;
    }
    
    return result;
}

/* Function that creates register pressure by using many function calls */
__attribute__((noinline))
static int recursive_pressure(int n, int depth) {
    if (depth <= 0 || n <= 1) {
        return n;
    }
    
    int a = recursive_pressure(n - 1, depth - 1);
    int b = recursive_pressure(n - 2, depth - 1);
    int c = recursive_pressure(n - 3, depth - 1);
    int d = recursive_pressure(n - 4, depth - 1);
    int e = recursive_pressure(n - 5, depth - 1);
    
    /* All recursive results are live here */
    int result = a + b + c + d + e;
    
    /* Volatile asm to prevent optimization and clobber registers */
    asm volatile (
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
    );
    
    return result;
}

/* Main test driver that exercises different scenarios */
int main(void) {
    int total = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Test 1: Complex live ranges with many variables */
    printf("Test 1: Complex live ranges\n");
    for (int i = 0; i < 5; i++) {
        total += test_complex_live_ranges(10 + i);
    }
    
    /* Test 2: Imbalanced flow patterns */
    printf("Test 2: Imbalanced flow patterns\n");
    for (int i = 0; i < 10; i++) {
        total += test_imbalanced_flow(i * 100);
    }
    
    /* Test 3: Complex control flow */
    printf("Test 3: Complex control flow\n");
    for (int i = 0; i < 20; i++) {
        total += test_control_flow(i);
    }
    
    /* Test 4: Recursive register pressure */
    printf("Test 4: Recursive register pressure\n");
    total += recursive_pressure(10, 3);
    
    printf("Total result: %d\n", total);
    printf("Test completed. Check compiler output for MCF debug messages.\n");
    
    return total > 0 ? 0 : 1;
}
