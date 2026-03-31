/* test_mcf_coverage.c
 * 
 * This test is designed to trigger GCC's min-cost flow solver during
 * integrated register allocation (IRA) and cause the debug output
 * to print the "NEW_EXIT" label from dump_fixup_edge().
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c
 */

/* Force MCF_DEBUG to be defined if not already */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

/* Function with many overlapping live ranges to create complex conflict graph */
int test_ira_conflict(int iterations) {
    /* Declare many variables that will have overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int aa, bb, cc, dd, ee, ff, gg, hh, ii, jj;
    
    /* Initialize with different values to prevent optimization */
    a = iterations * 1;
    b = iterations * 2;
    c = iterations * 3;
    d = iterations * 4;
    e = iterations * 5;
    f = iterations * 6;
    g = iterations * 7;
    h = iterations * 8;
    i = iterations * 9;
    j = iterations * 10;
    k = iterations * 11;
    l = iterations * 12;
    m = iterations * 13;
    n = iterations * 14;
    o = iterations * 15;
    p = iterations * 16;
    q = iterations * 17;
    r = iterations * 18;
    s = iterations * 19;
    t = iterations * 20;
    u = iterations * 21;
    v = iterations * 22;
    w = iterations * 23;
    x = iterations * 24;
    y = iterations * 25;
    z = iterations * 26;
    aa = iterations * 27;
    bb = iterations * 28;
    cc = iterations * 29;
    dd = iterations * 30;
    ee = iterations * 31;
    ff = iterations * 32;
    gg = iterations * 33;
    hh = iterations * 34;
    ii = iterations * 35;
    jj = iterations * 36;
    
    /* Nested loops to create complex liveness patterns */
    int sum = 0;
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across inner loop */
        int temp1 = a + b + c + d;
        int temp2 = e + f + g + h;
        
        /* Volatile asm to clobber many registers and increase pressure */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        for (int inner = 0; inner < 10; inner++) {
            /* Complex computation keeping many variables live */
            int t1 = i + j + k + l;
            int t2 = m + n + o + p;
            int t3 = q + r + s + t;
            int t4 = u + v + w + x;
            int t5 = y + z + aa + bb;
            int t6 = cc + dd + ee + ff;
            int t7 = gg + hh + ii + jj;
            
            /* Force all variables to be used to prevent dead code elimination */
            sum += temp1 + temp2 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
            
            /* More volatile asm to create register pressure spikes */
            asm volatile (
                "add %0, %0, %1\n\t"
                "add %0, %0, %2\n\t"
                : "+r" (sum)
                : "r" (inner), "r" (outer)
                : "cc"
            );
            
            /* Rotate values to create data dependencies */
            a = b; b = c; c = d; d = e;
            e = f; f = g; g = h; h = i;
            i = j; j = k; k = l; l = m;
            m = n; n = o; o = p; p = q;
            q = r; r = s; s = t; t = u;
            u = v; v = w; w = x; x = y;
            y = z; z = aa; aa = bb; bb = cc;
            cc = dd; dd = ee; ee = ff; ff = gg;
            gg = hh; hh = ii; ii = jj; jj = sum & 0xFF;
        }
        
        /* Conditional branches to create control flow complexity */
        if (outer % 3 == 0) {
            sum += a * b * c;
            asm volatile ("nop\n\tnop\n\t" : : : "memory");
        } else if (outer % 3 == 1) {
            sum += d * e * f;
            asm volatile ("nop\n\tnop\n\tnop\n\t" : : : "memory");
        } else {
            sum += g * h * i;
            asm volatile ("nop\n\t" : : : "memory");
        }
    }
    
    /* Final computation using all variables */
    int result = 
        a + b + c + d + e + f + g + h + i + j +
        k + l + m + n + o + p + q + r + s + t +
        u + v + w + x + y + z + aa + bb + cc + dd +
        ee + ff + gg + hh + ii + jj + sum;
    
    return result;
}

/* Second test function with different pattern to explore more graph configurations */
int test_ira_conflict2(int seed) {
    /* Use array to create many pseudo-registers */
    int arr[40];
    for (int i = 0; i < 40; i++) {
        arr[i] = seed * i + i * i;
    }
    
    /* Complex loop with many live values */
    int total = 0;
    for (int i = 0; i < 20; i++) {
        /* Keep many array elements live */
        int v1 = arr[i];
        int v2 = arr[i+1];
        int v3 = arr[i+2];
        int v4 = arr[i+3];
        int v5 = arr[i+4];
        int v6 = arr[i+5];
        int v7 = arr[i+6];
        int v8 = arr[i+7];
        int v9 = arr[i+8];
        int v10 = arr[i+9];
        
        /* Nested loop with all variables live */
        for (int j = 0; j < 5; j++) {
            total += v1 * v2 + v3 * v4 - v5 * v6 + v7 * v8 - v9 * v10;
            
            /* Clobber registers */
            asm volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "mul r2, r0, r1\n\t"
                : : "r" (v1), "r" (v2)
                : "r0", "r1", "r2", "cc"
            );
            
            /* Rotate values */
            int tmp = v1;
            v1 = v2; v2 = v3; v3 = v4; v4 = v5;
            v5 = v6; v6 = v7; v7 = v8; v8 = v9;
            v9 = v10; v10 = tmp;
        }
        
        /* Update array to prevent optimization */
        arr[i] = total & 0xFF;
    }
    
    return total;
}

/* Third test: Function with switch statement for complex CFG */
int test_ira_conflict3(int mode) {
    int x1 = 1, x2 = 2, x3 = 3, x4 = 4, x5 = 5;
    int x6 = 6, x7 = 7, x8 = 8, x9 = 9, x10 = 10;
    int result = 0;
    
    switch (mode % 5) {
        case 0:
            result = x1 * x2 + x3 * x4;
            asm volatile ("nop\n\tnop\n\t" : : : "memory", "r0", "r1");
            break;
        case 1:
            result = x5 * x6 - x7 * x8;
            asm volatile ("nop\n\t" : : : "memory", "r2", "r3");
            break;
        case 2:
            result = x9 * x10 + x1 * x3;
            asm volatile ("nop\n\tnop\n\tnop\n\t" : : : "memory", "r4", "r5");
            break;
        case 3:
            result = x2 * x4 - x6 * x8;
            asm volatile ("nop\n\t" : : : "memory", "r6", "r7");
            break;
        case 4:
            result = x10 * x5 + x7 * x9;
            asm volatile ("nop\n\tnop\n\t" : : : "memory", "r8", "r9");
            break;
    }
    
    /* Force all variables to be used */
    result += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
    
    return result;
}

/* Main function to call test cases with different parameters */
int main() {
    int total = 0;
    
    /* Call first test with different iteration counts */
    for (int i = 0; i < 5; i++) {
        total += test_ira_conflict(3 + i);
    }
    
    /* Call second test */
    for (int i = 0; i < 10; i++) {
        total += test_ira_conflict2(i * 7);
    }
    
    /* Call third test */
    for (int i = 0; i < 15; i++) {
        total += test_ira_conflict3(i);
    }
    
    return total & 0xFF; /* Return small value to avoid overflow */
}
