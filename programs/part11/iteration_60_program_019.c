/* Test case for GCC early rematerialization pass - targeting lines 930-937 in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile short v3 = 1000;
volatile float v4 = 3.14159f;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int complex_remat_test(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    short sa, sb, sc, sd, se, sf, sg, sh, si, sj;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    
    /* Use volatile to force loads and create dataflow edges */
    int base = v1;
    
    /* Long sequence of arithmetic operations - creates many intermediate values */
    a = x1 + x2 + base;          /* 1 */
    b = a * x3 - v2;             /* 2 */
    c = b ^ x4;                  /* 3 */
    d = c + (x5 << 2);           /* 4 */
    e = d - (x6 * 3);            /* 5 */
    f = e | x7;                  /* 6 */
    g = f & x8;                  /* 7 */
    h = g + (x9 / 2);            /* 8 */
    i = h * x10;                 /* 9 */
    j = i - a + b;               /* 10 */
    k = j * c / (d + 1);         /* 11 */
    l = k ^ e ^ f;               /* 12 */
    m = l + g - h;               /* 13 */
    n = m * i / j;               /* 14 */
    o = n | k & l;               /* 15 */
    p = o + m - n;               /* 16 */
    q = p * 2 + o / 3;           /* 17 */
    r = q ^ p;                   /* 18 */
    s = r + q - p;               /* 19 */
    t = s * 2 - r / 2;           /* 20 */
    u = t | s & r;               /* 21 */
    v = u + t - s;               /* 22 */
    w = v * u / t;               /* 23 */
    x = w ^ v;                   /* 24 */
    y = x + w - v;               /* 25 */
    z = y * 2 - x / 2;           /* 26 */
    aa = z | y & x;              /* 27 */
    ab = aa + z - y;             /* 28 */
    ac = ab * aa / z;            /* 29 */
    ad = ac ^ ab;                /* 30 */
    ae = ad + ac - ab;           /* 31 */
    af = ae * 2 - ad / 2;        /* 32 */
    ag = af | ae & ad;           /* 33 */
    ah = ag + af - ae;           /* 34 */
    ai = ah * ag / af;           /* 35 */
    aj = ai ^ ah;                /* 36 */
    
    /* Mode mixing - integer to short conversions */
    sa = (short)a;
    sb = (short)b;
    sc = (short)c;
    sd = (short)d;
    se = (short)e;
    sf = (short)f;
    sg = (short)g;
    sh = (short)h;
    si = (short)i;
    sj = (short)j;
    
    /* More operations with shorts (different mode) */
    sa = sa + sb - sc;
    sd = sd * se / (sf + 1);
    sg = sg | sh & si;
    sj = sj ^ sa ^ sd;
    
    /* Float operations (another different mode) */
    fa = (float)a * v4;
    fb = (float)b / v4;
    fc = fa + fb;
    fd = fc * fa - fb;
    fe = fd / fc;
    ff = fe * 2.0f;
    fg = ff - fe;
    fh = fg * fd;
    fi = fh / fg;
    fj = fi + fh - fg;
    
    /* Complex switch to create control flow with different live value subsets */
    int switch_val = aj & 0x7;  /* Use lowest 3 bits */
    
    switch (switch_val) {
        case 0:
            /* Use subset 1 */
            a = b + c - d;
            sa = (short)(e + f);
            fa = (float)g * v4;
            break;
        case 1:
            /* Use subset 2 */
            h = i * j / k;
            sb = (short)(l + m);
            fb = (float)n / v4;
            break;
        case 2:
            /* Use subset 3 */
            o = p ^ q ^ r;
            sc = (short)(s + t);
            fc = (float)u * v4;
            break;
        case 3:
            /* Use subset 4 */
            v = w * x / y;
            sd = (short)(z + aa);
            fd = (float)ab * v4;
            break;
        case 4:
            /* Use subset 5 */
            ac = ad ^ ae ^ af;
            se = (short)(ag + ah);
            fe = (float)ai * v4;
            break;
        case 5:
            /* Use subset 6 - mix modes */
            aj = (int)sa + (int)sb;
            sf = (short)fc;
            ff = (float)aj * v4;
            break;
        case 6:
            /* Use subset 7 - more mode mixing */
            a = (int)sa * (int)sb;
            sg = (short)fd;
            fg = (float)a / v4;
            break;
        case 7:
            /* Use subset 8 - all modes */
            b = c + d - (int)se;
            sh = (short)fe;
            fh = (float)b * v4;
            break;
    }
    
    /* Loop with address calculations - encourages rematerialization of addresses */
    int sum = 0;
    for (int idx = 0; idx < 100; idx++) {
        /* Compute address multiple times (potential rematerialization candidate) */
        int* addr1 = &global_array[(idx + a) & 0xFF];
        int* addr2 = &global_array[(idx + b) & 0xFF];
        int* addr3 = &global_array[(idx + c) & 0xFF];
        int* addr4 = &global_array[(idx + d) & 0xFF];
        
        /* Use addresses in independent operations */
        *addr1 = *addr1 + idx;
        *addr2 = *addr2 * idx;
        *addr3 = *addr3 ^ idx;
        *addr4 = *addr4 - idx;
        
        /* More computations using many live values */
        int tmp1 = a + idx;
        int tmp2 = b * idx;
        int tmp3 = c ^ idx;
        int tmp4 = d - idx;
        int tmp5 = e + tmp1;
        int tmp6 = f * tmp2;
        int tmp7 = g ^ tmp3;
        int tmp8 = h - tmp4;
        
        /* Mix with short mode */
        short stmp1 = (short)tmp1;
        short stmp2 = (short)tmp2;
        short stmp3 = stmp1 + stmp2 - (short)idx;
        
        /* Mix with float mode */
        float ftmp1 = (float)tmp5 * v4;
        float ftmp2 = (float)tmp6 / v4;
        float ftmp3 = ftmp1 + ftmp2 - (float)idx;
        
        /* Use volatile as barrier */
        if (__builtin_expect(v1 > 10000, 0)) {
            stmp3 = (short)v3;
            ftmp3 = v4;
        }
        
        /* Accumulate results */
        sum += tmp1 + tmp2 + tmp3 + tmp4 + 
               tmp5 + tmp6 + tmp7 + tmp8 +
               (int)stmp3 + (int)ftmp3;
    }
    
    /* Final aggregation using all computed values */
    int result = a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t +
                 u + v + w + x + y + z + aa + ab + ac + ad +
                 ae + af + ag + ah + ai + aj +
                 (int)sa + (int)sb + (int)sc + (int)sd +
                 (int)se + (int)sf + (int)sg + (int)sh +
                 (int)si + (int)sj +
                 (int)fa + (int)fb + (int)fc + (int)fd +
                 (int)fe + (int)ff + (int)fg + (int)fh +
                 (int)fi + (int)fj +
                 sum;
    
    return result;
}

/* Inline assembly to create complex dataflow patterns */
void asm_dataflow_barrier(int* p1, int* p2, short* p3, float* p4) {
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl %%eax, %0\n\t"
        "movw %2, %%cx\n\t"
        "addw %%cx, %3\n\t"
        : "+r" (*p1), "+r" (*p2), "+r" (*p3), "+r" (*p4)
        : 
        : "eax", "cx", "memory"
    );
}

/* Main function with multiple calls to increase optimization opportunities */
int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    int total = 0;
    
    /* Call multiple times with different arguments */
    for (int iter = 0; iter < 50; iter++) {
        int x1 = iter * 1;
        int x2 = iter * 2;
        int x3 = iter * 3;
        int x4 = iter * 4;
        int x5 = iter * 5;
        int x6 = iter * 6;
        int x7 = iter * 7;
        int x8 = iter * 8;
        int x9 = iter * 9;
        int x10 = iter * 10;
        
        /* Add some inline assembly to create dataflow complexity */
        asm_dataflow_barrier(&x1, &x2, (short*)&v3, (float*)&v4);
        
        int result = complex_remat_test(x1, x2, x3, x4, x5, x6, x7, x8, x9, x10);
        
        /* Use result in another computation to create longer live ranges */
        int processed = result;
        for (int i = 0; i < 10; i++) {
            processed = (processed * 1103515245 + 12345) & 0x7fffffff;
            processed = processed ^ (processed >> 16);
            processed = processed * 16807 % 2147483647;
        }
        
        total += processed;
        
        /* Another inline assembly barrier */
        __asm__ volatile (
            "addl %1, %0\n\t"
            : "+r" (total)
            : "r" (processed)
            : "cc"
        );
    }
    
    printf("Result: %d\n", total);
    return 0;
}
