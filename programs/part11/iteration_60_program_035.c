/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 54321;

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
    a = x1 + x2;           /* 1 */
    b = a * x3;            /* 2 */
    c = b - x4;            /* 3 */
    d = c ^ x5;            /* 4 */
    e = d | x6;            /* 5 */
    f = e & x7;            /* 6 */
    g = f + x8;            /* 7 */
    h = g - x9;            /* 8 */
    i = h * x10;           /* 9 */
    j = i / (x1 + 1);      /* 10 */
    k = j << 2;            /* 11 */
    l = k >> 1;            /* 12 */
    m = l & 0xFF;          /* 13 */
    n = m | 0x80;          /* 14 */
    o = n ^ 0x55;          /* 15 */
    p = o + v1;            /* 16 - volatile creates barrier */
    q = p - v2;            /* 17 */
    r = q * 3;             /* 18 */
    s = r / 2;             /* 19 */
    t = s % 17;            /* 20 */
    
    /* More computations with mode mixing */
    u = (short)t + (char)x2;      /* Mixed-size arithmetic */
    v = (unsigned int)u * 2999;   /* Different signedness */
    w = v & 0xFFFF;               /* Truncation */
    x = (w << 16) | w;            /* Combine */
    y = x + (long)v3;             /* Long addition */
    z = y - 0x7FFFFFFF;           /* Large constant */
    
    /* Additional intermediate values */
    aa = z * 2;
    ab = aa / 3;
    ac = ab << 4;
    ad = ac >> 2;
    ae = ad | 0xF0F0F0F0;
    af = ae & 0x0F0F0F0F;
    ag = af ^ 0xAAAAAAAA;
    ah = ag + 0x55555555;
    ai = ah * 7;
    aj = ai % 19;
    
    /* Complex loop with high register pressure */
    for (int iter = 0; iter < 100; iter++) {
        /* Use volatile to prevent optimization */
        if (__builtin_expect(v1 > 0, 1)) {
            /* Many live values used in switch */
            int selector = (aj + iter) % 8;
            
            /* Switch creates complex control flow for dataflow analysis */
            switch (selector) {
                case 0:
                    result += a + b + (short)c;  /* Mode mixing */
                    break;
                case 1:
                    result += d - e - (char)f;   /* More mode mixing */
                    break;
                case 2:
                    result += g * h * (unsigned short)i;
                    break;
                case 3:
                    result += j / (k + 1) + (long)l;
                    break;
                case 4:
                    result += m | n | (unsigned char)o;
                    break;
                case 5:
                    result += p ^ q ^ (short)r;
                    break;
                case 6:
                    result += s & t & (char)u;
                    break;
                case 7:
                    result += v - w - (unsigned int)x;
                    break;
            }
            
            /* Address calculations that may be rematerialized */
            int idx1 = (y + iter) % 256;
            int idx2 = (z - iter) % 256;
            int idx3 = (aa * iter) % 256;
            
            /* Multiple memory accesses with address calculations */
            result += global_array[idx1];
            result += global_array[idx2];
            result += global_array[idx3];
            
            /* Inline asm to create complex dataflow */
            asm volatile (
                "addl %1, %0\n\t"
                "subl %2, %0\n\t"
                : "+r" (result)
                : "r" (ab), "r" (ac)
                : "cc"
            );
        }
        
        /* Update some values to create new def-use chains */
        a = (a + 1) & 0xFF;
        b = (b * 2) % 100;
        c = c ^ iter;
        d = d | 0x01;
        e = e + v1;  /* Volatile access creates barrier */
        
        /* More computations keeping values live */
        af = ae + ag;
        ag = af - ah;
        ah = ag * ai;
        ai = ah / (aj + 1);
        aj = ai % 31;
    }
    
    /* Final computations mixing all types */
    short final_short = (short)result;
    char final_char = (char)(result >> 8);
    unsigned int final_uint = (unsigned int)result;
    long final_long = (long)result * 100;
    
    /* Use all final values */
    result = final_short + final_char + final_uint + (int)(final_long % 1000);
    
    return result;
}

/* Another function to create more compilation context */
void setup_array(void) {
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * i;
    }
}

int main(void) {
    setup_array();
    
    /* Call with many different values to create diverse dataflow */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += complex_remat_test(
            i * 1, i * 2, i * 3, i * 4, i * 5,
            i * 6, i * 7, i * 8, i * 9, i * 10
        );
    }
    
    printf("Result: %d\n", sum);
    return 0;
}
