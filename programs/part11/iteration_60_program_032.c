/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;
volatile int v5 = 5;

/* Global arrays to create address calculations */
int global_arr[256];
short global_short_arr[512];
float global_float_arr[128];

/* Complex function with high register pressure */
int complex_remat_test(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    short sa, sb, sc, sd, se, sf, sg, sh, si, sj;
    float fa, fb, fc, fd, fe, ff, fg, fh, fi, fj;
    
    /* Use volatile to prevent constant propagation */
    int base1 = x1 + v1;
    int base2 = x2 + v2;
    
    /* Long chain of arithmetic operations - creates many intermediate values */
    a = base1 + base2;
    b = a * x3 - v3;
    c = b ^ x4;
    d = c + (x5 << 2);
    e = d | x6;
    f = e - (x7 * 3);
    g = f & x8;
    h = g + (x9 / 2);
    i = h ^ x10;
    j = i * base1;
    k = j + (base2 << 1);
    l = k - (x3 >> 1);
    m = l | x4;
    n = m & x5;
    o = n + x6;
    p = o * x7;
    q = p - x8;
    r = q ^ x9;
    s = r + x10;
    t = s * base1;
    
    /* More intermediate values */
    u = t + (a << 1);
    v = u - (b >> 1);
    w = v | c;
    x = w & d;
    y = x ^ e;
    z = y + f;
    aa = z * g;
    ab = aa - h;
    ac = ab ^ i;
    ad = ac + j;
    ae = ad * k;
    af = ae - l;
    ag = af ^ m;
    ah = ag + n;
    ai = ah * o;
    aj = ai - p;
    
    /* Mixed-type operations to trigger mode changes */
    sa = (short)q;
    sb = (short)r;
    sc = sa + sb;
    sd = sc - (short)s;
    se = sd * (short)t;
    sf = se & (short)u;
    sg = sf | (short)v;
    sh = sg ^ (short)w;
    si = sh + (short)x;
    sj = si - (short)y;
    
    /* Floating point operations - different register class */
    fa = (float)a * 1.5f;
    fb = (float)b + 2.5f;
    fc = fa - fb;
    fd = (float)c * 3.5f;
    fe = fd / fc;
    ff = (float)d + 4.5f;
    fg = ff * fe;
    fh = (float)e - 5.5f;
    fi = fh / fg;
    fj = fi + (float)f;
    
    /* Complex loop with switch statement - creates complex CFG */
    int result = 0;
    for (int idx = 0; idx < 100; idx++) {
        /* Address calculation that could be rematerialized */
        int *addr1 = &global_arr[(idx + a) & 0xFF];
        short *addr2 = &global_short_arr[(idx + b) & 0x1FF];
        float *addr3 = &global_float_arr[(idx + c) & 0x7F];
        
        /* Use volatile in condition */
        if (__builtin_expect(v4 > 0, 1)) {
            /* Switch with multiple cases using different live values */
            switch (idx & 0x7) {
                case 0:
                    *addr1 = a + b + idx;
                    result += *addr1;
                    break;
                case 1:
                    *addr2 = (short)(c - d + idx);
                    result += *addr2;
                    break;
                case 2:
                    *addr3 = fa + fb + idx;
                    result += (int)*addr3;
                    break;
                case 3:
                    *addr1 = e ^ f ^ idx;
                    result += *addr1;
                    break;
                case 4:
                    *addr2 = (short)(g | h | idx);
                    result += *addr2;
                    break;
                case 5:
                    *addr3 = fc - fd + idx;
                    result += (int)*addr3;
                    break;
                case 6:
                    *addr1 = i & j & idx;
                    result += *addr1;
                    break;
                case 7:
                    *addr2 = (short)(k + l + idx);
                    result += *addr2;
                    break;
            }
        }
        
        /* More arithmetic to keep values live */
        a = a + (idx & 1);
        b = b - (idx & 2);
        c = c ^ (idx & 4);
        d = d | (idx & 8);
        e = e + (idx & 16);
        fa = fa + (idx & 1) * 0.1f;
        fb = fb - (idx & 2) * 0.1f;
        
        /* Inline assembly to create complex dataflow */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "subl %[val3], %[val4]\n\t"
            : [val2] "+r" (result), [val4] "+r" (idx)
            : [val1] "r" (a), [val3] "r" (b)
            : "cc"
        );
    }
    
    /* Use all computed values to prevent dead code elimination */
    result += sa + sb + sc + sd + se + sf + sg + sh + si + sj;
    result += (int)fa + (int)fb + (int)fc + (int)fd + (int)fe;
    result += (int)ff + (int)fg + (int)fh + (int)fi + (int)fj;
    result += u + v + w + x + y + z + aa + ab + ac + ad + ae + af + ag + ah + ai + aj;
    
    return result;
}

/* Another function with different patterns */
int nested_remat_test(int iter) {
    int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        /* Bit-field like operations */
        struct {
            unsigned int a : 4;
            unsigned int b : 8;
            unsigned int c : 12;
            unsigned int d : 8;
        } bits;
        
        bits.a = i & 0xF;
        bits.b = (i >> 4) & 0xFF;
        bits.c = (i >> 8) & 0xFFF;
        bits.d = (i >> 20) & 0xFF;
        
        /* Operations that might trigger sub-register modes */
        unsigned int val1 = bits.a * 3;
        unsigned int val2 = bits.b << bits.a;
        unsigned int val3 = bits.c >> bits.b;
        unsigned int val4 = bits.d | bits.a;
        
        /* Complex expression with many intermediates */
        int tmp1 = val1 + val2;
        int tmp2 = tmp1 - val3;
        int tmp3 = tmp2 * val4;
        int tmp4 = tmp3 ^ val1;
        int tmp5 = tmp4 + val2;
        int tmp6 = tmp5 - val3;
        int tmp7 = tmp6 * val4;
        int tmp8 = tmp7 ^ val1;
        int tmp9 = tmp8 + val2;
        int tmp10 = tmp9 - val3;
        
        /* Use volatile to force register usage */
        if (v5 > 0) {
            tmp10 += v1;
        }
        
        sum += tmp1 + tmp2 + tmp3 + tmp4 + tmp5 + tmp6 + tmp7 + tmp8 + tmp9 + tmp10;
        
        /* Vector-like operations using arrays */
        int vec[4] = {tmp1, tmp2, tmp3, tmp4};
        for (int j = 0; j < 4; j++) {
            vec[j] = vec[j] * 2 - 1;
        }
        sum += vec[0] + vec[1] + vec[2] + vec[3];
    }
    
    return sum;
}

/* Main function that creates maximum register pressure */
int main(void) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = i;
    }
    for (int i = 0; i < 512; i++) {
        global_short_arr[i] = (short)(i * 2);
    }
    for (int i = 0; i < 128; i++) {
        global_float_arr[i] = i * 1.5f;
    }
    
    /* Call complex function with many parameters */
    int result1 = complex_remat_test(
        100, 200, 300, 400, 500,
        600, 700, 800, 900, 1000
    );
    
    /* Call nested function */
    int result2 = nested_remat_test(50);
    
    /* Final result computation using all results */
    int final_result = result1 + result2 * 2;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
