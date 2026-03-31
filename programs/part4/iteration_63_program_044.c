/* test_mcf_coverage.c
 * 
 * This test program creates a complex register allocation scenario
 * designed to trigger GCC's min-cost flow solver debug output,
 * specifically covering the dump_fixup_edge special node index cases.
 *
 * Compile with: gcc -O3 -fira-algorithm=priority -DMCF_DEBUG -c test_mcf_coverage.c -o test_mcf_coverage.o
 * Or with: gcc -O2 -funroll-loops -fira-algorithm=CB -DMCF_DEBUG -c test_mcf_coverage.c -o test_mcf_coverage.o
 */

/* Force MCF_DEBUG if not already defined via command line */
#ifndef MCF_DEBUG
#define MCF_DEBUG 1
#endif

#include <stdio.h>
#include <stdlib.h>

/* Complex function with many overlapping live ranges */
/* Using volatile to prevent optimization and increase register pressure */
static volatile int global_counter = 0;

/* Function designed to create maximum register pressure
 * and complex conflict graph for IRA's min-cost flow solver */
__attribute__((noinline))
static int test_ira_conflict(int iterations) {
    /* Declare many local variables with overlapping live ranges */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z;
    int aa, bb, cc, dd, ee, ff, gg, hh, ii, jj;
    
    /* Initialize with complex expressions to create data dependencies */
    a = iterations * 2;
    b = a + 1;
    c = b * 3;
    d = c - a;
    e = d / 2;
    f = e + b;
    g = f * c;
    h = g - d;
    i = h + e;
    j = i * f;
    k = j - g;
    l = k + h;
    m = l * i;
    n = m - j;
    o = n + k;
    p = o * l;
    q = p - m;
    r = q + n;
    s = r * o;
    t = s - p;
    u = t + q;
    v = u * r;
    w = v - s;
    x = w + t;
    y = x * u;
    z = y - v;
    aa = z + w;
    bb = aa * x;
    cc = bb - y;
    dd = cc + z;
    ee = dd * aa;
    ff = ee - bb;
    gg = ff + cc;
    hh = gg * dd;
    ii = hh - ee;
    jj = ii + ff;
    
    /* Nested loops with many live variables across iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* All variables are live here - maximum register pressure */
        int temp = a + b + c + d + e + f + g + h + i + j +
                   k + l + m + n + o + p + q + r + s + t +
                   u + v + w + x + y + z + aa + bb + cc + dd +
                   ee + ff + gg + hh + ii + jj;
        
        /* Inner loop with volatile asm to clobber registers */
        for (int inner = 0; inner < 3; inner++) {
            /* Inline asm that clobbers many registers */
            __asm__ volatile (
                "mov r0, %0\n\t"
                "mov r1, %1\n\t"
                "add r0, r0, r1\n\t"
                "mov %0, r0\n\t"
                : "+r" (temp), "+r" (a)
                :
                : "r0", "r1", "cc", "memory"
            );
            
            /* More complex operations keeping variables live */
            b += temp;
            c -= a;
            d *= b;
            e /= c + 1;
            f = (f + d) ^ e;
            g = g * f;
            h = h + g;
            i = i - h;
            j = j * i;
            k = k + j;
            l = l - k;
            m = m * l;
            n = n + m;
            o = o - n;
            p = p * o;
            q = q + p;
            r = r - q;
            s = s * r;
            t = t + s;
            u = u - t;
            v = v * u;
            w = w + v;
            x = x - w;
            y = y * x;
            z = z + y;
            aa = aa - z;
            bb = bb * aa;
            cc = cc + bb;
            dd = dd - cc;
            ee = ee * dd;
            ff = ff + ee;
            gg = gg - ff;
            hh = hh * gg;
            ii = ii + hh;
            jj = jj - ii;
        }
        
        /* Force spilling by using all variables in a complex expression */
        global_counter += a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t +
                         u + v + w + x + y + z + aa + bb + cc + dd +
                         ee + ff + gg + hh + ii + jj;
    }
    
    /* Return a complex expression using all variables */
    return (a + b + c + d + e + f + g + h + i + j +
            k + l + m + n + o + p + q + r + s + t +
            u + v + w + x + y + z + aa + bb + cc + dd +
            ee + ff + gg + hh + ii + jj) % 1000;
}

/* Second test function with different variable usage patterns */
__attribute__((noinline))
static int test_ira_conflict2(int seed) {
    /* Create many variables with short but overlapping live ranges */
    int v1 = seed;
    int v2 = v1 * 2;
    int v3 = v2 + v1;
    int v4 = v3 - v2;
    int v5 = v4 * v3;
    int v6 = v5 / (v4 + 1);
    int v7 = v6 ^ v5;
    int v8 = v7 + v6;
    int v9 = v8 - v7;
    int v10 = v9 * v8;
    int v11 = v10 + v9;
    int v12 = v11 - v10;
    int v13 = v12 * v11;
    int v14 = v13 + v12;
    int v15 = v14 - v13;
    int v16 = v15 * v14;
    int v17 = v16 + v15;
    int v18 = v17 - v16;
    int v19 = v18 * v17;
    int v20 = v19 + v18;
    
    /* Switch statement to create complex control flow */
    switch (seed % 5) {
        case 0:
            v1 += v20;
            v2 -= v19;
            break;
        case 1:
            v3 *= v18;
            v4 /= v17 + 1;
            break;
        case 2:
            v5 ^= v16;
            v6 |= v15;
            break;
        case 3:
            v7 &= v14;
            v8 <<= v13;
            break;
        case 4:
            v9 >>= v12;
            v10 = ~v11;
            break;
    }
    
    /* Multiple parallel computations */
    int sum1 = v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17 + v19;
    int sum2 = v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18 + v20;
    int prod = v1 * v2 * v3 * v4 * v5;
    
    /* More inline asm with many clobbered registers */
    __asm__ volatile (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "mov r2, %2\n\t"
        "add r0, r0, r1\n\t"
        "add r0, r0, r2\n\t"
        "mov %0, r0\n\t"
        : "+r" (sum1)
        : "r" (sum2), "r" (prod)
        : "r0", "r1", "r2", "cc"
    );
    
    return sum1 + sum2 + prod;
}

/* Third test: Function with array accesses and pointer arithmetic */
__attribute__((noinline))
static int test_ira_conflict3(int size) {
    int array[50];
    int *ptr = array;
    int result = 0;
    
    /* Initialize array with complex pattern */
    for (int i = 0; i < 50; i++) {
        array[i] = i * i - i + size;
    }
    
    /* Multiple pointer traversals with overlapping live ranges */
    for (int i = 0; i < 10; i++) {
        int *p1 = ptr + i;
        int *p2 = ptr + i * 2;
        int *p3 = ptr + i * 3;
        int *p4 = ptr + i * 4;
        int *p5 = ptr + i * 5;
        
        /* Complex pointer arithmetic keeping all pointers live */
        int val1 = *p1 + *p2;
        int val2 = *p3 - *p4;
        int val3 = *p5 * val1;
        int val4 = val2 / (val3 + 1);
        int val5 = val1 ^ val2;
        int val6 = val3 | val4;
        int val7 = val5 & val6;
        
        /* Force register pressure with many simultaneous computations */
        result += val1 + val2 + val3 + val4 + val5 + val6 + val7 +
                  (int)(p1 - ptr) + (int)(p2 - ptr) + (int)(p3 - ptr) +
                  (int)(p4 - ptr) + (int)(p5 - ptr);
    }
    
    return result;
}

/* Main function that exercises all test cases */
int main(int argc, char **argv) {
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int total = 0;
    
    printf("Testing IRA min-cost flow solver coverage...\n");
    
    /* Call test functions multiple times with different parameters
     * to explore different conflict graph configurations */
    for (int run = 0; run < 5; run++) {
        total += test_ira_conflict(iterations + run);
        total += test_ira_conflict2(run * 7 + 1);
        total += test_ira_conflict3(iterations + run * 2);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
