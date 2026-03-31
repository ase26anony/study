/* Test case for GCC early rematerialization pass - targeting lines 930-937 in early-remat.cc */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops -fno-schedule-insns -fno-schedule-insns2 -march=x86-64 -mtune=generic -fPIC */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 54321;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) high_pressure_function(int p1, int p2, int p3, int p4, 
                                                     int p5, int p6, int p7, int p8) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj, ak, al;
    
    /* Initial computations creating many live values */
    a = p1 + p2;          /* 1 */
    b = p3 - p4;          /* 2 */
    c = a * b;            /* 3 */
    d = p5 ^ p6;          /* 4 */
    e = c | d;            /* 5 */
    f = p7 & p8;          /* 6 */
    g = e + f;            /* 7 */
    h = a - b;            /* 8 */
    i = c ^ d;            /* 9 */
    j = e & f;            /* 10 */
    k = g * h;            /* 11 */
    l = i | j;            /* 12 */
    m = k - l;            /* 13 */
    n = h + i;            /* 14 */
    o = j * k;            /* 15 */
    p = l - m;            /* 16 */
    q = n ^ o;            /* 17 */
    r = p & q;            /* 18 */
    s = m | n;            /* 19 */
    t = o + p;            /* 20 */
    
    /* More computations mixing in volatile reads (dataflow barriers) */
    u = t * v1;           /* 21 - volatile read creates barrier */
    v = q ^ v2;           /* 22 - another barrier */
    w = r | s;            /* 23 */
    x = u - v;            /* 24 */
    y = w * x;            /* 25 */
    z = t + u;            /* 26 */
    aa = v ^ w;           /* 27 */
    ab = x | y;           /* 28 */
    ac = z & aa;          /* 29 */
    ad = ab - ac;         /* 30 */
    ae = y * z;           /* 31 */
    af = aa ^ ab;         /* 32 */
    ag = ac | ad;         /* 33 */
    ah = ae & af;         /* 34 */
    ai = ag - ah;         /* 35 */
    aj = ad ^ ae;         /* 36 */
    ak = af & ag;         /* 37 */
    al = ah | ai;         /* 38 */
    
    /* Mixed-type operations to create mode conversions */
    short s1 = (short)aj;      /* int -> short conversion */
    short s2 = (short)ak;      /* another conversion */
    int i1 = (int)s1 * s2;     /* short -> int promotion */
    
    unsigned short us1 = (unsigned short)al;
    unsigned short us2 = (unsigned short)ai;
    unsigned int ui1 = (unsigned int)us1 + us2;
    
    /* Loop with complex control flow and address calculations */
    int sum = 0;
    for (int idx = 0; idx < 100; idx++) {
        /* Base address calculation - potential rematerialization candidate */
        int *base_addr = &global_array[idx];
        
        /* Switch statement creating complex control flow */
        switch (idx % 7) {
            case 0:
                *base_addr = a + b + i1;
                sum += *base_addr;
                break;
            case 1:
                *base_addr = c - d + ui1;
                sum += *base_addr;
                break;
            case 2:
                *base_addr = e * f + (int)s1;
                sum += *base_addr;
                break;
            case 3:
                *base_addr = g ^ h + (int)us1;
                sum += *base_addr;
                break;
            case 4:
                *base_addr = i | j + i1;
                sum += *base_addr;
                break;
            case 5:
                *base_addr = k & l + ui1;
                sum += *base_addr;
                break;
            case 6:
                *base_addr = m - n + (int)s2;
                sum += *base_addr;
                break;
        }
        
        /* More computations using live values */
        a = a ^ idx;
        b = b + idx;
        c = c * (idx + 1);
        d = d | idx;
        
        /* Inline assembly to create complex dataflow patterns */
        asm volatile (
            "mov %[val1], %%eax\n\t"
            "add %[val2], %%eax\n\t"
            "mov %%eax, %[result]"
            : [result] "=r" (e)
            : [val1] "r" (e), [val2] "r" (idx)
            : "eax"
        );
        
        /* Use __builtin_expect to create conditional basic blocks */
        if (__builtin_expect((idx & 3) == 0, 0)) {
            f = f * v3;  /* volatile read in conditional path */
            g = g ^ f;
        } else {
            h = h + idx;
            i = i | h;
        }
    }
    
    /* Final aggregation using many live values */
    int result = (a + b + c + d + e + f + g + h + i + j + 
                  k + l + m + n + o + p + q + r + s + t +
                  u + v + w + x + y + z + aa + ab + ac + ad +
                  ae + af + ag + ah + ai + aj + ak + al + 
                  i1 + ui1 + sum);
    
    return result;
}

/* Packed structure to create sub-register accesses */
struct __attribute__((packed)) packed_struct {
    unsigned char a;
    unsigned short b;
    unsigned int c;
    unsigned char d;
};

/* Another function using bit-fields and mixed types */
int __attribute__((noinline)) mixed_mode_function(struct packed_struct *ps) {
    /* Bit-field operations */
    struct {
        unsigned int f1 : 3;
        unsigned int f2 : 5;
        unsigned int f3 : 8;
        unsigned int f4 : 16;
    } bits;
    
    bits.f1 = ps->a & 0x7;
    bits.f2 = (ps->a >> 3) & 0x1F;
    bits.f3 = ps->b & 0xFF;
    bits.f4 = ps->c & 0xFFFF;
    
    /* Mixed-type arithmetic with mode conversions */
    int val1 = bits.f1;
    short val2 = bits.f2;
    unsigned int val3 = bits.f3;
    unsigned short val4 = bits.f4;
    
    /* Complex expression chain with mode mixing */
    int r1 = val1 + val2;           /* int + short -> int */
    unsigned int r2 = val3 * val4;  /* unsigned multiplication */
    int r3 = r1 - (int)r2;          /* signed-unsigned mix */
    unsigned int r4 = (unsigned int)r3 + val3;
    int r5 = (int)r4 ^ val1;
    
    /* Vector-like operations using arrays */
    int arr[4] = {r1, r2, r3, r4};
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += arr[i] * (i + 1);
    }
    
    return sum + r5;
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Create packed structure */
    struct packed_struct ps;
    ps.a = 0xAB;
    ps.b = 0xCDEF;
    ps.c = 0x12345678;
    ps.d = 0x99;
    
    /* Call high-pressure function with many arguments */
    int result1 = high_pressure_function(1, 2, 3, 4, 5, 6, 7, 8);
    
    /* Call again with different arguments */
    int result2 = high_pressure_function(9, 10, 11, 12, 13, 14, 15, 16);
    
    /* Call mixed-mode function */
    int result3 = mixed_mode_function(&ps);
    
    /* Final result to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
