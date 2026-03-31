/* Test program to trigger early rematerialization with virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 0xABCDEF;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int complex_remat_test(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    int result = 0;
    
    /* Initial computations creating many live values */
    a = x1 + x2 + v1;           /* volatile creates barrier */
    b = x3 * x4 - a;
    c = (x5 ^ x6) | b;
    d = c << (x7 & 3);
    e = d - x8 + v2;            /* another volatile barrier */
    f = e * x9 / (x10 + 1);
    g = f & 0xFF;
    h = g | 0x80;
    i = h ^ x1;
    j = i * x2;
    k = j - x3;
    l = k + x4;
    m = l * x5;
    n = m / (x6 + 1);
    o = n ^ x7;
    p = o | x8;
    q = p & x9;
    r = q + x10;
    s = r * a;
    t = s - b;
    u = t ^ c;
    v = u | d;
    w = v & e;
    y = w + f;
    z = y - g;
    aa = z * h;
    ab = aa / (i + 1);
    ac = ab ^ j;
    ad = ac | k;
    ae = ad & l;
    af = ae + m;
    ag = af - n;
    ah = ag * o;
    ai = ah / (p + 1);
    aj = ai ^ q;
    
    /* Loop with complex control flow to create dataflow graphs */
    for (int iter = 0; iter < 100; iter++) {
        /* Mix in mode conversions (int -> short -> int) */
        short sa = (short)(a + iter);
        short sb = (short)(b - iter);
        short sc = (short)(c ^ iter);
        short sd = (short)(d | iter);
        
        /* Use these in calculations with different modes */
        int ta = (int)sa * 2;
        int tb = (int)sb / 2;
        int tc = (int)sc + 100;
        int td = (int)sd - 50;
        
        /* Complex switch with different live value usage */
        int selector = (ta + tb + tc + td) & 0xF;
        
        switch (selector) {
            case 0:
                /* Use subset 1 with address calculation */
                result += global_array[ta & 0xFF] + tb;
                break;
            case 1:
                /* Use subset 2 with mode mixing */
                result += (short)(tc * td) + ta;
                break;
            case 2:
                /* Use subset 3 with volatile */
                result += v3 + td;
                break;
            case 3:
                /* Use subset 4 with bitfield-like ops */
                result += (ta & 0xF0F0) | (tb & 0x0F0F);
                break;
            case 4:
                /* Use subset 5 with shifts */
                result += (tc << 4) + (td >> 4);
                break;
            case 5:
                /* Use subset 6 with arithmetic */
                result += ta * 3 + tb * 5 + tc * 7;
                break;
            case 6:
                /* Use subset 7 with conditional */
                if (v1 > 10000) result += td;
                break;
            case 7:
                /* Use subset 8 with all values */
                result += ta + tb + tc + td + sa + sb + sc + sd;
                break;
            default:
                /* Use different subset */
                result += (ta ^ tb) | (tc & td);
                break;
        }
        
        /* Update some values to create new def-use chains */
        a = (a + tb) & 0x7FFF;
        b = (b - tc) | 0x8000;
        c = (c ^ td) + ta;
        d = (d + sa) * 2;
        
        /* Inline asm to create complex dataflow edges */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "subl %[val3], %[val4]"
            : [val2] "+r" (e), [val4] "+r" (f)
            : [val1] "r" (g), [val3] "r" (h)
            : "cc"
        );
        
        /* More computations to keep values live */
        i = e + f + g + h;
        j = i * a - b;
        k = j | c & d;
        l = k ^ (e << 2);
        m = l + (f >> 1);
        n = m * g / (h + 1);
        o = n ^ i;
        p = o | j;
        q = p & k;
        r = q + l;
        s = r - m;
        t = s * n;
        u = t / (o + 1);
        v = u ^ p;
        w = v | q;
        y = w & r;
        z = y + s;
        aa = z - t;
        ab = aa * u;
        ac = ab / (v + 1);
        ad = ac ^ w;
        ae = ad | y;
        af = ae & z;
        ag = af + aa;
        ah = ag - ab;
        ai = ah * ac;
        aj = ai / (ad + 1);
    }
    
    /* Final mixing of all values */
    result += a + b + c + d + e + f + g + h + i + j;
    result += k + l + m + n + o + p + q + r + s + t;
    result += u + v + w + y + z + aa + ab + ac + ad + ae;
    result += af + ag + ah + ai + aj;
    
    return result;
}

/* Another function with different patterns */
int mixed_mode_operations(int base) {
    /* Mixed integer types to force mode conversions */
    char c1 = base & 0xFF;
    short s1 = base >> 8;
    int i1 = base;
    long l1 = base * 100L;
    
    /* Operations causing sign/zero extensions */
    int r1 = c1 + s1;      /* char -> int, short -> int */
    int r2 = i1 * c1;      /* char -> int */
    long r3 = l1 / (s1 + 1); /* short -> long */
    int r4 = (short)r1 * (char)r2; /* multiple conversions */
    
    /* Bitfield operations */
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
        unsigned int d : 8;
    } bitfield;
    
    bitfield.a = r1 & 0xF;
    bitfield.b = r2 & 0xFF;
    bitfield.c = r3 & 0xFFF;
    bitfield.d = r4 & 0xFF;
    
    /* Access causing subreg operations */
    unsigned int bf_result = bitfield.a + bitfield.b + 
                            bitfield.c + bitfield.d;
    
    /* Vector-like operations using arrays */
    int vec1[4] = {r1, r2, r3 & 0xFFFF, r4};
    int vec2[4] = {5, 10, 15, 20};
    
    int dot = 0;
    for (int i = 0; i < 4; i++) {
        dot += vec1[i] * vec2[i];
    }
    
    return bf_result + dot + (int)r3;
}

/* Main function to drive everything */
int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * i;
    }
    
    /* Call complex function many times with different args */
    int total = 0;
    for (int run = 0; run < 10; run++) {
        int args[10];
        for (int i = 0; i < 10; i++) {
            args[i] = run * 100 + i * 10 + (v1 & 0xF);
        }
        
        total += complex_remat_test(args[0], args[1], args[2], args[3], args[4],
                                   args[5], args[6], args[7], args[8], args[9]);
        
        total += mixed_mode_operations(run * 50);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
