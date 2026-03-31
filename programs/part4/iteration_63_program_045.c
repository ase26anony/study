/* test_mcf_coverage.c
 * 
 * This test program creates a complex register allocation scenario
 * designed to trigger the min-cost flow solver's fixup graph construction
 * with NEW_EXIT and NEW_ENTRY nodes, covering the uncovered debug dump
 * lines in GCC's mcf.cc.
 *
 * Compile with: gcc -O2 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 * Or with: gcc -O3 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test.o
 */

#include <stdio.h>
#include <stdlib.h>

/* Force many overlapping live ranges with complex control flow */
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
    
    int result = 0;
    
    /* Nested loops to create complex liveness patterns */
    for (int outer = 0; outer < iterations; outer++) {
        /* Many variables live across the outer loop */
        a += outer;
        b += a;
        c += b;
        
        /* Inner loop with its own live variables */
        for (int inner = 0; inner < 10; inner++) {
            /* Force many variables to be live simultaneously */
            d = a + inner;
            e = b + inner;
            f = c + inner;
            g = d + e;
            h = f + g;
            i = h + inner;
            j = i + outer;
            
            /* Complex computation chain */
            k = j * 2;
            l = k / 3;
            m = l % 7;
            n = m + k;
            o = n - l;
            p = o * o;
            q = p >> 2;
            r = q << 1;
            s = r | 0xFF;
            t = s & 0x0F;
            
            /* More computations to increase register pressure */
            u = t + a + b + c;
            v = u * u;
            w = v - u;
            x = w / 2;
            y = x % 3;
            z = y | z;
            
            aa = z + aa;
            bb = aa * bb;
            cc = bb - cc;
            dd = cc + dd;
            ee = dd * ee;
            ff = ee / ff;
            gg = ff + gg;
            hh = gg * hh;
            ii = hh - ii;
            jj = ii + jj;
            
            result += a + b + c + d + e + f + g + h + i + j +
                     k + l + m + n + o + p + q + r + s + t +
                     u + v + w + x + y + z + aa + bb + cc +
                     dd + ee + ff + gg + hh + ii + jj;
        }
        
        /* Force clobbering of many registers with inline asm */
        /* This increases register pressure significantly */
        asm volatile (
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            "nop\n\t"
            : 
            : 
            : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        
        /* More computations after asm clobber */
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
        
        /* Conditional branches to create control flow complexity */
        if (outer % 3 == 0) {
            k = l + m;
            l = m + n;
            m = n + o;
        } else if (outer % 3 == 1) {
            n = o + p;
            o = p + q;
            p = q + r;
        } else {
            q = r + s;
            r = s + t;
            s = t + u;
        }
        
        /* Switch statement for more control flow edges */
        switch (outer % 5) {
            case 0:
                t = u + v;
                u = v + w;
                break;
            case 1:
                v = w + x;
                w = x + y;
                break;
            case 2:
                x = y + z;
                y = z + aa;
                break;
            case 3:
                z = aa + bb;
                aa = bb + cc;
                break;
            case 4:
                bb = cc + dd;
                cc = dd + ee;
                break;
        }
    }
    
    /* Final computation using all variables */
    result += a * 2 + b * 3 + c * 4 + d * 5 + e * 6 + f * 7 + g * 8 +
              h * 9 + i * 10 + j * 11 + k * 12 + l * 13 + m * 14 +
              n * 15 + o * 16 + p * 17 + q * 18 + r * 19 + s * 20 +
              t * 21 + u * 22 + v * 23 + w * 24 + x * 25 + y * 26 +
              z * 27 + aa * 28 + bb * 29 + cc * 30 + dd * 31 +
              ee * 32 + ff * 33 + gg * 34 + hh * 35 + ii * 36 + jj * 37;
    
    return result;
}

/* Second test function with different pattern */
int test_ira_conflict2(int seed) {
    /* Use array to create many pseudo-registers */
    int arr[50];
    int sum = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < 50; i++) {
        arr[i] = seed * i + i * i;
    }
    
    /* Complex access pattern with many live ranges */
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            if (i != j) {
                /* Force many array elements to be live */
                arr[i] = arr[i] + arr[j] * 3;
                arr[j] = arr[j] - arr[i] / 2;
                
                /* Additional computations */
                int tmp1 = arr[i] * arr[j];
                int tmp2 = arr[i] + arr[j];
                int tmp3 = arr[i] - arr[j];
                int tmp4 = arr[i] / (arr[j] + 1);
                int tmp5 = arr[j] % (arr[i] + 1);
                
                sum += tmp1 + tmp2 + tmp3 + tmp4 + tmp5;
            }
        }
        
        /* Clobber registers periodically */
        if (i % 10 == 0) {
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5"
            );
        }
    }
    
    return sum;
}

/* Third test: function with many parameters */
int test_many_params(int p1, int p2, int p3, int p4, int p5,
                     int p6, int p7, int p8, int p9, int p10,
                     int p11, int p12, int p13, int p14, int p15) {
    /* All parameters are live initially */
    int sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10 +
              p11 + p12 + p13 + p14 + p15;
    
    /* Create many temporary variables */
    int t1 = p1 * p2;
    int t2 = p3 * p4;
    int t3 = p5 * p6;
    int t4 = p7 * p8;
    int t5 = p9 * p10;
    int t6 = p11 * p12;
    int t7 = p13 * p14;
    int t8 = p15 * p1;
    
    /* Complex computation with all variables live */
    for (int i = 0; i < 100; i++) {
        t1 = t1 + p1 + i;
        t2 = t2 + p2 + i;
        t3 = t3 + p3 + i;
        t4 = t4 + p4 + i;
        t5 = t5 + p5 + i;
        t6 = t6 + p6 + i;
        t7 = t7 + p7 + i;
        t8 = t8 + p8 + i;
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
        
        /* Force spilling with large clobber list */
        if (i % 25 == 0) {
            asm volatile (
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                "nop\n\t"
                : 
                : 
                : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14"
            );
        }
    }
    
    return sum;
}

/* Main function to exercise all test cases */
int main() {
    int total = 0;
    
    printf("Starting IRA/MCF coverage test...\n");
    
    /* Call test functions with different parameters to explore
     * different register allocation scenarios */
    for (int run = 0; run < 10; run++) {
        printf("Run %d\n", run);
        
        /* Test 1: Many local variables with nested loops */
        total += test_ira_conflict(run + 1);
        
        /* Test 2: Array-based computation */
        total += test_ira_conflict2(run * 100);
        
        /* Test 3: Many parameters */
        total += test_many_params(
            run * 1, run * 2, run * 3, run * 4, run * 5,
            run * 6, run * 7, run * 8, run * 9, run * 10,
            run * 11, run * 12, run * 13, run * 14, run * 15
        );
    }
    
    printf("Total result: %d\n", total);
    printf("Test completed. Check compiler output for MCF debug info.\n");
    
    return 0;
}
