/* Test case for GCC early rematerialization pass
 * Targeting lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 54321;

/* Global array to create address calculations */
int global_arr[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
high_pressure_function(int p1, int p2, int p3, int p4, int p5,
                       int p6, int p7, int p8, int p9, int p10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Initial computations creating many def-use chains */
    a = p1 + p2;          /* 1 */
    b = p3 - p4;          /* 2 */
    c = a * b;            /* 3 */
    d = p5 ^ p6;          /* 4 */
    e = c & d;            /* 5 */
    f = p7 | p8;          /* 6 */
    g = e + f;            /* 7 */
    h = p9 * p10;         /* 8 */
    i = g - h;            /* 9 */
    j = a + b + c;        /* 10 */
    k = d ^ e ^ f;        /* 11 */
    l = g * h * i;        /* 12 */
    m = j & k & l;        /* 13 */
    n = p1 * p3 * p5;     /* 14 */
    o = p2 + p4 + p6;     /* 15 */
    p = n | o;            /* 16 */
    q = m ^ p;            /* 17 */
    r = q << 3;           /* 18 */
    s = r >> 1;           /* 19 */
    t = s + v1;           /* 20 - volatile read creates barrier */
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)t;
    unsigned short us1 = (unsigned short)(t + 100);
    char c1 = (char)(t & 0xFF);
    
    /* More computations with mixed types */
    u = (int)s1 * 2;      /* 21 */
    v = (int)us1 / 3;     /* 22 */
    w = (int)c1 + 50;     /* 23 */
    x = u ^ v ^ w;        /* 24 */
    
    /* Complex loop with address calculations (rematerialization candidates) */
    int sum = 0;
    for (int idx = 0; idx < 32; idx++) {
        /* Base address calculation - candidate for rematerialization */
        int *addr = &global_arr[idx];
        
        /* Use the address in multiple independent accesses */
        int val1 = addr[0] + idx;
        int val2 = addr[1] * idx;
        int val3 = addr[2] ^ idx;
        
        /* Many intermediate values in loop */
        y = val1 + x;     /* 25 */
        z = val2 - u;     /* 26 */
        aa = val3 & v;    /* 27 */
        ab = y | z;       /* 28 */
        ac = aa ^ ab;     /* 29 */
        ad = ac * idx;    /* 30 */
        
        /* Switch statement creating complex control flow */
        switch (ad & 0x7) {  /* 8 cases for different dataflow */
            case 0:
                ae = ad + a + b;
                af = ae * c;
                break;
            case 1:
                ae = ad - d - e;
                af = ae / f;
                break;
            case 2:
                ae = ad ^ g ^ h;
                af = ae & i;
                break;
            case 3:
                ae = ad | j | k;
                af = ae ^ l;
                break;
            case 4:
                ae = ad & m & n;
                af = ae | o;
                break;
            case 5:
                ae = ad * p * q;
                af = ae - r;
                break;
            case 6:
                ae = ad + s + t;
                af = ae * u;
                break;
            case 7:
                ae = ad - v - w;
                af = ae / x;
                break;
        }
        
        /* More computations using switch results */
        ag = af + y + z;      /* 31 */
        ah = ag * aa * ab;    /* 32 */
        ai = ah ^ ac ^ ad;    /* 33 */
        aj = ai & ae & af;    /* 34 */
        
        /* Use volatile to prevent optimization */
        if (__builtin_expect(v2 > 0, 1)) {
            aj += v3;  /* volatile read */
        }
        
        sum += aj;
        
        /* Force register pressure by using all variables */
        a = (a + 1) & 0xFFF;
        b = (b - 1) & 0xFFF;
        c = c ^ idx;
        d = d | idx;
        e = e & ~idx;
    }
    
    /* Final aggregation using all computed values */
    int result = a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t +
                 u + v + w + x + y + z + aa + ab + ac + ad +
                 ae + af + ag + ah + ai + aj + sum;
    
    return result;
}

/* Another function with inline asm to create complex dataflow */
int __attribute__((noinline))
asm_dataflow(int x, int y) {
    int r1, r2, r3;
    
    /* Inline asm with multiple operands creates complex DF */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (r1)
        : "r" (x), "r" (y)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "shrl $2, %0\n\t"
        : "+r" (r1)
        : "r" (r1)
        : "cc"
    );
    
    /* More operations to extend live ranges */
    r2 = r1 * 3;
    r3 = r2 / 2;
    
    /* Use bit-fields for sub-register accesses */
    struct {
        unsigned int low : 8;
        unsigned int high : 24;
    } bf;
    
    bf.low = r3 & 0xFF;
    bf.high = (r3 >> 8) & 0xFFFFFF;
    
    return bf.low + bf.high;
}

/* Main function that creates maximum pressure */
int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = i * 3 + 1;
    }
    
    /* Call high-pressure function with many arguments */
    int result1 = high_pressure_function(
        v1, v2, v3, 4, 5, 6, 7, 8, 9, 10
    );
    
    /* Call asm function */
    int result2 = asm_dataflow(result1, v1);
    
    /* More computations to keep values live */
    int final = result1 + result2 * 2;
    
    /* Use vector operations for specific modes */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* Extract elements to force scalar operations */
    int vecelem = vec3[0] + vec4[1];
    final += vecelem;
    
    printf("Result: %d\n", final);
    
    return 0;
}
