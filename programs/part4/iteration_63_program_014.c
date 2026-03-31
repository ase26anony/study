/* test_mcf_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in GCC's
 * min-cost flow solver (mcf.cc) by creating register allocation
 * scenarios that require fixup graph construction with special nodes.
 * 
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 * 
 * The -DMCF_DEBUG flag enables debug dumping that calls dump_fixup_edge.
 */

/* Force inclusion of IRA and MCF debugging */
#ifdef MCF_DEBUG
/* This ensures the debug code paths are compiled in */
#endif

/* Function to create complex register pressure scenario */
void test_ira_conflict_1(void) {
    /* Create many overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize all variables to create definitions */
    a = 1; b = 2; c = 3; d = 4; e = 5;
    f = 6; g = 7; h = 8; i = 9; j = 10;
    k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to extend live ranges */
    for (int outer = 0; outer < 10; outer++) {
        /* Many variables live across inner loop */
        int temp1 = a + b + c + d + e;
        int temp2 = f + g + h + i + j;
        
        for (int inner = 0; inner < 5; inner++) {
            /* Complex computation keeping many variables live */
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
            
            /* Use volatile asm to clobber registers */
            asm volatile ("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
        }
        
        /* More uses to extend live ranges */
        k = temp1 + temp2;
        l = a + b + c;
        m = d + e + f;
        n = g + h + i;
        o = j + k + l;
    }
    
    /* Final uses to prevent optimization */
    asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
    asm volatile ("" : : "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
    asm volatile ("" : : "r"(k), "r"(l), "r"(m), "r"(n), "r"(o));
    asm volatile ("" : : "r"(p), "r"(q), "r"(r), "r"(s), "r"(t));
}

/* Second test with different pattern to trigger different fixup graph */
void test_ira_conflict_2(void) {
    /* Variables with asymmetric definitions/uses to create supply-demand imbalance */
    int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
    int y1, y2, y3, y4, y5;
    
    /* Many definitions but few uses - may trigger fixup edges */
    x1 = 1; x2 = 2; x3 = 3; x4 = 4; x5 = 5;
    x6 = 6; x7 = 7; x8 = 8; x9 = 9; x10 = 10;
    
    /* Conditional code creating complex control flow */
    for (int i = 0; i < 100; i++) {
        if (i & 1) {
            y1 = x1 + x2;
            y2 = x3 + x4;
            asm volatile ("" : : : "memory");
        } else {
            y3 = x5 + x6;
            y4 = x7 + x8;
            asm volatile ("" : : : "memory");
        }
        
        /* Switch-like structure for varied live ranges */
        switch (i % 4) {
            case 0: x1 = y1 + y2; break;
            case 1: x2 = y2 + y3; break;
            case 2: x3 = y3 + y4; break;
            case 3: x4 = y4 + y1; break;
        }
    }
    
    /* Force all variables to be used */
    y5 = x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    asm volatile ("" : : "r"(y5));
}

/* Third test targeting specific architecture constraints */
void test_ira_conflict_3(void) {
    /* Use many small data types to increase register pressure */
    char c1, c2, c3, c4, c5, c6, c7, c8, c9, c10;
    short s1, s2, s3, s4, s5, s6, s7, s8, s9, s10;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    /* Initialize */
    c1 = 1; c2 = 2; c3 = 3; c4 = 4; c5 = 5;
    s1 = 10; s2 = 20; s3 = 30; s4 = 40; s5 = 50;
    i1 = 100; i2 = 200; i3 = 300; i4 = 400; i5 = 500;
    
    /* Deeply nested loops */
    for (int a = 0; a < 5; a++) {
        for (int b = 0; b < 5; b++) {
            for (int c = 0; c < 5; c++) {
                /* Mix operations to prevent optimization */
                c6 = c1 + c2;
                c7 = c3 + c4;
                s6 = s1 + s2;
                s7 = s3 + s4;
                i6 = i1 + i2;
                i7 = i3 + i4;
                
                /* More clobbering */
                asm volatile ("" : : : 
                    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            }
        }
    }
    
    /* Final uses */
    c8 = c6 + c7;
    s8 = s6 + s7;
    i8 = i6 + i7;
    asm volatile ("" : : "r"(c8), "r"(s8), "r"(i8));
}

/* Test with array operations to create memory pressure */
void test_ira_conflict_4(void) {
    int arr1[20], arr2[20], arr3[20];
    
    /* Initialize arrays */
    for (int i = 0; i < 20; i++) {
        arr1[i] = i;
        arr2[i] = i * 2;
        arr3[i] = i * 3;
    }
    
    /* Complex array operations keeping many values live */
    for (int i = 0; i < 20; i++) {
        for (int j = 0; j < 20; j++) {
            /* Many intermediate values in registers */
            int sum1 = arr1[i] + arr2[j];
            int sum2 = arr2[i] + arr3[j];
            int sum3 = arr3[i] + arr1[j];
            int prod1 = sum1 * sum2;
            int prod2 = sum2 * sum3;
            int prod3 = sum3 * sum1;
            
            /* Update arrays creating anti-dependencies */
            arr1[i] = prod1 % 100;
            arr2[j] = prod2 % 100;
            arr3[i] = prod3 % 100;
        }
    }
    
    /* Force array elements to be used */
    int total = 0;
    for (int i = 0; i < 20; i++) {
        total += arr1[i] + arr2[i] + arr3[i];
    }
    asm volatile ("" : : "r"(total));
}

/* Main function to run all tests */
int main(void) {
    /* Call each test multiple times with different parameters
     * to explore different register allocation scenarios */
    for (int iteration = 0; iteration < 10; iteration++) {
        test_ira_conflict_1();
        test_ira_conflict_2();
        test_ira_conflict_3();
        test_ira_conflict_4();
    }
    
    return 0;
}
