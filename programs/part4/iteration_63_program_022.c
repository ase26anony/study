/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * min-cost flow solver (mcf.cc) by creating register allocation
 * scenarios that require fixup graph construction with NEW_EXIT
 * and NEW_ENTRY nodes.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Force many overlapping live ranges through nested loops and volatile operations */
__attribute__((noinline))
void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize with different values to prevent constant propagation */
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
    
    /* Nested loops to create complex liveness patterns */
    for (int i = 0; i < iterations; i++) {
        /* All variables are live here at the start of the loop */
        int sum = v0 + v1 + v2 + v3 + v4;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 5; j++) {
            /* Force many variables to be live simultaneously */
            sum += v5 + v6 + v7 + v8 + v9;
            sum += v10 + v11 + v12 + v13 + v14;
            
            /* Use inline asm to clobber many registers */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mov r2, %2\n\t"
                "mov r3, %3\n\t"
                "mov r4, %4\n\t"
                "mov r5, %5\n\t"
                "mov r6, %6\n\t"
                "mov r7, %7\n\t"
                :
                : "r" (v15), "r" (v16), "r" (v17), "r" (v18),
                  "r" (v19), "r" (v20), "r" (v21), "r" (v22)
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
            );
            
            /* More computations keeping variables live */
            sum += v23 + v24 + v25 + v26 + v27 + v28 + v29;
        }
        
        /* Force spilling by using all variables again */
        v0 = sum + v1;
        v1 = v0 + v2;
        v2 = v1 + v3;
        v3 = v2 + v4;
        v4 = v3 + v5;
        
        /* Another asm block clobbering different registers */
        asm volatile (
            "add %0, %1, %2\n\t"
            "sub %3, %4, %5\n\t"
            "mul %6, %7, %8\n\t"
            :
            : "r" (v6), "r" (v7), "r" (v8),
              "r" (v9), "r" (v10), "r" (v11),
              "r" (v12), "r" (v13), "r" (v14)
            : "cc"
        );
    }
    
    /* Final use to prevent dead code elimination */
    asm volatile ("" : : "r" (v0), "r" (v1), "r" (v2), "r" (v3), "r" (v4));
    asm volatile ("" : : "r" (v5), "r" (v6), "r" (v7), "r" (v8), "r" (v9));
    asm volatile ("" : : "r" (v10), "r" (v11), "r" (v12), "r" (v13), "r" (v14));
    asm volatile ("" : : "r" (v15), "r" (v16), "r" (v17), "r" (v18), "r" (v19));
    asm volatile ("" : : "r" (v20), "r" (v21), "r" (v22), "r" (v23), "r" (v24));
    asm volatile ("" : : "r" (v25), "r" (v26), "r" (v27), "r" (v28), "r" (v29));
}

/* Alternative function with different register pressure pattern */
__attribute__((noinline))
void test_ira_conflict2(int seed) {
    /* Create a data flow pattern that might require fixup edges */
    int a = seed;
    int b = a * 2;
    int c = b + seed;
    int d = c * 3;
    int e = d - seed;
    int f = e / 2;
    int g = f + a;
    int h = g * b;
    int i = h - c;
    int j = i + d;
    int k = j * e;
    int l = k - f;
    int m = l + g;
    int n = m * h;
    int o = n - i;
    int p = o + j;
    int q = p * k;
    int r = q - l;
    int s = r + m;
    int t = s * n;
    
    /* Chain of dependencies forcing sequential evaluation */
    for (int x = 0; x < 100; x++) {
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
        k = l + m;
        l = m + n;
        m = n + o;
        n = o + p;
        o = p + q;
        p = q + r;
        q = r + s;
        r = s + t;
        s = t + a;
        t = a + b;
    }
    
    /* Force all variables to be live at the end */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Function with switch statement creating complex control flow */
__attribute__((noinline))
int test_ira_conflict3(int mode, int value) {
    int r0 = value;
    int r1 = r0 * 2;
    int r2 = r1 + 1;
    int r3 = r2 * 3;
    int r4 = r3 - 1;
    int r5 = r4 / 2;
    int r6 = r5 + r0;
    int r7 = r6 * r1;
    int r8 = r7 - r2;
    int r9 = r8 + r3;
    int r10 = r9 * r4;
    
    switch (mode) {
        case 0:
            return r0 + r1 + r2;
        case 1:
            return r3 + r4 + r5;
        case 2:
            return r6 + r7 + r8;
        case 3:
            return r9 + r10 + r0;
        case 4:
            /* All live here */
            asm volatile ("" : : "r" (r0), "r" (r1), "r" (r2), "r" (r3), "r" (r4));
            asm volatile ("" : : "r" (r5), "r" (r6), "r" (r7), "r" (r8), "r" (r9), "r" (r10));
            return r0 + r10;
        default:
            /* Complex computation with all variables */
            for (int i = 0; i < 10; i++) {
                r0 = r1 + r2;
                r1 = r2 + r3;
                r2 = r3 + r4;
                r3 = r4 + r5;
                r4 = r5 + r6;
                r5 = r6 + r7;
                r6 = r7 + r8;
                r7 = r8 + r9;
                r8 = r9 + r10;
                r9 = r10 + r0;
                r10 = r0 + r1;
            }
            return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    }
}

/* Main function to test different scenarios */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Test different functions to explore different conflict graph configurations */
    test_ira_conflict(iterations);
    test_ira_conflict2(iterations);
    
    for (int i = 0; i < 6; i++) {
        test_ira_conflict3(i, iterations + i);
    }
    
    /* Additional test with array operations */
    {
        int arr[50];
        for (int i = 0; i < 50; i++) {
            arr[i] = i * iterations;
        }
        
        /* Complex array computation creating many temporary values */
        for (int i = 0; i < 49; i++) {
            arr[i] = arr[i] * arr[i+1] - arr[i] / (iterations + 1);
            asm volatile ("" : : "r" (arr[i]));
        }
    }
    
    return 0;
}
