/* Test case for early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 0xABCD;
volatile short v4 = 1000;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
high_pressure_computation(int x1, int x2, int x3, int x4, int x5,
                          int x6, int x7, int x8, int x9, int x10)
{
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah;
    short sa, sb, sc, sd, se, sf, sg, sh;
    unsigned int ua, ub, uc, ud, ue, uf, ug, uh;
    
    /* Initial computations creating many def-use chains */
    a = x1 + x2;           /* 1 */
    b = x3 - x4;           /* 2 */
    c = a * b;             /* 3 */
    d = x5 ^ x6;           /* 4 */
    e = c + d;             /* 5 */
    f = x7 & x8;           /* 6 */
    g = e | f;             /* 7 */
    h = x9 * x10;          /* 8 */
    i = g - h;             /* 9 */
    j = a + b + c;         /* 10 */
    k = d * e / 2;         /* 11 */
    l = f ^ g ^ h;         /* 12 */
    m = i + j - k;         /* 13 */
    n = l * m;             /* 14 */
    o = a * c * e;         /* 15 */
    p = b + d + f;         /* 16 */
    q = g * h * i;         /* 17 */
    r = j ^ k ^ l;         /* 18 */
    s = m + n + o;         /* 19 */
    t = p * q / r;         /* 20 */
    u = s - t;             /* 21 */
    v = a * u;             /* 22 */
    w = b + v;             /* 23 */
    x = c * w;             /* 24 */
    y = d + x;             /* 25 */
    z = e * y;             /* 26 */
    aa = f + z;            /* 27 */
    ab = g * aa;           /* 28 */
    ac = h + ab;           /* 29 */
    ad = i * ac;           /* 30 */
    ae = j + ad;           /* 31 */
    af = k * ae;           /* 32 */
    ag = l + af;           /* 33 */
    ah = m * ag;           /* 34 */
    
    /* Mixed-type operations to trigger mode changes */
    sa = (short)a;
    sb = (short)b;
    sc = sa * sb;          /* Mode: HI */
    sd = (short)c;
    se = sc + sd;          /* Mode: HI */
    sf = (short)d;
    sg = se ^ sf;          /* Mode: HI */
    sh = sg * v4;          /* Mode: HI with volatile */
    
    /* Unsigned operations for different mode */
    ua = (unsigned int)a;
    ub = (unsigned int)b;
    uc = ua * ub;          /* Mode: SI unsigned */
    ud = (unsigned int)c;
    ue = uc + ud;          /* Mode: SI unsigned */
    uf = (unsigned int)d;
    ug = ue ^ uf;          /* Mode: SI unsigned */
    uh = ug & 0xFFFF;      /* Mode: HI unsigned */
    
    /* Complex loop with switch to create control flow complexity */
    int result = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Use volatile to create dataflow barrier */
        int selector = (iter + v1) % 8;
        
        /* Address calculation that might be rematerialized */
        int *addr = &global_array[iter % 256];
        
        /* Switch with different live value usage in each case */
        switch (selector) {
            case 0:
                /* Use many live values in case 0 */
                *addr = a + b + c + d + e;
                result += *addr + sa + sc;
                break;
            case 1:
                /* Different subset in case 1 */
                *addr = f + g + h + i + j;
                result += *addr + sb + sd;
                break;
            case 2:
                /* More values in case 2 */
                *addr = k + l + m + n + o;
                result += *addr + se + sf;
                break;
            case 3:
                /* Mixed types in case 3 */
                *addr = p + q + r + s + t;
                result += *addr + (int)sg + (int)sh;
                break;
            case 4:
                /* Unsigned operations in case 4 */
                *addr = u + v + w + x + y;
                result += *addr + (int)ua + (int)ub;
                break;
            case 5:
                /* More unsigned in case 5 */
                *addr = z + aa + ab + ac + ad;
                result += *addr + (int)uc + (int)ud;
                break;
            case 6:
                /* Complex expression in case 6 */
                *addr = ae + af + ag + ah;
                result += *addr * 2 - (int)ue;
                break;
            case 7:
                /* All values in case 7 */
                *addr = (a + b + c + d + e + f + g + h + i + j +
                         k + l + m + n + o + p + q + r + s + t +
                         u + v + w + x + y + z + aa + ab + ac + ad +
                         ae + af + ag + ah);
                result += *addr / 2 + (int)uf + (int)ug + (int)uh;
                break;
        }
        
        /* Inline assembly to create complex dataflow */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "subl %[val3], %[val4]\n\t"
            : [val2] "+r" (b), [val4] "+r" (d)
            : [val1] "r" (a), [val3] "r" (c)
            : "cc"
        );
        
        /* Another volatile read as barrier */
        if (__builtin_expect(v3 > 1000, 0)) {
            a = v2;
        }
    }
    
    /* Final computation using all values */
    int final = 
        (a + b + c + d + e + f + g + h + i + j +
         k + l + m + n + o + p + q + r + s + t +
         u + v + w + x + y + z + aa + ab + ac + ad +
         ae + af + ag + ah +
         (int)sa + (int)sb + (int)sc + (int)sd +
         (int)se + (int)sf + (int)sg + (int)sh +
         (int)ua + (int)ub + (int)uc + (int)ud +
         (int)ue + (int)uf + (int)ug + (int)uh);
    
    return result + final;
}

/* Another function with different patterns */
int __attribute__((noinline))
secondary_computation(int base)
{
    /* Bit-field structure for sub-register accesses */
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
    int x = bits.a * bits.b;
    int y = bits.c + bits.d;
    int z = x ^ y;
    
    /* Vector-like operations using arrays */
    int arr[4] = {x, y, z, base};
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr[i] * (i + 1);
    }
    
    return sum;
}

int main(void)
{
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Call high-pressure function with many arguments */
    int result1 = high_pressure_computation(
        v1, v2, 3, 4, 5, 6, 7, 8, 9, 10
    );
    
    /* Call secondary function */
    int result2 = secondary_computation(result1);
    
    /* More computations to increase overall pressure */
    int total = 0;
    for (int i = 0; i < 1000; i++) {
        total += high_pressure_computation(
            i, i+1, i+2, i+3, i+4,
            i+5, i+6, i+7, i+8, i+9
        );
        total += secondary_computation(i);
    }
    
    printf("Result: %d\n", total + result1 + result2);
    return 0;
}
