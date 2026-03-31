/* test_mcf_coverage.c
 * 
 * This test program is designed to trigger the uncovered lines in GCC's
 * min-cost flow solver (mcf.cc) during integrated register allocation (IRA).
 * Specifically, it aims to cause the dump_fixup_edge function to be called
 * with node indices equal to fixup_graph->new_exit_index and 
 * fixup_graph->new_entry_index.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
__attribute__((noinline))
void test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    volatile int v20, v21, v22, v23, v24, v25, v26, v27, v28, v29;
    
    /* Initialize all variables to create many definitions */
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
    v23 = iterations * 23;
    v24 = iterations * 24;
    v25 = iterations * 25;
    v26 = iterations * 26;
    v27 = iterations * 27;
    v28 = iterations * 28;
    v29 = iterations * 29;
    
    /* Nested loops with many live variables across loop boundaries */
    for (int i = 0; i < iterations; i++) {
        /* All variables are live here due to the volatile reads below */
        int sum = 0;
        
        /* Inner loop with complex register pressure */
        for (int j = 0; j < 100; j++) {
            /* Use many variables in computation to keep them live */
            sum += v0 + v1 + v2 + v3 + v4;
            sum += v5 + v6 + v7 + v8 + v9;
            sum += v10 + v11 + v12 + v13 + v14;
            
            /* Inline asm that clobbers many registers to increase pressure */
            asm volatile (
                "mov r0, %0\n"
                "mov r1, %1\n"
                "mov r2, %2\n"
                "mov r3, %3\n"
                "add r0, r0, r1\n"
                "add r2, r2, r3\n"
                "mul r0, r0, r2\n"
                : 
                : "r" (v15), "r" (v16), "r" (v17), "r" (v18)
                : "r0", "r1", "r2", "r3", "cc", "memory"
            );
            
            /* More computations with different variable groups */
            if (j % 2 == 0) {
                sum += v19 + v20 + v21 + v23 + v24;
            } else {
                sum += v25 + v26 + v27 + v28 + v29;
            }
        }
        
        /* Force spilling by using all variables in a volatile context */
        asm volatile (
            "/* Clobber many registers to force spill/reload */\n"
            "mov r4, %0\n"
            "mov r5, %1\n"
            "mov r6, %2\n"
            "mov r7, %3\n"
            "mov r8, %4\n"
            "mov r9, %5\n"
            "mov r10, %6\n"
            "mov r11, %7\n"
            "add r4, r4, r5\n"
            "add r6, r6, r7\n"
            "add r8, r8, r9\n"
            "add r10, r10, r11\n"
            : 
            : "r" (v0), "r" (v1), "r" (v2), "r" (v3),
              "r" (v4), "r" (v5), "r" (v6), "r" (v7)
            : "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "cc", "memory"
        );
        
        /* Update variables to create new definitions */
        v0 = sum % 100;
        v1 = (sum + v0) % 100;
        v2 = (sum + v1) % 100;
        v3 = (sum + v2) % 100;
        v4 = (sum + v3) % 100;
    }
    
    /* Final use of all variables to ensure they're live until the end */
    int final_result = 
        v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
        v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
        v20 + v21 + v23 + v24 + v25 + v26 + v27 + v28 + v29;
    
    /* Prevent dead code elimination */
    asm volatile ("" : : "r" (final_result));
}

/* Alternative test with switch-case to create complex control flow */
__attribute__((noinline))
void test_ira_switch(int mode) {
    volatile int a = 1, b = 2, c = 3, d = 4, e = 5;
    volatile int f = 6, g = 7, h = 8, i = 9, j = 10;
    volatile int k = 11, l = 12, m = 13, n = 14, o = 15;
    
    /* Complex switch creates many basic blocks with different live ranges */
    switch (mode % 8) {
        case 0:
            a = b + c;
            /* Use many variables */
            d = e + f + g + h;
            break;
        case 1:
            i = j * k;
            l = m - n;
            o = a + b;
            break;
        case 2:
            /* All variables live here */
            a = b + c + d + e + f + g + h + i + j + k + l + m + n + o;
            break;
        case 3:
            /* Nested computation */
            for (int x = 0; x < 10; x++) {
                a += b * x;
                c += d * x;
                e += f * x;
            }
            break;
        case 4:
            /* Another asm clobber */
            asm volatile (
                "mov r0, %0\n"
                "mov r1, %1\n"
                "mov r2, %2\n"
                "mov r3, %3\n"
                "mov r4, %4\n"
                "mov r5, %5\n"
                : 
                : "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f)
                : "r0", "r1", "r2", "r3", "r4", "r5", "cc", "memory"
            );
            break;
        case 5:
        case 6:
        case 7:
            /* Default path with all variables */
            a = b = c = d = e = f = g = h = i = j = k = l = m = n = o = mode;
            break;
    }
    
    /* Force all variables to be used */
    volatile int result = 
        a + b + c + d + e + f + g + h + i + j + k + l + m + n + o;
    
    asm volatile ("" : : "r" (result));
}

/* Test with artificial register pressure via many function arguments */
__attribute__((noinline))
int test_many_args(int a1, int a2, int a3, int a4, int a5,
                   int a6, int a7, int a8, int a9, int a10,
                   int a11, int a12, int a13, int a14, int a15) {
    /* Create many local variables that conflict with arguments */
    volatile int l1 = a1 * 2, l2 = a2 * 3, l3 = a3 * 4, l4 = a4 * 5;
    volatile int l5 = a5 * 6, l6 = a6 * 7, l7 = a7 * 8, l8 = a8 * 9;
    volatile int l9 = a9 * 10, l10 = a10 * 11, l11 = a11 * 12;
    volatile int l12 = a12 * 13, l13 = a13 * 14, l14 = a14 * 15;
    volatile int l15 = a15 * 16;
    
    /* Complex computation keeping all live */
    for (int i = 0; i < 50; i++) {
        l1 = l1 + l2 + l3;
        l4 = l4 + l5 + l6;
        l7 = l7 + l8 + l9;
        l10 = l10 + l11 + l12;
        l13 = l13 + l14 + l15;
        
        /* Mix with arguments */
        if (i % 3 == 0) {
            l1 += a1;
            l4 += a4;
            l7 += a7;
            l10 += a10;
            l13 += a13;
        }
    }
    
    return l1 + l4 + l7 + l10 + l13;
}

/* Main function that exercises different IRA scenarios */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    printf("Testing IRA min-cost flow coverage...\n");
    
    /* Test 1: Complex nested loops with many volatile variables */
    test_ira_conflict(iterations);
    
    /* Test 2: Switch-based control flow */
    for (int i = 0; i < 20; i++) {
        test_ira_switch(i);
    }
    
    /* Test 3: Many arguments creating register pressure */
    int result = test_many_args(
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
        11, 12, 13, 14, 15
    );
    
    printf("Result: %d\n", result);
    
    return 0;
}
