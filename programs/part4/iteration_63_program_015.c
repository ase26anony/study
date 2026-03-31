/* test_mcf_coverage.c
 * 
 * This test is designed to trigger GCC's min-cost flow solver during IRA
 * (Integrated Register Allocator) to cover the debug dumping code that
 * prints special node labels like "NEW_EXIT" and "NEW_ENTRY".
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of necessary headers for IRA/MCF internals */
#define __GCC_INSN_CODES_H
#define RTL_H

/* Function to create complex register pressure scenario */
int test_ira_conflict(int iterations) {
    /* Declare many variables with overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize variables to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across inner loop */
        int temp1 = a + b;
        int temp2 = c + d;
        
        /* Inner loop with register pressure */
        for (int inner = 0; inner < 100; inner++) {
            /* Complex computation keeping many variables live */
            e = temp1 + inner;
            f = temp2 + inner * 2;
            g = e + f + a;
            h = g + b + c;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile (
                "/* Clobber many registers */"
                :
                : 
                : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
            );
            
            /* More computations with different variable combinations */
            i = h + d + e;
            j = i + f + g;
            k = j + h + i;
            l = k + j + temp1;
            
            /* Another asm statement with different clobbers */
            asm volatile (
                "nop"
                : 
                : 
                : "memory", "cc"
            );
            
            m = l + k + temp2;
            n = m + l + a;
            o = n + m + b;
            p = o + n + c;
            q = p + o + d;
            r = q + p + e;
            s = r + q + f;
            t = s + r + g;
            
            /* Accumulate result to prevent dead code elimination */
            result += t + s + h;
        }
        
        /* Rotate variable usage to create different conflict patterns */
        int swap = a;
        a = b; b = c; c = d; d = e; e = f;
        f = g; g = h; h = i; i = j; j = k;
        k = l; l = m; m = n; n = o; o = p;
        p = q; q = r; r = s; s = t; t = swap;
        
        /* Conditional that forces different register allocation decisions */
        if (outer % 3 == 0) {
            /* Force spill/reload scenario */
            asm volatile (
                "/* Force memory operations */"
                :
                :
                : "memory"
            );
            
            /* Recompute some values */
            temp1 = a * 2 + b;
            temp2 = c * 3 + d;
        } else if (outer % 3 == 1) {
            /* Different computation pattern */
            temp1 = e + f + g;
            temp2 = h + i + j;
        } else {
            /* Yet another pattern */
            temp1 = k + l + m;
            temp2 = n + o + p;
        }
    }
    
    return result;
}

/* Second test function with different register pressure pattern */
int test_ira_conflict2(int seed) {
    /* Use array to create indexed access pattern */
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = seed + i;
    }
    
    int sum = 0;
    
    /* Complex loop with many intermediate values */
    for (int i = 0; i < 1000; i++) {
        /* Many parallel computations */
        int t0 = arr[0] + i;
        int t1 = arr[1] + t0;
        int t2 = arr[2] + t1;
        int t3 = arr[3] + t2;
        int t4 = arr[4] + t3;
        int t5 = arr[5] + t4;
        int t6 = arr[6] + t5;
        int t7 = arr[7] + t6;
        int t8 = arr[8] + t7;
        int t9 = arr[9] + t8;
        int t10 = arr[10] + t9;
        int t11 = arr[11] + t10;
        int t12 = arr[12] + t11;
        int t13 = arr[13] + t12;
        int t14 = arr[14] + t13;
        int t15 = arr[15] + t14;
        int t16 = arr[16] + t15;
        int t17 = arr[17] + t16;
        int t18 = arr[18] + t17;
        int t19 = arr[19] + t18;
        
        /* Force register pressure with asm */
        asm volatile (
            "/* Clobber working registers */"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
        );
        
        /* More computations */
        sum += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 +
               t10 + t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19;
        
        /* Update array elements to create data dependencies */
        for (int j = 0; j < 20; j++) {
            arr[j] = (arr[j] + sum) & 0xFF;
        }
    }
    
    return sum;
}

/* Third test: Function with switch statement creating different
 * control flow paths with varying register usage */
int test_ira_conflict3(int mode, int value) {
    int x1 = value, x2 = value * 2, x3 = value * 3;
    int x4 = value * 4, x5 = value * 5, x6 = value * 6;
    int x7 = value * 7, x8 = value * 8, x9 = value * 9;
    int x10 = value * 10;
    
    switch (mode % 5) {
        case 0:
            /* Path 0: Many variables live */
            asm volatile ("nop" : : : "r0", "r1", "r2");
            return x1 + x2 + x3 + x4 + x5;
        case 1:
            /* Path 1: Different combination */
            asm volatile ("nop" : : : "r3", "r4", "r5");
            return x6 + x7 + x8 + x9 + x10;
        case 2:
            /* Path 2: Mix of variables */
            asm volatile ("nop" : : : "r6", "r7", "r8");
            return x1 + x3 + x5 + x7 + x9;
        case 3:
            /* Path 3: Another mix */
            asm volatile ("nop" : : : "r9", "r10", "r11");
            return x2 + x4 + x6 + x8 + x10;
        case 4:
            /* Path 4: All variables */
            asm volatile ("nop" : : : "r12", "r13", "r14", "r15");
            return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    }
    
    return 0;
}

/* Main function to drive tests with different parameters */
int main() {
    int total = 0;
    
    /* Test with different iteration counts to create different
     * conflict graph sizes */
    for (int test_case = 0; test_case < 10; test_case++) {
        total += test_ira_conflict(test_case + 1);
        total += test_ira_conflict2(test_case * 100);
        total += test_ira_conflict3(test_case, test_case * 50);
    }
    
    /* Prevent optimization of entire program */
    asm volatile ("" : : "r"(total));
    
    return total % 256;
}
