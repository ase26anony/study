/* Test case for GCC early rematerialization pass - targeting lines 930-937 in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20;

/* Global array to create address calculations */
int global_arr[100];

/* Function with high register pressure and complex dataflow */
unsigned long test_remat(int p1, int p2, int p3, int p4, int p5,
                         short ps1, short ps2, char pc1) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af;
    short sa, sb, sc, sd, se, sf;
    long la, lb, lc;
    unsigned long result = 0;
    
    /* Initial values from parameters - create many live values */
    a = p1 + p2;           /* 1 */
    b = p3 - p4;           /* 2 */
    c = a * b;             /* 3 */
    d = c ^ p5;            /* 4 */
    e = d + v1;            /* 5 - volatile creates barrier */
    f = e >> 2;            /* 6 */
    g = f & 0xFF;          /* 7 */
    h = g * p1;            /* 8 */
    i = h + p2;            /* 9 */
    j = i - p3;            /* 10 */
    k = j | p4;            /* 11 */
    l = k ^ p5;            /* 12 */
    m = l * 13;            /* 13 - constant for remat */
    n = m + 42;            /* 14 - another constant */
    o = n - 17;            /* 15 */
    p = o / 3;             /* 16 */
    q = p % 7;             /* 17 */
    r = q << 1;            /* 18 */
    s = r >> 2;            /* 19 */
    t = s & 0x0F;          /* 20 */
    
    /* Mode mixing: int -> short conversions */
    sa = (short)a;         /* 21 */
    sb = (short)b;         /* 22 */
    sc = sa + sb;          /* 23 */
    sd = sc * vs1;         /* 24 - volatile short */
    se = sd - ps1;         /* 25 */
    sf = se & 0x7F;        /* 26 */
    
    /* More computations creating def-use chains */
    u = t + sf;            /* 27 */
    v = u * 2;             /* 28 */
    w = v - 1;             /* 29 */
    x = w ^ 0x55AA;        /* 30 */
    y = x + v2;            /* 31 - volatile */
    z = y * 3;             /* 32 */
    aa = z / 5;            /* 33 */
    ab = aa << 2;          /* 34 */
    ac = ab >> 1;          /* 35 */
    ad = ac & 0x3F;        /* 36 */
    ae = ad + pc1;         /* 37 */
    af = ae * 11;          /* 38 */
    
    /* Long type computations - different mode */
    la = (long)af * 1001L; /* 39 */
    lb = la + 123456L;     /* 40 */
    lc = lb - 98765L;      /* 41 */
    
    /* Complex loop with high register pressure */
    for (int iter = 0; iter < 100; iter++) {
        /* Address calculation - potential rematerialization candidate */
        int *addr1 = &global_arr[iter];
        int *addr2 = &global_arr[iter + 1];
        int *addr3 = &global_arr[iter + 2];
        
        /* Use many live values in the loop */
        int tmp1 = *addr1 + a + b + c;
        int tmp2 = *addr2 + d + e + f;
        int tmp3 = *addr3 + g + h + i;
        
        /* Switch statement creating complex control flow */
        switch (iter % 8) {
            case 0:
                tmp1 = tmp1 + j + k + l;
                tmp2 = tmp2 * m * n;
                break;
            case 1:
                tmp1 = tmp1 - o - p - q;
                tmp2 = tmp2 / (r + 1);
                break;
            case 2:
                tmp1 = tmp1 ^ s ^ t ^ u;
                tmp2 = tmp2 | v | w;
                break;
            case 3:
                tmp1 = tmp1 & x & y & z;
                tmp2 = tmp2 + aa + ab;
                break;
            case 4:
                tmp1 = tmp1 * ac * ad;
                tmp2 = tmp2 - ae - af;
                break;
            case 5:
                /* Mode mixing in switch cases */
                short stmp = (short)tmp1 + sa + sb + sc;
                tmp2 = tmp2 + stmp;
                break;
            case 6:
                tmp1 = tmp1 + (int)sd + (int)se + (int)sf;
                tmp2 = tmp2 * la;
                break;
            case 7:
                tmp1 = tmp1 + (int)(lc >> 16);
                tmp2 = tmp2 - (int)(lc & 0xFFFF);
                break;
        }
        
        /* Use volatile to prevent optimization and create barriers */
        if (__builtin_expect(v3 > 0, 1)) {
            tmp1 = tmp1 + v4;
        }
        
        /* More computations to extend live ranges */
        int final1 = tmp1 * 3 + tmp2 * 7;
        int final2 = final1 ^ tmp3;
        
        /* Inline assembly to create complex dataflow patterns */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "imull %[val3], %[val2]"
            : [val2] "+r" (final2)
            : [val1] "r" (final1), [val3] "r" (iter)
            : "cc"
        );
        
        result += final2;
        
        /* Update some values to create circular dependencies */
        a = (a + 1) & 0xFF;
        b = (b - 1) & 0xFF;
        c = (c * 2) & 0xFF;
        d = (d ^ iter) & 0xFF;
    }
    
    /* Final aggregation using all computed values */
    result += a + b + c + d + e + f + g + h + i + j;
    result += k + l + m + n + o + p + q + r + s + t;
    result += u + v + w + x + y + z + aa + ab + ac + ad;
    result += ae + af + (unsigned long)la + (unsigned long)lb + (unsigned long)lc;
    result += sa + sb + sc + sd + se + sf;
    
    return result;
}

/* Second function with different pattern to increase chances */
void another_high_pressure(int *arr, int n) {
    /* Bit-field like operations */
    struct packed {
        unsigned int a : 3;
        unsigned int b : 5;
        unsigned int c : 8;
        unsigned int d : 16;
    } pack;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression with many temporaries */
        int t1 = arr[i] * 3;
        int t2 = t1 + i;
        int t3 = t2 << 2;
        int t4 = t3 ^ 0xABCD;
        int t5 = t4 >> 1;
        int t6 = t5 & 0x7F;
        int t7 = t6 * 13;
        int t8 = t7 - 29;
        int t9 = t8 | 0xFF00;
        int t10 = t9 ^ t1;
        int t11 = t10 + t2;
        int t12 = t11 * t3;
        int t13 = t12 - t4;
        int t14 = t13 ^ t5;
        int t15 = t14 + t6;
        int t16 = t15 * t7;
        int t17 = t16 - t8;
        int t18 = t17 ^ t9;
        int t19 = t18 + t10;
        int t20 = t19 * t11;
        
        /* Mode conversions */
        short s1 = (short)t20;
        short s2 = (short)t19;
        short s3 = s1 + s2;
        int t21 = t20 + (int)s3;
        
        /* Bit-field operations causing sub-register accesses */
        pack.a = t21 & 0x7;
        pack.b = (t21 >> 3) & 0x1F;
        pack.c = (t21 >> 8) & 0xFF;
        pack.d = (t21 >> 16) & 0xFFFF;
        
        arr[i] = pack.a + pack.b * 8 + pack.c * 64 + pack.d * 16384;
        
        /* Volatile memory access as barrier */
        if (__builtin_expect(v5 > 0, 0)) {
            asm volatile ("" : : "r" (arr[i]) : "memory");
        }
    }
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i * 3 + 1;
    }
    
    /* Call with many parameters to increase register pressure */
    unsigned long res = test_remat(1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Process array with another high-pressure function */
    int local_arr[50];
    for (int i = 0; i < 50; i++) {
        local_arr[i] = i * 2 + res;
    }
    another_high_pressure(local_arr, 50);
    
    /* Aggregate results */
    unsigned long final = res;
    for (int i = 0; i < 50; i++) {
        final += local_arr[i];
    }
    
    printf("Result: %lu\n", final % 1000000);
    return 0;
}
