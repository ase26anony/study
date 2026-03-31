/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver
 * to create fixup graphs with NEW_EXIT and NEW_ENTRY nodes,
 * covering the debug dumping code in mcf.cc.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
void test_ira_conflict(int iterations) {
    /* Declare many variables to create many pseudo-registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with different values to prevent optimization */
    v1 = iterations * 1;
    v2 = iterations * 2;
    v3 = iterations * 3;
    v4 = iterations * 4;
    v5 = iterations * 5;
    v6 = iterations * 6;
    v7 = iterations * 7;
    v8 = iterations * 8;
    v9 = iterations * 9;
    v10 = iterations * 10;
    v11 = iterations * 11;
    v12 = iterations * 12;
    v13 = iterations * 13;
    v14 = iterations * 14;
    v15 = iterations * 15;
    v16 = iterations * 16;
    v17 = iterations * 17;
    v18 = iterations * 18;
    v19 = iterations * 19;
    v20 = iterations * 20;
    v21 = iterations * 21;
    v22 = iterations * 22;
    v23 = iterations * 23;
    v24 = iterations * 24;
    v25 = iterations * 25;
    v26 = iterations * 26;
    v27 = iterations * 27;
    v28 = iterations * 28;
    v29 = iterations * 29;
    v30 = iterations * 30;
    
    /* Nested loops to create complex liveness intervals */
    for (int i = 0; i < iterations; i++) {
        /* Many variables live across loop iterations */
        v1 += v2;
        v3 += v4;
        v5 += v6;
        v7 += v8;
        v9 += v10;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 5; j++) {
            /* All these variables are live in the inner loop */
            v11 = v1 + v2 + v3;
            v12 = v4 + v5 + v6;
            v13 = v7 + v8 + v9;
            v14 = v10 + v11 + v12;
            v15 = v13 + v14 + v1;
            
            /* Volatile asm to clobber registers and increase pressure */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory"
            );
            
            /* More computations to keep variables live */
            v16 = v15 + v14 + v13;
            v17 = v16 + v15 + v14;
            v18 = v17 + v16 + v15;
            v19 = v18 + v17 + v16;
            v20 = v19 + v18 + v17;
            
            /* Another asm that clobbers many registers */
            asm volatile (
                "mov r0, r0\n\t"
                "mov r1, r1\n\t"
                "mov r2, r2\n\t"
                "mov r3, r3\n\t"
                : 
                : 
                : "r0", "r1", "r2", "r3", "memory"
            );
            
            v21 = v20 + v19 + v18;
            v22 = v21 + v20 + v19;
            v23 = v22 + v21 + v20;
            v24 = v23 + v22 + v21;
            v25 = v24 + v23 + v22;
            
            /* Conditional to create control flow divergence */
            if (j % 2 == 0) {
                v26 = v25 + v24 + v23;
                v27 = v26 + v25 + v24;
            } else {
                v28 = v25 + v24 + v23;
                v29 = v28 + v25 + v24;
                v30 = v29 + v28 + v27;
            }
        }
        
        /* Force all variables to be used to prevent dead code elimination */
        v1 = v30 - v29;
        v2 = v29 - v28;
        v3 = v28 - v27;
        v4 = v27 - v26;
        v5 = v26 - v25;
    }
    
    /* Final use to ensure all variables are live at some point */
    printf("%d %d %d %d %d\n", v1, v2, v3, v4, v5);
    printf("%d %d %d %d %d\n", v6, v7, v8, v9, v10);
    printf("%d %d %d %d %d\n", v11, v12, v13, v14, v15);
    printf("%d %d %d %d %d\n", v16, v17, v18, v19, v20);
    printf("%d %d %d %d %d\n", v21, v22, v23, v24, v25);
    printf("%d %d %d %d %d\n", v26, v27, v28, v29, v30);
}

/* Another test function with different register pressure pattern */
__attribute__((noinline))
void test_imbalance_supply_demand(int n) {
    /* Create imbalance: more uses than definitions */
    int a = n;
    int b = n * 2;
    int c = n * 3;
    int d = n * 4;
    int e = n * 5;
    
    /* Many uses of the same variables */
    for (int i = 0; i < n; i++) {
        /* Chain of dependencies to create long live ranges */
        a = a + b + c;
        b = b + c + d;
        c = c + d + e;
        d = d + e + a;
        e = e + a + b;
        
        /* Use all variables multiple times */
        a = a * 2 - b;
        b = b * 2 - c;
        c = c * 2 - d;
        d = d * 2 - e;
        e = e * 2 - a;
        
        /* More complex expressions to increase register pressure */
        int t1 = a + b;
        int t2 = c + d;
        int t3 = e + a;
        int t4 = b + c;
        int t5 = d + e;
        
        /* All temporaries used together */
        a = t1 + t2;
        b = t2 + t3;
        c = t3 + t4;
        d = t4 + t5;
        e = t5 + t1;
        
        /* Clobber registers to force spills */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory"
        );
    }
    
    /* Force output to prevent optimization */
    printf("Imbalance test: %d %d %d %d %d\n", a, b, c, d, e);
}

/* Test with switch statement for complex control flow */
__attribute__((noinline))
void test_complex_control_flow(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int y1 = 6, y2 = 7, y3 = 8, y4 = 9, y5 = 10;
    int z1 = 11, z2 = 12, z3 = 13, z4 = 14, z5 = 15;
    
    switch (mode % 5) {
        case 0:
            x1 = y1 + z1;
            x2 = y2 + z2;
            x3 = y3 + z3;
            break;
        case 1:
            x4 = y4 + z4;
            x5 = y5 + z5;
            y1 = x1 + z1;
            break;
        case 2:
            y2 = x2 + z2;
            y3 = x3 + z3;
            y4 = x4 + z4;
            break;
        case 3:
            z1 = x1 + y1;
            z2 = x2 + y2;
            z3 = x3 + y3;
            z4 = x4 + y4;
            break;
        case 4:
            z5 = x5 + y5;
            x1 = y1 + z1 + x2 + y2 + z2;
            x2 = y2 + z2 + x3 + y3 + z3;
            x3 = y3 + z3 + x4 + y4 + z4;
            x4 = y4 + z4 + x5 + y5 + z5;
            break;
    }
    
    /* Make all variables live at the end */
    printf("Control flow: %d %d %d %d %d\n", x1, x2, x3, x4, x5);
    printf("            : %d %d %d %d %d\n", y1, y2, y3, y4, y5);
    printf("            : %d %d %d %d %d\n", z1, z2, z3, z4, z5);
}

/* Main function that exercises different patterns */
int main(int argc, char **argv) {
    int iterations = 10;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 10;
    }
    
    /* Test 1: Many overlapping live ranges */
    test_ira_conflict(iterations);
    
    /* Test 2: Imbalance in supply/demand */
    test_imbalance_supply_demand(iterations);
    
    /* Test 3: Complex control flow */
    for (int i = 0; i < 5; i++) {
        test_complex_control_flow(i);
    }
    
    return 0;
}
