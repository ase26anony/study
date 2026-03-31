/* Test case for early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20, vs3 = 30;
volatile char vc1 = 100, vc2 = 101;

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
    
    /* Force initial values into registers */
    a = x1 + v1; b = x2 + v2; c = x3 + v3; d = x4 + v4; e = x5 + v5;
    f = x6 - v1; g = x7 - v2; h = x8 - v3; i = x9 - v4; j = x10 - v5;
    
    /* Long chain of arithmetic operations creating many intermediate values */
    k = (a * b) + (c ^ d) - (e | f);
    l = (g & h) * (i ^ j) + (a << 2);
    m = (b >> 1) + (c * d) - (e & f);
    n = (g | h) ^ (i * j) + (k - l);
    o = (m * n) / (a + 1) + (b ^ c);
    p = (d & e) | (f ^ g) + (h * i);
    q = (j << 3) - (k >> 2) + (l & m);
    r = (n | o) ^ (p & q) + (r ^ s);
    s = (t * u) + (v ^ w) - (x | y);
    t = (z & aa) * (ab ^ ac) + (ad << 2);
    
    /* Mix in volatile reads to create dataflow complexity */
    u = v1 + (a * v2) - (b / v3);
    v = v4 ^ (c & v5) | (d * v1);
    w = v2 - (e ^ v3) + (f & v4);
    x = v5 * (g | v1) ^ (h / v2);
    y = v3 + (i & v4) - (j ^ v5);
    z = v1 ^ (k | v2) + (l & v3);
    
    /* Create many distinct intermediate values in a loop */
    int sum = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Complex address calculation - potential rematerialization candidate */
        int *addr1 = &global_array[(a + iter) & 0xFF];
        int *addr2 = &global_array[(b + iter * 2) & 0xFF];
        int *addr3 = &global_array[(c + iter * 3) & 0xFF];
        
        /* Use addresses in computations - may force register creation */
        aa = *addr1 + (d * iter);
        ab = *addr2 - (e ^ iter);
        ac = *addr3 | (f & iter);
        
        /* More arithmetic creating register pressure */
        ad = (aa * ab) + (ac ^ iter);
        ae = (ad << 1) - (iter >> 2);
        af = (ae & 0xFF) | (iter << 8);
        ag = (af ^ aa) + (ab * ac);
        ah = (ag / (iter + 1)) | (ad & ae);
        ai = (ah ^ af) - (ag * iter);
        aj = (ai & 0xFFFF) + (iter ^ 0xAA);
        
        /* Mixed-type operations causing mode conversions */
        sa = (short)(aa & 0xFFFF);
        sb = (short)(ab >> 8);
        sc = (short)(ac & 0xFF);
        sd = sa + sb - sc;
        se = (short)(sd * iter);
        sf = (short)(se ^ sa);
        sg = (short)(sf & sb);
        sh = (short)(sg | sc);
        si = (short)(sh << 2);
        sj = (short)(si >> 1);
        
        /* Character operations with different mode */
        ca = (char)(aa & 0xFF);
        cb = (char)(ab & 0xFF);
        cc = (char)(ac & 0xFF);
        cd = ca + cb - cc;
        ce = (char)(cd * (iter & 0xFF));
        cf = (char)(ce ^ ca);
        cg = (char)(cf & cb);
        ch = (char)(cg | cc);
        ci = (char)(ch << 1);
        cj = (char)(ci >> 2);
        
        /* Switch statement creating complex control flow */
        switch (iter & 0x7) {
            case 0:
                sum += aa + sa + ca;
                break;
            case 1:
                sum += ab + sb + cb;
                break;
            case 2:
                sum += ac + sc + cc;
                break;
            case 3:
                sum += ad + sd + cd;
                break;
            case 4:
                sum += ae + se + ce;
                break;
            case 5:
                sum += af + sf + cf;
                break;
            case 6:
                sum += ag + sg + cg;
                break;
            case 7:
                sum += ah + sh + ch;
                break;
        }
        
        /* Use __builtin_expect to influence branch prediction */
        if (__builtin_expect((iter & 0xF) == 0, 0)) {
            /* This block merges many live values */
            sum += ai + si + ci;
            sum += aj + sj + cj;
        }
        
        /* Inline assembly to create complex dataflow patterns */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            "xorl %3, %0"
            : "+r" (sum)
            : "r" (aa), "r" (ab), "r" (ac)
            : "cc"
        );
        
        /* Volatile memory access as dataflow barrier */
        if (v1) {
            sum += v2;
        }
    }
    
    /* Final aggregation using all computed values */
    int result = sum + a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t +
                 u + v + w + x + y + z + aa + ab + ac + ad +
                 ae + af + ag + ah + ai + aj +
                 sa + sb + sc + sd + se + sf + sg + sh + si + sj +
                 ca + cb + cc + cd + ce + cf + cg + ch + ci + cj;
    
    return result;
}

/* Another function with different patterns */
int secondary_test(int base) {
    /* Bit-field structure to trigger sub-register accesses */
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
        unsigned int d : 8;
    } bits;
    
    bits.a = base & 0xF;
    bits.b = (base >> 4) & 0xFF;
    bits.c = (base >> 12) & 0xFFF;
    bits.d = (base >> 24) & 0xFF;
    
    /* Operations on bit-fields cause mode changes */
    int x = bits.a * 3;
    int y = bits.b / 5;
    int z = bits.c ^ 0xABC;
    int w = bits.d | 0x42;
    
    /* Vector-like operations using arrays */
    int arr[4] = {x, y, z, w};
    int vec_sum = 0;
    
    for (int i = 0; i < 4; i++) {
        vec_sum += arr[i] * (i + 1);
        vec_sum ^= arr[(i + 1) & 3];
    }
    
    return vec_sum;
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Call with many arguments to increase register pressure */
    int result1 = complex_remat_test(
        1000, 2000, 3000, 4000, 5000,
        6000, 7000, 8000, 9000, 10000
    );
    
    int result2 = secondary_test(result1);
    
    /* Use results to prevent dead code elimination */
    printf("Result1: %d\n", result1);
    printf("Result2: %d\n", result2);
    printf("Final: %d\n", result1 + result2);
    
    return 0;
}
