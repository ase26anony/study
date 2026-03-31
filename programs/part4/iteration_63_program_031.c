/* test_mcf_coverage.c
 * 
 * This program creates a complex register allocation scenario that forces
 * GCC's IRA to build a fixup graph with NEW_EXIT/NEW_ENTRY nodes.
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force MCF_DEBUG to be defined if not already */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Complex function with many overlapping live ranges */
int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize all variables with different values */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the inner loop */
        int temp1 = a + b + c + d + e;
        int temp2 = f + g + h + i + j;
        
        /* Volatile asm that clobbers many registers */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "add r0, r0, r1\n\t"
            : 
            : "r" (temp1), "r" (temp2)
            : "r0", "r1", "cc", "memory"
        );
        
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
            a = b + c * inner;
            b = c + d / (inner + 1);
            c = d + e - inner;
            d = e + f % (inner + 2);
            e = f + g ^ inner;
            
            /* More volatile asm with different clobbers */
            asm volatile (
                "mov r2, %0\n\t"
                "mov r3, %1\n\t"
                "mul r2, r2, r3\n\t"
                : 
                : "r" (a), "r" (b)
                : "r2", "r3", "cc"
            );
            
            /* Use remaining variables to keep them live */
            f = g + h + inner;
            g = h + i * inner;
            h = i + j - inner;
            i = j + k / (inner + 1);
            j = k + l % (inner + 2);
            
            /* Force spilling by using all variables in computation */
            result += a + b + c + d + e + f + g + h + i + j +
                     k + l + m + n + o + p + q + r + s + t;
        }
        
        /* Another asm block clobbering different registers */
        asm volatile (
            "mov r4, %0\n\t"
            "mov r5, %1\n\t"
            "sub r4, r4, r5\n\t"
            : 
            : "r" (k), "r" (l)
            : "r4", "r5", "cc"
        );
        
        /* Rotate values to create data dependencies */
        int tmp = a;
        a = b; b = c; c = d; d = e; e = f;
        f = g; g = h; h = i; i = j; j = k;
        k = l; l = m; m = n; n = o; o = p;
        p = q; q = r; r = s; s = t; t = tmp;
    }
    
    /* Final computation using all variables */
    result = a + b + c + d + e + f + g + h + i + j +
            k + l + m + n + o + p + q + r + s + t;
    
    return result;
}

/* Second test function with different pattern to explore more graph configurations */
int test_ira_conflict2(int seed) {
    /* Array of variables to create indexed access pattern */
    int vars[20];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        vars[i] = seed + i;
    }
    
    /* Complex loop with pointer aliasing */
    for (int i = 0; i < 100; i++) {
        int *ptr1 = &vars[i % 10];
        int *ptr2 = &vars[10 + (i % 10)];
        
        /* Force register pressure with many simultaneous computations */
        int t1 = *ptr1 + vars[0];
        int t2 = *ptr2 + vars[1];
        int t3 = vars[2] + vars[3];
        int t4 = vars[4] + vars[5];
        int t5 = vars[6] + vars[7];
        int t6 = vars[8] + vars[9];
        int t7 = vars[10] + vars[11];
        int t8 = vars[12] + vars[13];
        int t9 = vars[14] + vars[15];
        int t10 = vars[16] + vars[17];
        
        /* Asm that clobbers many registers */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"
            "add r0, r0, r1\n\t"
            "add r2, r2, r3\n\t"
            : 
            : "r" (t1), "r" (t2), "r" (t3), "r" (t4)
            : "r0", "r1", "r2", "r3", "cc"
        );
        
        /* Update all array elements to keep them live */
        for (int j = 0; j < 20; j++) {
            vars[j] += (i * j) % 7;
        }
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    }
    
    return sum;
}

/* Third test: Function with switch statement creating irregular control flow */
int test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    int result = 0;
    
    switch (mode % 5) {
        case 0:
            /* Many variables live in this case */
            result = x1 + x2 + x3 + x4 + x5;
            asm volatile ("" : : "r" (x1), "r" (x2), "r" (x3), "r" (x4), "r" (x5) : "memory");
            break;
        case 1:
            result = x6 + x7 + x8 + x9 + x10;
            asm volatile ("" : : "r" (x6), "r" (x7), "r" (x8), "r" (x9), "r" (x10) : "memory");
            break;
        case 2:
            /* Mix variables from both groups */
            result = x1 + x3 + x5 + x7 + x9;
            asm volatile ("" : : "r" (x1), "r" (x3), "r" (x5), "r" (x7), "r" (x9) : "memory");
            break;
        case 3:
            /* Use all variables */
            result = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
            asm volatile ("" : : "r" (x1), "r" (x2), "r" (x3), "r" (x4), "r" (x5),
                                       "r" (x6), "r" (x7), "r" (x8), "r" (x9), "r" (x10) : "memory");
            break;
        case 4:
            /* Complex computation */
            for (int i = 0; i < 10; i++) {
                x1 = x2 + i;
                x2 = x3 * i;
                x3 = x4 - i;
                x4 = x5 / (i + 1);
                x5 = x6 % (i + 2);
                x6 = x7 + x8;
                x7 = x8 * x9;
                x8 = x9 - x10;
                x9 = x10 + i;
                x10 = x1 * 2;
                result += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
            }
            break;
    }
    
    return result;
}

/* Main function to call all test cases with different parameters */
int main() {
    int total = 0;
    
    /* Call first test with different iteration counts */
    for (int i = 0; i < 5; i++) {
        total += test_ira_conflict(i + 1);
    }
    
    /* Call second test with different seeds */
    for (int i = 0; i < 10; i++) {
        total += test_ira_conflict2(i * 7);
    }
    
    /* Call third test with different modes */
    for (int i = 0; i < 15; i++) {
        total += test_ira_conflict3(i);
    }
    
    return total % 256; /* Return non-zero to prevent optimization */
}
