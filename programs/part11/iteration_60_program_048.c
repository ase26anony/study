/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 123;
volatile int v2 = 456;
volatile int v3 = 789;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int complex_remat_test(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    int result = 0;
    
    /* Initial computations creating many live values */
    a = x1 + x2 + v1;          /* volatile creates barrier */
    b = a * x3 - x4;
    c = b ^ x5 + x6;
    d = c * 7 + x7;            /* constant for potential remat */
    e = d / (x8 | 1);
    f = e << 2;
    g = f - x9 * x10;
    h = g & 0xFF;
    i = h + (x1 * x2);
    j = i - (x3 / (x4 | 1));
    k = j * 3;                 /* another constant for remat */
    l = k | (x5 ^ x6);
    m = l + (x7 * 2);          /* constant 2 for remat */
    n = m - (x8 >> 1);
    o = n * (x9 + 1);
    p = o ^ (x10 * 3);
    q = p + (a * b) / 100;
    r = q - (c ^ d);
    s = r * (e & f);
    t = s + (g | h);
    u = t - (i * j);
    v = u ^ (k & l);
    w = v + (m | n);
    x = w - (o ^ p);
    y = x * (q + r);
    z = y ^ (s & t);
    aa = z + (u | v);
    ab = aa - (w * x);
    ac = ab ^ (y & z);
    ad = ac + (aa | ab);
    ae = ad - (ac * 2);        /* constant 2 for remat */
    af = ae ^ (ad & 3);        /* constant 3 for remat */
    ag = af + (ae * 4);        /* constant 4 for remat */
    ah = ag - (af >> 2);
    ai = ah ^ (ag << 1);
    aj = ai + (ah * 3);
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)aj;
    short s2 = (short)(aj >> 16);
    int mixed1 = (int)s1 * (int)s2;  /* mode extension */
    
    char c1 = (char)aj;
    unsigned char uc1 = (unsigned char)(aj >> 8);
    int mixed2 = (int)c1 + (int)uc1; /* sign/zero extension */
    
    /* Loop with high pressure and switch statement */
    for (int iter = 0; iter < 100; iter++) {
        /* Address calculation that could be rematerialized */
        int *ptr = &global_array[iter & 0xFF];
        
        /* Use volatile to prevent optimization */
        int base = v2 + iter;
        
        /* Complex switch creating different live value patterns */
        switch (iter & 0x7) {  /* 8 cases */
            case 0:
                result += a + *ptr + base;
                /* Mode mixing */
                result += (short)a * (int)b;
                break;
            case 1:
                result += b - *ptr + base;
                result += (char)b + (unsigned short)c;
                break;
            case 2:
                result += c ^ *ptr + base;
                result += (int)((short)d * (char)e);
                break;
            case 3:
                result += d | *ptr + base;
                result += (unsigned int)f + (signed int)g;
                break;
            case 4:
                result += e & *ptr + base;
                result += mixed1 - mixed2;
                break;
            case 5:
                result += f * *ptr + base;
                result += (short)h * (int)i;
                break;
            case 6:
                result += g - *ptr + base;
                result += (char)j + (unsigned short)k;
                break;
            case 7:
                result += h ^ *ptr + base;
                result += (int)((short)l * (char)m);
                break;
        }
        
        /* More computations keeping values live */
        a = (a + 1) & 0xFFF;
        b = (b * 3) & 0xFFF;      /* constant 3 */
        c = (c ^ base) & 0xFFF;
        d = (d - iter) & 0xFFF;
        e = (e | v3) & 0xFFF;     /* volatile */
        f = (f << 1) & 0xFFF;
        g = (g >> 1) & 0xFFF;
        h = (h + base) & 0xFFF;
        i = (i * 5) & 0xFFF;      /* constant 5 */
        j = (j ^ iter) & 0xFFF;
        
        /* Inline asm to create complex dataflow */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "subl %[val3], %[val2]\n\t"
            : [val2] "+r" (result)
            : [val1] "r" (a), [val3] "r" (b)
            : "cc"
        );
    }
    
    /* Final aggregation using all values */
    result += a + b + c + d + e + f + g + h + i + j;
    result += k + l + m + n + o + p + q + r + s + t;
    result += u + v + w + x + y + z + aa + ab + ac + ad;
    result += ae + af + ag + ah + ai + aj + mixed1 + mixed2;
    
    return result;
}

/* Helper to initialize array */
void init_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
}

int main(void) {
    init_array();
    
    /* Call with many parameters to increase register pressure */
    int result = complex_remat_test(
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    );
    
    /* Second call with different args */
    result += complex_remat_test(
        11, 12, 13, 14, 15, 16, 17, 18, 19, 20
    );
    
    printf("Result: %d\n", result);
    return 0;
}
