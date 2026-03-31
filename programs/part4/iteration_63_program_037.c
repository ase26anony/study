/* test_mcf_coverage.c
 * 
 * This test is designed to trigger the uncovered lines in GCC's mcf.cc
 * Specifically, the dump_fixup_edge function's handling of NEW_EXIT and NEW_ENTRY
 * nodes in the fixup graph during register allocation.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Declare many variables to create register pressure */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int result = 0;
    
    /* Initialize all variables with different values to prevent optimization */
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
    v20 = iterations * 21;
    v21 = iterations * 22;
    v22 = iterations * 23;
    v23 = iterations * 24;
    v24 = iterations * 25;
    v25 = iterations * 26;
    v26 = iterations * 27;
    v27 = iterations * 28;
    v28 = iterations * 29;
    v29 = iterations * 30;
    
    /* Nested loops with many live variables across loop boundaries */
    for (int i = 0; i < iterations; i++) {
        /* All variables are live here due to volatile accesses */
        asm volatile ("" : "+r" (v0), "+r" (v1), "+r" (v2), "+r" (v3), "+r" (v4));
        
        for (int j = 0; j < 10; j++) {
            /* More volatile accesses to keep variables live */
            asm volatile ("" : "+r" (v5), "+r" (v6), "+r" (v7), "+r" (v8), "+r" (v9));
            asm volatile ("" : "+r" (v10), "+r" (v11), "+r" (v12), "+r" (v13), "+r" (v14));
            
            /* Complex computation with many live ranges */
            v15 = v0 + v1 + v2 + v3 + v4;
            v16 = v5 + v6 + v7 + v8 + v9;
            v17 = v10 + v11 + v12 + v13 + v14;
            
            /* Conditional that creates different control flow paths */
            if ((i + j) % 3 == 0) {
                asm volatile ("" : "+r" (v18), "+r" (v19), "+r" (v20), "+r" (v21), "+r" (v22));
                v23 = v15 * v16 - v17;
            } else if ((i + j) % 3 == 1) {
                asm volatile ("" : "+r" (v24), "+r" (v25), "+r" (v26), "+r" (v27), "+r" (v28));
                v29 = v16 / (v15 + 1) + v17;
            } else {
                /* Use all remaining variables */
                asm volatile ("" : "+r" (v18), "+r" (v19), "+r" (v20), "+r" (v21), "+r" (v22));
                asm volatile ("" : "+r" (v24), "+r" (v25), "+r" (v26), "+r" (v27), "+r" (v28));
                v23 = v15 + v16 + v17;
                v29 = v23 * 2;
            }
            
            /* Force all variables to be used in result calculation */
            result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                     v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                     v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
        }
        
        /* Additional volatile asm that clobbers many registers */
        asm volatile ("# Force register clobber" : : : 
            "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    }
    
    return result;
}

/* Alternative function with different variable count to affect graph size */
__attribute__((noinline))
static int test_smaller_graph(int x) {
    /* Use exactly 10 variables to create a specific graph size */
    volatile int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9;
    int sum = 0;
    
    a0 = x;
    a1 = x * 2;
    a2 = x * 3;
    a3 = x * 4;
    a4 = x * 5;
    a5 = x * 6;
    a6 = x * 7;
    a7 = x * 8;
    a8 = x * 9;
    a9 = x * 10;
    
    /* Loop with all variables live */
    for (int i = 0; i < 100; i++) {
        asm volatile ("" : "+r" (a0), "+r" (a1), "+r" (a2), "+r" (a3), "+r" (a4));
        
        /* Switch statement creates complex control flow */
        switch (i % 4) {
            case 0:
                a5 = a0 + a1;
                a6 = a2 + a3;
                break;
            case 1:
                a7 = a1 + a2;
                a8 = a3 + a4;
                break;
            case 2:
                a9 = a4 + a5;
                a0 = a6 + a7;
                break;
            default:
                a1 = a7 + a8;
                a2 = a8 + a9;
                break;
        }
        
        sum += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
    }
    
    return sum;
}

/* Function with imbalanced register usage to force fixup edges */
__attribute__((noinline))
static int test_imbalanced_usage(int n) {
    /* Create scenario with more uses than definitions */
    volatile int x = n;
    volatile int y = n * 2;
    volatile int z = n * 3;
    int acc = 0;
    
    /* x, y, z are defined once but used many times */
    for (int i = 0; i < n; i++) {
        /* Many uses of the same variables */
        acc += x + y + z;
        acc += x * y - z;
        acc += y * z + x;
        acc += z * x - y;
        
        /* Conditional that creates different live ranges */
        if (i % 2 == 0) {
            volatile int t = x + y;  /* New definition */
            acc += t * z;
            asm volatile ("" : "+r" (t));
        } else {
            volatile int u = y + z;  /* New definition */
            acc += u * x;
            asm volatile ("" : "+r" (u));
        }
        
        /* Force spilling/reloading */
        asm volatile ("# Many clobbers" : : : 
            "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    }
    
    return acc;
}

/* Main function that exercises different scenarios */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Test with different iteration counts to explore different graph sizes */
    for (int i = 1; i <= 5; i++) {
        printf("Running test iteration %d\n", i);
        
        /* Large conflict graph with many variables */
        total += test_ira_conflict(i * 10);
        
        /* Medium-sized graph */
        total += test_smaller_graph(i * 5);
        
        /* Imbalanced usage pattern */
        total += test_imbalanced_usage(i * 3);
    }
    
    printf("Total result: %d\n", total);
    return total != 0 ? 0 : 1;
}
