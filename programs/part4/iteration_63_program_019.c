/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * Min-Cost Flow solver (mcf.cc) by creating register allocation
 * scenarios that require fixup graph construction with special nodes.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of IRA and MCF debugging */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with many overlapping live ranges to create complex conflict graph */
__attribute__((noinline))
static void test_ira_conflict_high_pressure(void) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
    volatile int v10, v11, v12, v13, v14, v15, v16, v17, v18, v19;
    
    /* Initialize with different values to prevent optimization */
    v0 = 0; v1 = 1; v2 = 2; v3 = 3; v4 = 4;
    v5 = 5; v6 = 6; v7 = 7; v8 = 8; v9 = 9;
    v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14;
    v15 = 15; v16 = 16; v17 = 17; v18 = 18; v19 = 19;
    
    /* Nested loops to extend live ranges and create register pressure */
    for (int i = 0; i < 100; i++) {
        /* All variables are live here in the outer loop */
        for (int j = 0; j < 50; j++) {
            /* Complex computation using all variables to keep them live */
            v0 = v1 + v2;
            v1 = v3 + v4;
            v2 = v5 + v6;
            v3 = v7 + v8;
            v4 = v9 + v10;
            v5 = v11 + v12;
            v6 = v13 + v14;
            v7 = v15 + v16;
            v8 = v17 + v18;
            v9 = v19 + v0;
            
            /* More computations to increase register pressure */
            v10 = v0 * v1;
            v11 = v2 * v3;
            v12 = v4 * v5;
            v13 = v6 * v7;
            v14 = v8 * v9;
            v15 = v10 * v11;
            v16 = v12 * v13;
            v17 = v14 * v15;
            v18 = v16 * v17;
            v19 = v18 * v0;
            
            /* Inline asm that clobbers many registers to force spills */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
        }
        
        /* Additional computation between loops */
        v0 = v19 - v18;
        v1 = v18 - v17;
        v2 = v17 - v16;
        v3 = v16 - v15;
        v4 = v15 - v14;
        
        /* Another asm that clobbers registers */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4"
        );
    }
    
    /* Final use to prevent dead code elimination */
    asm volatile ("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                      "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
                      "r"(v10), "r"(v11), "r"(v12), "r"(v13), "r"(v14),
                      "r"(v15), "r"(v16), "r"(v17), "r"(v18), "r"(v19));
}

/* Alternative function with different variable count to affect graph size */
__attribute__((noinline))
static void test_ira_conflict_medium_pressure(void) {
    volatile int a, b, c, d, e, f, g, h, i, j, k, l;
    
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6;
    g = 7; h = 8; i = 9; j = 10; k = 11; l = 12;
    
    /* Switch statement to create complex control flow */
    for (int iter = 0; iter < 100; iter++) {
        switch (iter % 8) {
            case 0: a = b + c; break;
            case 1: d = e + f; break;
            case 2: g = h + i; break;
            case 3: j = k + l; break;
            case 4: a = d - g; break;
            case 5: b = e - h; break;
            case 6: c = f - i; break;
            case 7: 
                /* All variables live here */
                a = b + c + d + e + f + g + h + i + j + k + l;
                asm volatile ("nop\n\t" : : : "memory", "r0", "r1", "r2", "r3");
                break;
        }
        
        /* Cross-loop dependencies */
        if (iter % 3 == 0) {
            b = a + 1;
            c = b + 1;
            d = c + 1;
        } else if (iter % 3 == 1) {
            e = d - 1;
            f = e - 1;
            g = f - 1;
        } else {
            h = g * 2;
            i = h * 2;
            j = i * 2;
        }
    }
    
    asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), "r"(f),
                      "r"(g), "r"(h), "r"(i), "r"(j), "r"(k), "r"(l));
}

/* Function designed to create supply/demand imbalance */
__attribute__((noinline))
static void test_ira_imbalance(void) {
    /* Create many uses but few definitions to force fixup edges */
    volatile int src1, src2, src3;
    volatile int dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9, dst10;
    
    src1 = 1;
    src2 = 2;
    src3 = 3;
    
    /* Many uses of the same sources */
    for (int i = 0; i < 100; i++) {
        dst1 = src1 + i;
        dst2 = src2 + i;
        dst3 = src3 + i;
        dst4 = src1 * i;
        dst5 = src2 * i;
        dst6 = src3 * i;
        dst7 = src1 - i;
        dst8 = src2 - i;
        dst9 = src3 - i;
        dst10 = src1 + src2 + src3 + i;
        
        /* Force all to be live simultaneously */
        asm volatile ("nop\n\t" : : : "memory");
    }
    
    /* Final use */
    asm volatile ("" : : "r"(dst1), "r"(dst2), "r"(dst3), "r"(dst4), "r"(dst5),
                      "r"(dst6), "r"(dst7), "r"(dst8), "r"(dst9), "r"(dst10));
}

/* Function with exactly 10 pseudo-registers for predictable graph size */
__attribute__((noinline))
static void test_predictable_graph(void) {
    /* Exactly 10 variables to potentially create specific graph size */
    register int r0 asm("r0") = 0;
    register int r1 asm("r1") = 1;
    register int r2 asm("r2") = 2;
    register int r3 asm("r3") = 3;
    register int r4 asm("r4") = 4;
    register int r5 asm("r5") = 5;
    register int r6 asm("r6") = 6;
    register int r7 asm("r7") = 7;
    register int r8 asm("r8") = 8;
    register int r9 asm("r9") = 9;
    
    /* Complex dependency chain */
    for (int i = 0; i < 1000; i++) {
        r0 = r1 + r2;
        r1 = r3 + r4;
        r2 = r5 + r6;
        r3 = r7 + r8;
        r4 = r9 + r0;
        r5 = r0 * r1;
        r6 = r2 * r3;
        r7 = r4 * r5;
        r8 = r6 * r7;
        r9 = r8 * r0;
        
        /* Force register pressure with inline asm */
        asm volatile (
            "add %0, %1, %2\n\t"
            "add %3, %4, %5\n\t"
            "add %6, %7, %8\n\t"
            : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4),
              "+r"(r5), "+r"(r6), "+r"(r7), "+r"(r8)
            :
            : "cc", "memory"
        );
    }
    
    /* Prevent optimization */
    asm volatile ("" : : "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4),
                      "r"(r5), "r"(r6), "r"(r7), "r"(r8), "r"(r9));
}

/* Main function that calls all test scenarios */
int main(void) {
    /* Call different test functions to explore various graph configurations */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_ira_conflict_high_pressure();
        test_ira_conflict_medium_pressure();
        test_ira_imbalance();
        test_predictable_graph();
    }
    
    return 0;
}

/* Additional test with array accesses to create memory pressure */
__attribute__((noinline))
static void test_array_pressure(void) {
    volatile int arr[20];
    volatile int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        arr[i] = i;
    }
    
    /* Complex array access pattern */
    for (int i = 0; i < 100; i++) {
        for (int j = 0; j < 10; j++) {
            arr[j] = arr[j + 10] + i;
            arr[j + 10] = arr[j] * j;
        }
        
        /* Register pressure with array elements */
        sum = arr[0] + arr[1] + arr[2] + arr[3] + arr[4] +
              arr[5] + arr[6] + arr[7] + arr[8] + arr[9];
        
        /* Force spills with asm */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
        );
    }
    
    asm volatile ("" : : "r"(sum));
}

/* Call this from main to add more pressure */
void extra_tests(void) {
    test_array_pressure();
}
