/* test_mcf_coverage.c
 * 
 * This test is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the dump_fixup_edge
 * function with new_exit_index and new_entry_index values.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of MCF debugging code */
#ifdef MCF_DEBUG
/* This ensures the debug code paths are compiled in */
#endif

/* Function with complex register pressure to trigger fixup graph creation */
__attribute__((noinline))
static void test_ira_conflict(int iterations) {
    /* Create many variables with overlapping live ranges */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    
    /* Initialize with different values to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8;
    i = 9; j = 10; k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the inner loop */
        int temp1 = a + b;
        int temp2 = c + d;
        int temp3 = e + f;
        int temp4 = g + h;
        int temp5 = i + j;
        
        /* Inner loop with register pressure */
        for (int inner = 0; inner < 100; inner++) {
            /* Force all variables to be used and modified */
            a = b + temp1;
            b = c + temp2;
            c = d + temp3;
            d = e + temp4;
            e = f + temp5;
            f = g + a;
            g = h + b;
            h = i + c;
            i = j + d;
            j = k + e;
            
            /* Use more variables to increase pressure */
            k = l + f;
            l = m + g;
            m = n + h;
            n = o + i;
            o = p + j;
            p = q + k;
            q = r + l;
            r = s + m;
            s = t + n;
            t = a + o;
            
            /* Inline asm to clobber registers and increase pressure */
            __asm__ volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "add r0, r0, r1\n\t"
                "mov %0, r0\n\t"
                : "+r" (a), "+r" (b)
                :
                : "r0", "r1", "cc"
            );
        }
        
        /* Cross-usage to ensure overlapping live ranges */
        a = b + c + d;
        b = c + d + e;
        c = d + e + f;
        d = e + f + g;
        e = f + g + h;
        
        /* Another asm block clobbering many registers */
        __asm__ volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mov r3, %3\n\t"
            "add r0, r0, r1\n\t"
            "add r2, r2, r3\n\t"
            "add r0, r0, r2\n\t"
            "mov %0, r0\n\t"
            : "+r" (f), "+r" (g), "+r" (h), "+r" (i)
            :
            : "r0", "r1", "r2", "r3", "cc"
        );
    }
    
    /* Final use to prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t;
    (void)result;
}

/* Second test function with different pattern to explore more graph configurations */
__attribute__((noinline))
static void test_ira_conflict2(int seed) {
    /* Variables with arithmetic sequence to create different values */
    int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    int v5 = seed * 5;
    int v6 = seed * 6;
    int v7 = seed * 7;
    int v8 = seed * 8;
    int v9 = seed * 9;
    int v10 = seed * 10;
    int v11 = seed * 11;
    int v12 = seed * 12;
    int v13 = seed * 13;
    int v14 = seed * 14;
    int v15 = seed * 15;
    
    /* Complex dependency chain */
    for (int i = 0; i < 50; i++) {
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v10;
        v9 = v10 + v11;
        v10 = v11 + v12;
        v11 = v12 + v13;
        v12 = v13 + v14;
        v13 = v14 + v15;
        v14 = v15 + v1;
        v15 = v1 + v2;
        
        /* Conditional to create control flow complexity */
        if (i % 3 == 0) {
            v1 = v15 - v14;
            v2 = v14 - v13;
        } else if (i % 3 == 1) {
            v3 = v13 - v12;
            v4 = v12 - v11;
        } else {
            v5 = v11 - v10;
            v6 = v10 - v9;
        }
        
        /* More asm to increase register pressure */
        __asm__ volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            "mul r0, r0, r1\n\t"
            "add r0, r0, r2\n\t"
            "mov %0, r0\n\t"
            : "+r" (v7), "+r" (v8), "+r" (v9)
            :
            : "r0", "r1", "r2", "cc"
        );
    }
    
    /* Prevent optimization */
    volatile int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15;
    (void)sum;
}

/* Third test with switch statement for more control flow edges */
__attribute__((noinline))
static void test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int y1 = 6, y2 = 7, y3 = 8, y4 = 9, y5 = 10;
    int z1 = 11, z2 = 12, z3 = 13, z4 = 14, z5 = 15;
    
    switch (mode % 5) {
        case 0:
            x1 = y1 + z1;
            x2 = y2 + z2;
            x3 = y3 + z3;
            break;
        case 1:
            x4 = y4 + z4;
            x5 = y5 + z5;
            y1 = x1 + z1;
            break;
        case 2:
            y2 = x2 + z2;
            y3 = x3 + z3;
            y4 = x4 + z4;
            break;
        case 3:
            y5 = x5 + z5;
            z1 = x1 + y1;
            z2 = x2 + y2;
            break;
        case 4:
            z3 = x3 + y3;
            z4 = x4 + y4;
            z5 = x5 + y5;
            break;
    }
    
    /* Loop with all variables live */
    for (int i = 0; i < 20; i++) {
        x1 = x2 + x3;
        x2 = x3 + x4;
        x3 = x4 + x5;
        x4 = x5 + y1;
        x5 = y1 + y2;
        y1 = y2 + y3;
        y2 = y3 + y4;
        y3 = y4 + y5;
        y4 = y5 + z1;
        y5 = z1 + z2;
        z1 = z2 + z3;
        z2 = z3 + z4;
        z3 = z4 + z5;
        z4 = z5 + x1;
        z5 = x1 + x2;
    }
    
    /* Final asm with many clobbered registers */
    __asm__ volatile (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        "mov r3, %3\n\t"
        "mov r4, %4\n\t"
        "add r0, r0, r1\n\t"
        "add r2, r2, r3\n\t"
        "add r4, r4, r0\n\t"
        "add r4, r4, r2\n\t"
        "mov %0, r4\n\t"
        : "+r" (x1), "+r" (x2), "+r" (x3), "+r" (x4), "+r" (x5)
        :
        : "r0", "r1", "r2", "r3", "r4", "cc"
    );
    
    volatile int total = x1 + x2 + x3 + x4 + x5 + y1 + y2 + y3 + y4 + y5 +
                        z1 + z2 + z3 + z4 + z5;
    (void)total;
}

/* Main function that calls test functions with different parameters
 * to explore various conflict graph configurations */
int main(int argc, char **argv) {
    int base = (argc > 1) ? 10 : 5;
    
    /* Call first test multiple times */
    for (int i = 0; i < base; i++) {
        test_ira_conflict(5 + i);
    }
    
    /* Call second test */
    for (int i = 0; i < base; i++) {
        test_ira_conflict2(100 + i * 7);
    }
    
    /* Call third test */
    for (int i = 0; i < base; i++) {
        test_ira_conflict3(i);
    }
    
    return 0;
}
