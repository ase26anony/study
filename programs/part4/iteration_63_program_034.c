/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * Min-Cost Flow solver (mcf.cc) by creating register allocation
 * scenarios that require complex fixup graph construction.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of MCF debugging code */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create complex liveness patterns */
volatile int global_seed = 42;

/* Function with many overlapping live ranges to create complex conflict graph */
int test_ira_conflict_1(int a, int b, int c, int d, int e) {
    /* Create many local variables with overlapping lifetimes */
    int v1 = a + b;
    int v2 = b * c;
    int v3 = c - d;
    int v4 = d / (e ? e : 1);
    int v5 = e + a;
    int v6 = v1 * v2;
    int v7 = v3 + v4;
    int v8 = v5 - v6;
    int v9 = v7 * v8;
    int v10 = v9 + v1;
    
    /* Nested loops with many live variables across iterations */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        /* v1-v10 are all live here, creating register pressure */
        sum += v1 + v2 + v3 + v4 + v5;
        
        /* Inner loop with more live variables */
        for (int j = 0; j < 10; j++) {
            /* Additional temporaries increase pressure */
            int t1 = v6 * j;
            int t2 = v7 + j;
            int t3 = v8 - j;
            int t4 = v9 / (j ? j : 1);
            int t5 = v10 + j;
            
            /* All these are live simultaneously */
            sum += t1 + t2 + t3 + t4 + t5 + v1 + v2;
            
            /* Volatile asm to clobber registers and force spills */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mov r2, %2\n\t"
                : 
                : "r" (t1), "r" (t2), "r" (t3)
                : "r0", "r1", "r2", "memory"
            );
        }
        
        /* Modify variables to prevent dead code elimination */
        v1 += i;
        v2 -= i;
        v3 *= (i & 1) ? 2 : 1;
        v4 = v4 > 0 ? v4 : 1;
    }
    
    /* Final computation using all variables */
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Another test with different pattern to explore different graph sizes */
int test_ira_conflict_2(int iterations) {
    /* Create exactly 20 variables to target specific graph size */
    int var[20];
    
    /* Initialize with pattern */
    for (int i = 0; i < 20; i++) {
        var[i] = i * global_seed;
    }
    
    int result = 0;
    
    /* Complex loop structure with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* First group of variables live */
        int a = var[0] + var[1];
        int b = var[2] * var[3];
        int c = var[4] - var[5];
        
        /* Middle computation - many temporaries */
        for (int j = 0; j < 5; j++) {
            /* Variables a, b, c still live */
            int d = var[6 + j] + a;
            int e = var[11 + j] * b;
            int f = c - j;
            
            /* All 6 temporaries live simultaneously */
            result += d + e + f;
            
            /* Force register clobbering */
            asm volatile (
                "# Force register pressure\n\t"
                :
                :
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "memory"
            );
        }
        
        /* Update source variables to create data flow */
        var[0] += result;
        var[1] -= result;
        var[2] = var[2] > 0 ? var[2] : 1;
    }
    
    /* Final sum of all variables */
    for (int i = 0; i < 20; i++) {
        result += var[i];
    }
    
    return result;
}

/* Test with artificial source/sink imbalance */
int test_ira_conflict_3(void) {
    /* Create scenario with more uses than definitions */
    int def1 = global_seed;
    int def2 = def1 * 2;
    int def3 = def2 + 1;
    
    /* Many uses of the same variables */
    int use1, use2, use3, use4, use5, use6, use7, use8, use9, use10;
    
    /* Chain of computations creating demand > supply */
    use1 = def1 + def2;
    use2 = use1 * def3;
    use3 = use2 - def1;
    use4 = use3 / (def2 ? def2 : 1);
    use5 = use4 + def3;
    use6 = use5 * use1;
    use7 = use6 - use2;
    use8 = use7 + use3;
    use9 = use8 * use4;
    use10 = use9 - use5;
    
    /* All uses live simultaneously in loop */
    int total = 0;
    for (int i = 0; i < 50; i++) {
        total += use1 + use2 + use3 + use4 + use5 + 
                use6 + use7 + use8 + use9 + use10;
        
        /* Force spilling with inline asm */
        asm volatile (
            "# Clobber everything\n\t"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r14", "memory"
        );
    }
    
    return total + def1 + def2 + def3;
}

/* Test targeting specific architecture constraints */
#ifdef __arm__
int test_arm_specific(void) {
    /* ARM has only 16 general purpose registers - easier to pressure */
    register int r0 asm("r0") = 1;
    register int r1 asm("r1") = 2;
    register int r2 asm("r2") = 3;
    register int r3 asm("r3") = 4;
    register int r4 asm("r4") = 5;
    register int r5 asm("r5") = 6;
    register int r6 asm("r6") = 7;
    register int r7 asm("r7") = 8;
    register int r8 asm("r8") = 9;
    register int r9 asm("r9") = 10;
    register int r10 asm("r10") = 11;
    
    /* Already using 11 registers, leaving only 5 free */
    int v1 = r0 + r1;
    int v2 = r2 * r3;
    int v3 = r4 - r5;
    int v4 = r6 / (r7 ? r7 : 1);
    int v5 = r8 + r9;
    int v6 = r10 * v1;
    int v7 = v2 + v3;
    int v8 = v4 - v5;
    int v9 = v6 * v7;
    int v10 = v8 + v9;
    int v11 = v10 - v1;
    int v12 = v11 * v2;
    int v13 = v12 + v3;
    int v14 = v13 - v4;
    int v15 = v14 * v5;
    
    /* All 15 variables live - must spill on ARM */
    asm volatile (
        "# ARM register pressure test\n\t"
        "add %0, %0, %1\n\t"
        "mul %1, %1, %2\n\t"
        : "+r" (v1), "+r" (v2)
        : "r" (v3)
        : "cc", "memory"
    );
    
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + r0 + r1 + r2 + r3 + r4 +
           r5 + r6 + r7 + r8 + r9 + r10;
}
#endif

/* Main function to run all tests with different parameters */
int main(int argc, char **argv) {
    int result = 0;
    
    printf("Starting MCF coverage test...\n");
    
    /* Run test 1 multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        result += test_ira_conflict_1(i, i+1, i+2, i+3, i+4);
    }
    
    /* Run test 2 with varying iteration counts */
    result += test_ira_conflict_2(20);
    result += test_ira_conflict_2(5);
    result += test_ira_conflict_2(50);
    
    /* Run test 3 */
    result += test_ira_conflict_3();
    
    /* Run architecture-specific test if applicable */
    #ifdef __arm__
    result += test_arm_specific();
    #endif
    
    printf("Final result: %d\n", result);
    
    /* Use result to prevent dead code elimination */
    return result > 0 ? 0 : 1;
}

/* Additional helper to create even more complex scenarios */
void __attribute__((noinline)) 
create_complex_liveness(int *out, int n) {
    /* Function with many basic blocks and phi nodes */
    int x = 0, y = 0, z = 0;
    
    for (int i = 0; i < n; i++) {
        if (i & 1) {
            x = x * 2 + 1;
            y = y - i;
        } else {
            x = x / 2;
            y = y + i;
        }
        
        /* Switch-like structure for more control flow */
        switch (i % 4) {
            case 0: z = x + y; break;
            case 1: z = x - y; break;
            case 2: z = x * y; break;
            case 3: z = y ? x / y : 0; break;
        }
        
        /* All variables live here */
        out[i] = x + y + z;
        
        /* Memory barrier to prevent optimizations */
        asm volatile ("" ::: "memory");
    }
}
