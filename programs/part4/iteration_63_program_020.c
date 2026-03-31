/* test_mcf_coverage.c
 * 
 * This test is designed to trigger the uncovered lines in GCC's Min-Cost Flow
 * solver (mcf.cc) by creating register allocation scenarios that require
 * fixup graph construction with NEW_EXIT and NEW_ENTRY nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges through nested loops and volatile operations */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    int sum = 0;
    
    /* Initialize all variables with different values to prevent optimization */
    v0 = 1; v1 = 2; v2 = 3; v3 = 4; v4 = 5;
    v5 = 6; v6 = 7; v7 = 8; v8 = 9; v9 = 10;
    v10 = 11; v11 = 12; v12 = 13; v13 = 14; v14 = 15;
    v15 = 16; v16 = 17; v17 = 18; v18 = 19; v19 = 20;
    v20 = 21; v21 = 22; v22 = 23; v23 = 24; v24 = 25;
    v25 = 26; v26 = 27; v27 = 28; v28 = 29; v29 = 30;
    
    /* Nested loops to create complex liveness patterns */
    for (int i = 0; i < iterations; i++) {
        /* Many variables live across loop iterations */
        v0 = v0 + v1;
        v1 = v1 + v2;
        v2 = v2 + v3;
        v3 = v3 + v4;
        v4 = v4 + v5;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 5; j++) {
            /* Force register pressure by using many variables in computation */
            v5 = v5 + v6 + v0;
            v6 = v6 + v7 + v1;
            v7 = v7 + v8 + v2;
            v8 = v8 + v9 + v3;
            v9 = v9 + v10 + v4;
            
            /* Use inline asm to clobber registers, increasing pressure */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12"
            );
            
            /* More computations to keep variables live */
            v10 = v10 + v11 + v5;
            v11 = v11 + v12 + v6;
            v12 = v12 + v13 + v7;
            v13 = v13 + v14 + v8;
            v14 = v14 + v15 + v9;
        }
        
        /* Another set of operations to extend live ranges */
        v15 = v15 + v16;
        v16 = v16 + v17;
        v17 = v17 + v18;
        v18 = v18 + v19;
        v19 = v19 + v20;
        
        /* Use all variables in sum to prevent dead code elimination */
        sum += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
               v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
               v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    }
    
    return sum;
}

/* Alternative function with different register pressure pattern */
__attribute__((noinline))
static int test_ira_conflict2(int seed) {
    /* Use array to force more register/memory interactions */
    volatile int arr[20];
    int i, j, sum = 0;
    
    /* Initialize array */
    for (i = 0; i < 20; i++) {
        arr[i] = seed + i;
    }
    
    /* Complex loop with many live values */
    for (i = 0; i < 100; i++) {
        int t0 = arr[0];
        int t1 = arr[1];
        int t2 = arr[2];
        int t3 = arr[3];
        int t4 = arr[4];
        int t5 = arr[5];
        int t6 = arr[6];
        int t7 = arr[7];
        int t8 = arr[8];
        int t9 = arr[9];
        
        /* Chain computations to create dependency graph */
        for (j = 0; j < 10; j++) {
            t0 = t0 + t1;
            t1 = t1 + t2;
            t2 = t2 + t3;
            t3 = t3 + t4;
            t4 = t4 + t5;
            t5 = t5 + t6;
            t6 = t6 + t7;
            t7 = t7 + t8;
            t8 = t8 + t9;
            t9 = t9 + t0;
            
            /* Force spilling with large clobber list */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "add r0, r0, r1\n\t"
                "mov %0, r0\n\t"
                : "+r" (t0), "+r" (t1)
                :
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "cc", "memory"
            );
        }
        
        arr[0] = t0;
        arr[1] = t1;
        arr[2] = t2;
        arr[3] = t3;
        arr[4] = t4;
        
        sum += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9;
    }
    
    return sum;
}

/* Function with switch statement to create complex control flow */
__attribute__((noinline))
static int test_ira_conflict3(int mode) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    int result = 0;
    
    switch (mode % 5) {
        case 0:
            a = b + c;
            b = c + d;
            c = d + e;
            d = e + f;
            e = f + g;
            result = a + b;
            break;
        case 1:
            f = g + h;
            g = h + i;
            h = i + j;
            i = j + a;
            j = a + b;
            result = f + g;
            break;
        case 2:
            a = a + f;
            b = b + g;
            c = c + h;
            d = d + i;
            e = e + j;
            result = a + b + c;
            break;
        case 3:
            /* Force many live ranges across asm */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mov r2, %2\n\t"
                "mov r3, %3\n\t"
                "add r0, r0, r1\n\t"
                "add r2, r2, r3\n\t"
                "add r0, r0, r2\n\t"
                "mov %0, r0\n\t"
                : "+r" (a), "+r" (b), "+r" (c), "+r" (d)
                :
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "cc", "memory"
            );
            result = a + b + c + d;
            break;
        case 4:
            /* Use all variables */
            result = a + b + c + d + e + f + g + h + i + j;
            break;
    }
    
    return result;
}

/* Main function to exercise different patterns */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Call with different parameters to explore different allocation scenarios */
    total += test_ira_conflict(argc > 1 ? atoi(argv[1]) : 10);
    total += test_ira_conflict2(argc > 2 ? atoi(argv[2]) : 42);
    total += test_ira_conflict3(argc > 3 ? atoi(argv[3]) : 3);
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
