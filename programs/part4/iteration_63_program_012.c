/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * Min-Cost Flow solver (mcf.cc) by creating register allocation
 * scenarios that require fixup graph construction with special nodes.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

/* Force inclusion of IRA/MCF debugging code */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with many overlapping live ranges to create complex conflict graph */
__attribute__((noinline))
static void test_ira_conflict_high_pressure(void) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize with different values to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8;
    i = 9; j = 10; k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to extend live ranges and create register pressure */
    for (int outer = 0; outer < 100; outer++) {
        /* All variables are live here in the outer loop */
        for (int inner = 0; inner < 50; inner++) {
            /* Complex computation with all variables live */
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
            
            /* Inline asm that clobbers many registers to increase pressure */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mov r2, %2\n\t"
                "mov r3, %3\n\t"
                :
                : "r" (a), "r" (b), "r" (c), "r" (d)
                : "r0", "r1", "r2", "r3", "memory"
            );
        }
        
        /* More computations to keep variables live across loop iterations */
        a = a ^ b;
        b = b ^ c;
        c = c ^ d;
        d = d ^ e;
        e = e ^ f;
        f = f ^ g;
        g = g ^ h;
        h = h ^ i;
        i = i ^ j;
        j = j ^ k;
    }
    
    /* Final use to prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j + 
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Alternative function with different variable count to potentially
 * create different fixup graph sizes and trigger new_exit_index */
__attribute__((noinline))
static void test_ira_conflict_medium_pressure(void) {
    /* Different number of variables to potentially get different
     * fixup graph sizes and indices */
    volatile int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    volatile int v11, v12, v13, v14, v15;
    
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15;
    
    /* Loop with complex data flow */
    for (int i = 0; i < 1000; i++) {
        /* Chain of dependencies */
        v1 = v2 + i;
        v2 = v3 + v1;
        v3 = v4 + v2;
        v4 = v5 + v3;
        v5 = v6 + v4;
        v6 = v7 + v5;
        v7 = v8 + v6;
        v8 = v9 + v7;
        v9 = v10 + v8;
        v10 = v11 + v9;
        v11 = v12 + v10;
        v12 = v13 + v11;
        v13 = v14 + v12;
        v14 = v15 + v13;
        v15 = v1 + v14;
        
        /* Conditional that creates different control flow paths */
        if (i & 1) {
            /* Different computation on odd iterations */
            v1 = v1 * 2;
            v3 = v3 * 2;
            v5 = v5 * 2;
            v7 = v7 * 2;
            v9 = v9 * 2;
        } else {
            /* Different computation on even iterations */
            v2 = v2 / 2;
            v4 = v4 / 2;
            v6 = v6 / 2;
            v8 = v8 / 2;
            v10 = v10 / 2;
        }
        
        /* Another asm clobber to force register spills */
        asm volatile (
            "/* Clobber many registers */\n\t"
            :
            :
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "memory"
        );
    }
    
    /* Use all variables to keep them live */
    volatile int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15;
    (void)sum;
}

/* Function designed to create imbalance in register definitions/uses
 * which might trigger fixup edge creation */
__attribute__((noinline))
static void test_ira_imbalance(void) {
    /* Variables with more uses than definitions in some regions */
    volatile int x, y, z;
    
    x = 1;
    y = 2;
    z = 3;
    
    /* Many uses of x with few definitions */
    for (int i = 0; i < 100; i++) {
        /* x is used many times but only defined once per iteration */
        y = x + i;
        z = x + y;
        
        /* x is used again */
        if (y > z) {
            x = x + 1;  /* Only definition in loop */
        }
        
        /* More uses of x */
        y = x * 2;
        z = x / 2;
        
        /* Complex asm that references x multiple times */
        asm volatile (
            "add %0, %0, %1\n\t"
            "sub %2, %0, %2\n\t"
            : "+r" (y), "+r" (z)
            : "r" (x)
            : "cc"
        );
    }
    
    /* Final computation with all variables */
    volatile int result = x + y + z;
    (void)result;
}

/* Test function with switch statement to create complex control flow */
__attribute__((noinline))
static void test_ira_complex_cfg(void) {
    volatile int a = 0, b = 0, c = 0, d = 0, e = 0;
    
    for (int i = 0; i < 100; i++) {
        switch (i % 5) {
            case 0:
                a = i;
                b = a + 1;
                break;
            case 1:
                b = i * 2;
                c = b + a;
                break;
            case 2:
                c = i * 3;
                d = c + b;
                break;
            case 3:
                d = i * 4;
                e = d + c;
                break;
            case 4:
                e = i * 5;
                a = e + d;
                break;
        }
        
        /* All variables live here */
        asm volatile (
            "mov %0, %0\n\t"
            "mov %1, %1\n\t"
            "mov %2, %2\n\t"
            : "+r" (a), "+r" (b), "+r" (c)
        );
    }
    
    volatile int sum = a + b + c + d + e;
    (void)sum;
}

/* Main function that calls all test scenarios */
int main(void) {
    /* Call different test functions to explore various
     * register allocation scenarios */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_ira_conflict_high_pressure();
        test_ira_conflict_medium_pressure();
        test_ira_imbalance();
        test_ira_complex_cfg();
    }
    
    return 0;
}

/* Additional test targeting specific architecture constraints */
#ifdef __arm__
__attribute__((noinline))
static void test_ira_arm_constrained(void) {
    /* ARM has only 16 general purpose registers, so this should
     * create significant register pressure */
    register int r0 asm("r0");
    register int r1 asm("r1");
    register int r2 asm("r2");
    register int r3 asm("r3");
    register int r4 asm("r4");
    register int r5 asm("r5");
    register int r6 asm("r6");
    register int r7 asm("r7");
    register int r8 asm("r8");
    register int r9 asm("r9");
    register int r10 asm("r10");
    register int r11 asm("r11");
    register int r12 asm("r12");
    
    volatile int spill1, spill2, spill3, spill4, spill5;
    volatile int spill6, spill7, spill8, spill9, spill10;
    
    r0 = 1; r1 = 2; r2 = 3; r3 = 4; r4 = 5; r5 = 6;
    r6 = 7; r7 = 8; r8 = 9; r9 = 10; r10 = 11; r11 = 12; r12 = 13;
    
    spill1 = 14; spill2 = 15; spill3 = 16; spill4 = 17; spill5 = 18;
    spill6 = 19; spill7 = 20; spill8 = 21; spill9 = 22; spill10 = 23;
    
    /* Computation using all variables */
    for (int i = 0; i < 100; i++) {
        r0 = r1 + r2;
        r1 = r2 + r3;
        r2 = r3 + r4;
        r3 = r4 + r5;
        r4 = r5 + r6;
        r5 = r6 + r7;
        r6 = r7 + r8;
        r7 = r8 + r9;
        r8 = r9 + r10;
        r9 = r10 + r11;
        r10 = r11 + r12;
        r11 = r12 + spill1;
        r12 = spill1 + spill2;
        
        spill1 = spill2 + spill3;
        spill2 = spill3 + spill4;
        spill3 = spill4 + spill5;
        spill4 = spill5 + spill6;
        spill5 = spill6 + spill7;
        spill6 = spill7 + spill8;
        spill7 = spill8 + spill9;
        spill8 = spill9 + spill10;
        spill9 = spill10 + r0;
        spill10 = r0 + r1;
        
        /* Force register shuffling */
        asm volatile (
            "/* Empty asm to prevent optimization */\n\t"
            :
            :
            : "memory"
        );
    }
    
    volatile int total = r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 +
                       r10 + r11 + r12 + spill1 + spill2 + spill3 + spill4 +
                       spill5 + spill6 + spill7 + spill8 + spill9 + spill10;
    (void)total;
}
#endif
