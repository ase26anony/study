/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20, vs3 = 30;
volatile char vc1 = 'a', vc2 = 'b';

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int complex_remat_test(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    short sa, sb, sc, sd, se, sf, sg, sh, si, sj;
    char ca, cb, cc, cd, ce, cf, cg, ch, ci, cj;
    
    /* Use volatile inputs to create initial live values */
    a = x1 + v1;
    b = x2 * v2;
    c = x3 - v3;
    d = x4 ^ v4;
    e = x5 | v5;
    
    /* Create complex expression chain with many intermediates */
    f = (a + b) * (c - d);
    g = (e << 2) | (f >> 3);
    h = (b & c) ^ (d | e);
    i = (a * g) + (h - f);
    j = (i ^ 0x55AA55AA) + 42;
    
    /* Mode mixing: int -> short conversions */
    sa = (short)(a + b);
    sb = (short)(c - d);
    sc = (short)(e * 2);
    
    /* More arithmetic creating def-use chains */
    k = (j * 3) / 2;
    l = (k ^ j) & 0xFF;
    m = (l << 4) | (k >> 4);
    n = m + sa - sb;
    o = n * sc + 12345;
    
    /* Address calculation that might be rematerialized */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Use addresses in computations */
    p = (*ptr1 + *ptr2) * *ptr3;
    q = (p & 0xFFFF) | ((p >> 16) << 16);
    
    /* More intermediates */
    r = q + vs1 - vs2;
    s = r * vs3 / 100;
    t = (s ^ 0x12345678) + vc1 - vc2;
    
    /* Create control flow with switch to increase DF complexity */
    int switch_val = t & 0x7;
    int result = 0;
    
    switch (switch_val) {
        case 0:
            u = a + b + c;
            v = d * e * f;
            w = (u & v) | (sa << 8);
            result = w + g;
            break;
        case 1:
            u = b - c - d;
            v = e ^ f ^ g;
            w = (u | v) & (sb * 256);
            result = w - h;
            break;
        case 2:
            u = c * d * e;
            v = f + g + h;
            w = (u ^ v) + (sc / 2);
            result = w * i;
            break;
        case 3:
            u = d ^ e ^ f;
            v = g - h - i;
            w = (u & v) | (sa + sb);
            result = w / j;
            break;
        case 4:
            u = e | f | g;
            v = h ^ i ^ j;
            w = (u - v) * sc;
            result = w & k;
            break;
        case 5:
            u = f & g & h;
            v = i | j | k;
            w = (u + v) - sa;
            result = w ^ l;
            break;
        case 6:
            u = g - h - i;
            v = j & k & l;
            w = (u | v) + sb;
            result = w * m;
            break;
        default:
            u = h ^ i ^ j;
            v = k | l | m;
            w = (u & v) - sc;
            result = w / n;
            break;
    }
    
    /* More arithmetic after switch to keep values live */
    x = result + o + p;
    y = (x * q) / r;
    z = y ^ s ^ t;
    aa = z + u + v;
    ab = aa * w / 1000;
    
    /* Mode conversions: char -> short -> int */
    ca = (char)(ab & 0xFF);
    cb = (char)((ab >> 8) & 0xFF);
    cc = (char)((ab >> 16) & 0xFF);
    
    sd = (short)(ca + cb + cc);
    se = (short)(sd * 2);
    sf = (short)(se - 100);
    
    ac = (int)sd + (int)se + (int)sf;
    ad = ac * ca * cb;
    ae = ad ^ cc ^ 0xDEADBEEF;
    
    /* Use inline assembly to create complex dataflow */
    asm volatile (
        "addl %[val1], %[val2]\n\t"
        "imull %[val3], %[val4]\n\t"
        : [val2] "+r" (ae), [val4] "+r" (ad)
        : [val1] "r" (v1), [val3] "r" (v2)
        : "cc"
    );
    
    /* Final computations using all values */
    af = ae + ad + ac;
    ag = af * ab * aa;
    ah = ag ^ z ^ y;
    ai = ah + x + w;
    aj = ai * v * u;
    
    /* Prevent dead code elimination */
    if (__builtin_expect(v1 > 0, 1)) {
        return aj + t + s + r + q + p + o + n + m + l + k + j + i + h + g + f + e + d + c + b + a;
    } else {
        return 0;
    }
}

/* Another function with different patterns */
int second_remat_test(int base) {
    int sum = 0;
    
    /* Loop with many live values */
    for (int i = 0; i < 100; i++) {
        /* Many intermediate calculations */
        int t1 = base + i;
        int t2 = t1 * 2;
        int t3 = t2 - i;
        int t4 = t3 ^ 0x55;
        int t5 = t4 | base;
        int t6 = t5 & 0xFF;
        int t7 = t6 << 3;
        int t8 = t7 >> 1;
        int t9 = t8 + v1;
        int t10 = t9 * v2;
        
        /* Mix modes */
        short st1 = (short)t1;
        short st2 = (short)t2;
        short st3 = (short)(st1 + st2);
        int t11 = (int)st3 * t3;
        
        /* Address calculation that might be rematerialized */
        int *addr = &global_array[i & 0xFF];
        int t12 = *addr + t4;
        
        /* Complex expression */
        int t13 = (t5 + t6) * (t7 - t8);
        int t14 = (t9 ^ t10) & (t11 | t12);
        int t15 = t13 + t14 + t15;  /* Intentional self-use for complexity */
        
        sum += t15;
        
        /* Volatile access creates dataflow barrier */
        if (__builtin_expect(v3 > 0, 1)) {
            sum += v3;
        }
    }
    
    return sum;
}

/* Main function to drive everything */
int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    int result1 = complex_remat_test(100, 200, 300, 400, 500,
                                     600, 700, 800, 900, 1000);
    
    int result2 = second_remat_test(42);
    
    int final_result = result1 + result2;
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
