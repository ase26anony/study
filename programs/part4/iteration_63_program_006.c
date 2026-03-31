/* test_mcf_coverage.c
 * 
 * This program is designed to trigger GCC's min-cost flow solver
 * during register allocation, specifically to exercise the
 * dump_fixup_edge function with new_exit_index/new_entry_index cases.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force many overlapping live ranges to create complex conflict graph */
#define FORCE_REGISTER_PRESSURE

/* Function with many overlapping live variables to create register pressure */
int test_ira_conflict(int iterations) {
    /* Declare many integer variables that will have overlapping live ranges */
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
        int temp1 = a + b;
        int temp2 = c + d;
        
        for (int inner = 0; inner < 100; inner++) {
            /* Complex computation keeping many variables live */
            a = b + c + inner;
            b = c + d + outer;
            c = d + e + temp1;
            d = e + f + temp2;
            e = f + g + a;
            f = g + h + b;
            g = h + i + c;
            h = i + j + d;
            i = j + k + e;
            j = k + l + f;
            
            /* Use volatile asm to clobber registers */
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r14"
            );
            
            /* More computations to extend live ranges */
            k = l + m + g;
            l = m + n + h;
            m = n + o + i;
            n = o + p + j;
            o = p + q + k;
            p = q + r + l;
            q = r + s + m;
            r = s + t + n;
            s = t + a + o;
            t = a + b + p;
            
            /* Accumulate result to prevent optimization */
            result += a + b + c + d + e + f + g + h + i + j +
                     k + l + m + n + o + p + q + r + s + t;
        }
        
        /* Cross-iteration dependencies */
        a = result % 100;
        b = (result + 1) % 100;
        c = (result + 2) % 100;
    }
    
    /* Final computation using all variables */
    result = a + b + c + d + e + f + g + h + i + j +
            k + l + m + n + o + p + q + r + s + t;
    
    return result;
}

/* Second test function with different pattern to explore more graph configurations */
int test_ira_conflict2(int seed) {
    /* Use many local variables with complex data flow */
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
    
    /* Create a diamond-shaped control flow to increase complexity */
    if (seed % 2 == 0) {
        v1 = v2 + v3;
        v4 = v5 + v6;
        v7 = v8 + v9;
    } else {
        v1 = v3 + v4;
        v5 = v6 + v7;
        v8 = v9 + v10;
    }
    
    /* Loop with many live variables */
    for (int i = 0; i < 50; i++) {
        /* Rotate values through variables */
        int tmp = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = tmp;
        
        /* Force register clobbering */
        asm volatile (
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "add r0, r0, r1\n\t"
            : 
            : "r" (v1), "r" (v2)
            : "r0", "r1", "cc"
        );
    }
    
    /* Use all variables in final result */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15;
}

/* Third test: Function with switch statement for varied control flow */
int test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    
    switch (mode % 4) {
        case 0:
            x1 = x2 + x3;
            x4 = x5 + x6;
            x7 = x8 + x9;
            break;
        case 1:
            x2 = x3 + x4;
            x5 = x6 + x7;
            x8 = x9 + x10;
            break;
        case 2:
            x3 = x4 + x5;
            x6 = x7 + x8;
            x9 = x10 + x1;
            break;
        case 3:
            x4 = x5 + x6;
            x7 = x8 + x9;
            x10 = x1 + x2;
            break;
    }
    
    /* Small unrolled loop */
    for (int i = 0; i < 10; i++) {
        x1 = x1 + x2;
        x2 = x2 + x3;
        x3 = x3 + x4;
        x4 = x4 + x5;
        x5 = x5 + x6;
        x6 = x6 + x7;
        x7 = x7 + x8;
        x8 = x8 + x9;
        x9 = x9 + x10;
        x10 = x10 + x1;
    }
    
    return x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
}

/* Main function to call test cases with different parameters */
int main() {
    int total = 0;
    
    /* Call first test with different iteration counts */
    for (int i = 0; i < 10; i++) {
        total += test_ira_conflict(i + 1);
    }
    
    /* Call second test with different seeds */
    for (int i = 0; i < 20; i++) {
        total += test_ira_conflict2(i * 7);
    }
    
    /* Call third test with different modes */
    for (int i = 0; i < 15; i++) {
        total += test_ira_conflict3(i);
    }
    
    return total % 256; /* Return non-zero to prevent optimization */
}
