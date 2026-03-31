/* test_ira_mcf.c - Program to trigger GCC's min-cost flow solver debug output */
/* Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_ira_mcf.c -o test.o */

/* Force inclusion of mcf debugging by defining MCF_DEBUG if not already defined */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function designed to create complex register pressure scenario */
/* This creates many overlapping live ranges to force IRA to build a complex conflict graph */
int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    volatile int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int result = 0;
    
    /* Initialize all variables with different values to prevent optimization */
    a = 1; b = 2; c = 3; d = 4; e = 5; f = 6; g = 7; h = 8; 
    i = 9; j = 10; k = 11; l = 12; m = 13; n = 14; o = 15;
    p = 16; q = 17; r = 18; s = 19; t = 20;
    
    /* Nested loops to create complex control flow and overlapping live ranges */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the loop */
        a += outer;
        b += a;
        
        /* Inner loop with more live variables */
        for (int inner = 0; inner < 10; inner++) {
            /* Force all variables to be used in computation */
            c = a + b + inner;
            d = b + c + outer;
            e = c + d;
            f = d + e;
            g = e + f;
            h = f + g;
            i = g + h;
            j = h + i;
            
            /* Use volatile asm to clobber registers and increase pressure */
            asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e), 
                          "r"(f), "r"(g), "r"(h), "r"(i), "r"(j));
            
            /* More computations keeping variables live */
            k = i + j;
            l = j + k;
            m = k + l;
            n = l + m;
            o = m + n;
            p = n + o;
            q = o + p;
            r = p + q;
            s = q + r;
            t = r + s;
            
            /* Another asm statement clobbering different registers */
            asm volatile ("" : : "r"(k), "r"(l), "r"(m), "r"(n), "r"(o),
                          "r"(p), "r"(q), "r"(r), "r"(s), "r"(t));
            
            /* Cross-mix computations to create more overlapping live ranges */
            a = t + s;
            b = s + r;
            c = r + q;
        }
        
        /* Use all variables in result computation to keep them live */
        result += a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t;
    }
    
    /* Final computation using all variables */
    result = a * b + c * d + e * f + g * h + i * j +
            k * l + m * n + o * p + q * r + s * t;
    
    return result;
}

/* Second test function with different variable usage pattern */
/* This creates a different conflict graph structure */
int test_ira_conflict2(int seed) {
    /* Another set of variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    
    /* Initialize with seed-dependent values */
    v1 = seed; v2 = seed + 1; v3 = seed + 2; v4 = seed + 3; v5 = seed + 4;
    v6 = seed + 5; v7 = seed + 6; v8 = seed + 7; v9 = seed + 8; v10 = seed + 9;
    
    w1 = seed * 2; w2 = seed * 3; w3 = seed * 4; w4 = seed * 5; w5 = seed * 6;
    w6 = seed * 7; w7 = seed * 8; w8 = seed * 9; w9 = seed * 10; w10 = seed * 11;
    
    /* Complex conditional flow to create different basic blocks */
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            /* Path A: Use first set heavily */
            v1 = v2 + v3;
            v2 = v3 + v4;
            v3 = v4 + v5;
            v4 = v5 + v6;
            v5 = v6 + v7;
            
            /* Clobber registers */
            asm volatile ("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
        } else if (i % 3 == 1) {
            /* Path B: Use second set heavily */
            w1 = w2 + w3;
            w2 = w3 + w4;
            w3 = w4 + w5;
            w4 = w5 + w6;
            w5 = w6 + w7;
            
            /* Clobber different registers */
            asm volatile ("" : : "r"(w1), "r"(w2), "r"(w3), "r"(w4), "r"(w5));
        } else {
            /* Path C: Mix both sets */
            v6 = w6 + v1;
            v7 = w7 + v2;
            v8 = w8 + v3;
            v9 = w9 + v4;
            v10 = w10 + v5;
            
            /* Clobber many registers */
            asm volatile ("" : : "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                          "r"(w6), "r"(w7), "r"(w8), "r"(w9), "r"(w10));
        }
        
        /* Force all variables to be live periodically */
        if (i % 10 == 0) {
            v1 = v1 + w1;
            v2 = v2 + w2;
            v3 = v3 + w3;
            v4 = v4 + w4;
            v5 = v5 + w5;
            v6 = v6 + w6;
            v7 = v7 + w7;
            v8 = v8 + w8;
            v9 = v9 + w9;
            v10 = v10 + w10;
        }
    }
    
    /* Final sum using all variables */
    return v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10;
}

/* Third test: Function with switch statement for more control flow complexity */
int test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int y1 = 6, y2 = 7, y3 = 8, y4 = 9, y5 = 10;
    int z1 = 11, z2 = 12, z3 = 13, z4 = 14, z5 = 15;
    
    /* Switch creates multiple basic blocks with different live ranges */
    switch (mode % 4) {
        case 0:
            x1 = y1 + z1;
            x2 = y2 + z2;
            x3 = y3 + z3;
            asm volatile ("" : : "r"(x1), "r"(x2), "r"(x3));
            break;
        case 1:
            x4 = y4 + z4;
            x5 = y5 + z5;
            y1 = x1 + z1;
            asm volatile ("" : : "r"(x4), "r"(x5), "r"(y1));
            break;
        case 2:
            y2 = x2 + z2;
            y3 = x3 + z3;
            y4 = x4 + z4;
            asm volatile ("" : : "r"(y2), "r"(y3), "r"(y4));
            break;
        case 3:
            z1 = x1 + y1;
            z2 = x2 + y2;
            z3 = x3 + y3;
            z4 = x4 + y4;
            z5 = x5 + y5;
            asm volatile ("" : : "r"(z1), "r"(z2), "r"(z3), "r"(z4), "r"(z5));
            break;
    }
    
    /* Loop that uses all variables */
    for (int i = 0; i < 50; i++) {
        x1 = x2 + i;
        x2 = x3 + x1;
        x3 = x4 + x2;
        x4 = x5 + x3;
        x5 = y1 + x4;
        
        y1 = y2 + x5;
        y2 = y3 + y1;
        y3 = y4 + y2;
        y4 = y5 + y3;
        y5 = z1 + y4;
        
        z1 = z2 + y5;
        z2 = z3 + z1;
        z3 = z4 + z2;
        z4 = z5 + z3;
        z5 = x1 + z4;
        
        /* Periodic asm to clobber registers */
        if (i % 5 == 0) {
            asm volatile ("" : : "r"(x1), "r"(x2), "r"(x3), "r"(x4), "r"(x5),
                          "r"(y1), "r"(y2), "r"(y3), "r"(y4), "r"(y5),
                          "r"(z1), "r"(z2), "r"(z3), "r"(z4), "r"(z5));
        }
    }
    
    return x1 + x2 + x3 + x4 + x5 + y1 + y2 + y3 + y4 + y5 + z1 + z2 + z3 + z4 + z5;
}

/* Main function to call test functions with different parameters */
int main() {
    int total = 0;
    
    /* Call first test function multiple times with different iteration counts */
    total += test_ira_conflict(5);   /* Smaller graph */
    total += test_ira_conflict(20);  /* Larger graph */
    
    /* Call second test function */
    total += test_ira_conflict2(1);
    total += test_ira_conflict2(42);
    
    /* Call third test function with different modes */
    total += test_ira_conflict3(0);
    total += test_ira_conflict3(1);
    total += test_ira_conflict3(2);
    total += test_ira_conflict3(3);
    
    return total % 256; /* Return non-zero to prevent optimization */
}
