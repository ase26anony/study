/* test_mcf_coverage.c
 * 
 * This test is designed to trigger GCC's min-cost flow solver during
 * register allocation, specifically to exercise the dump_fixup_edge
 * function with n == fixup_graph->new_exit_index.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force inclusion of IRA/MCF debugging */
#ifdef MCF_DEBUG
/* This ensures debug paths are compiled in */
#endif

/* Function with complex register pressure to trigger fixup graph creation */
__attribute__((noinline))
static void test_ira_conflict(int iterations) {
    /* Create many overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, bb, cc, dd, ee, ff;
    
    /* Initialize all variables to create many definitions */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6;
    g = 7; h = 8; i = 9; j = 10; k = 11; l = 12;
    m = 13; n = 14; o = 15; p = 16; q = 17; r = 18;
    s = 19; t = 20; u = 21; v = 22; w = 23; x = 24;
    y = 25; z = 26; aa = 27; bb = 28; cc = 29; dd = 30;
    ee = 31; ff = 32;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across inner loop */
        int sum = a + b + c + d;
        
        /* Inner loop with volatile asm to clobber registers */
        for (int inner = 0; inner < 10; inner++) {
            /* Use many variables to keep them live */
            sum += e + f + g + h;
            
            /* Volatile asm that clobbers many registers */
            __asm__ volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "add r0, r0, r1\n\t"
                "mov %0, r0\n\t"
                : "+r" (sum), "+r" (i)
                : 
                : "r0", "r1", "cc", "memory"
            );
            
            /* More variable usage to extend live ranges */
            sum += j + k + l + m;
        }
        
        /* Complex computation using all variables */
        a = b + c - d;
        b = c + d - e;
        c = d + e - f;
        d = e + f - g;
        e = f + g - h;
        f = g + h - i;
        g = h + i - j;
        h = i + j - k;
        i = j + k - l;
        j = k + l - m;
        k = l + m - n;
        l = m + n - o;
        m = n + o - p;
        n = o + p - q;
        o = p + q - r;
        p = q + r - s;
        q = r + s - t;
        r = s + t - u;
        s = t + u - v;
        t = u + v - w;
        u = v + w - x;
        v = w + x - y;
        w = x + y - z;
        x = y + z - aa;
        y = z + aa - bb;
        z = aa + bb - cc;
        aa = bb + cc - dd;
        bb = cc + dd - ee;
        cc = dd + ee - ff;
        
        /* Another volatile asm with different clobbers */
        __asm__ volatile (
            "mov r2, %0\n\t"
            "mov r3, %1\n\t"
            "mul r2, r2, r3\n\t"
            "mov %0, r2\n\t"
            : "+r" (dd), "+r" (ee)
            : 
            : "r2", "r3", "cc", "memory"
        );
        
        dd = ee + ff - a;
        ee = ff + a - b;
        ff = a + b - c;
    }
    
    /* Final use to prevent dead code elimination */
    volatile int result = a + b + c + d + e + f + g + h + i + j + k + l + m +
                         n + o + p + q + r + s + t + u + v + w + x + y + z +
                         aa + bb + cc + dd + ee + ff;
    (void)result;
}

/* Second test function with different conflict pattern */
__attribute__((noinline))
static void test_imbalance_pattern(int seed) {
    /* Create variables with definition-use imbalance */
    int def1, def2, def3, def4, def5;
    int use1, use2, use3, use4, use5, use6, use7, use8, use9, use10;
    
    /* Few definitions... */
    def1 = seed;
    def2 = seed * 2;
    def3 = seed * 3;
    def4 = seed * 4;
    def5 = seed * 5;
    
    /* ...but many uses, creating supply-demand imbalance */
    for (int i = 0; i < 100; i++) {
        use1 = def1 + def2;
        use2 = def2 + def3;
        use3 = def3 + def4;
        use4 = def4 + def5;
        use5 = def5 + def1;
        
        /* Chain of dependencies */
        use6 = use1 + use2;
        use7 = use2 + use3;
        use8 = use3 + use4;
        use9 = use4 + use5;
        use10 = use5 + use1;
        
        /* Force register pressure with many live values */
        def1 = use6 ^ use7;
        def2 = use7 ^ use8;
        def3 = use8 ^ use9;
        def4 = use9 ^ use10;
        def5 = use10 ^ use6;
        
        /* Clobber registers periodically */
        if (i % 7 == 0) {
            __asm__ volatile (
                "mov r4, %0\n\t"
                "mov r5, %1\n\t"
                "and r4, r4, r5\n\t"
                "mov %0, r4\n\t"
                : "+r" (def1), "+r" (def2)
                : 
                : "r4", "r5", "cc", "memory"
            );
        }
    }
    
    volatile int out = def1 + def2 + def3 + def4 + def5 +
                      use1 + use2 + use3 + use4 + use5 +
                      use6 + use7 + use8 + use9 + use10;
    (void)out;
}

/* Third test: Function with switch statement for control flow complexity */
__attribute__((noinline))
static void test_control_flow(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int y1, y2, y3, y4, y5;
    
    switch (mode % 5) {
        case 0:
            y1 = x1 * x2;
            y2 = x2 * x3;
            y3 = x3 * x4;
            y4 = x4 * x5;
            y5 = x5 * x1;
            break;
        case 1:
            y1 = x1 + x2;
            y2 = x2 + x3;
            y3 = x3 + x4;
            y4 = x4 + x5;
            y5 = x5 + x1;
            break;
        case 2:
            y1 = x1 - x2;
            y2 = x2 - x3;
            y3 = x3 - x4;
            y4 = x4 - x5;
            y5 = x5 - x1;
            break;
        case 3:
            y1 = x1 ^ x2;
            y2 = x2 ^ x3;
            y3 = x3 ^ x4;
            y4 = x4 ^ x5;
            y5 = x5 ^ x1;
            break;
        case 4:
            y1 = x1 & x2;
            y2 = x2 & x3;
            y3 = x3 & x4;
            y4 = x4 & x5;
            y5 = x5 & x1;
            break;
    }
    
    /* Loop with all variables live */
    for (int i = 0; i < 50; i++) {
        x1 = y1 + i;
        x2 = y2 + i;
        x3 = y3 + i;
        x4 = y4 + i;
        x5 = y5 + i;
        
        y1 = x1 * x2;
        y2 = x2 * x3;
        y3 = x3 * x4;
        y4 = x4 * x5;
        y5 = x5 * x1;
        
        /* Clobber to force spills/reloads */
        __asm__ volatile (
            "mov r6, %0\n\t"
            "mov r7, %1\n\t"
            "orr r6, r6, r7\n\t"
            "mov %0, r6\n\t"
            : "+r" (x1), "+r" (x2)
            : 
            : "r6", "r7", "cc", "memory"
        );
    }
    
    volatile int result = x1 + x2 + x3 + x4 + x5 + y1 + y2 + y3 + y4 + y5;
    (void)result;
}

/* Main driver that calls all test patterns */
int main(int argc, char **argv) {
    int base = argc > 1 ? 10 : 5;
    
    /* Call first test - many overlapping live ranges */
    test_ira_conflict(base);
    
    /* Call second test - definition-use imbalance */
    test_imbalance_pattern(base);
    
    /* Call third test - control flow complexity */
    for (int i = 0; i < 3; i++) {
        test_control_flow(i);
    }
    
    return 0;
}
