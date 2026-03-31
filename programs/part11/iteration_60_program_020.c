/* Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops -fno-schedule-insns test.c -o test */
/* Also try: gcc -O3 -march=x86-64 -fPIC test.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 123, v2 = 456, v3 = 789;
volatile int v4 = 321, v5 = 654, v6 = 987;

/* Global array to force address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10)
{
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Initial computations creating many def-use chains */
    a = x1 + x2;           /* 1 */
    b = a * x3;            /* 2 */
    c = b - x4;            /* 3 */
    d = c ^ x5;            /* 4 */
    e = d | x6;            /* 5 */
    f = e & x7;            /* 6 */
    g = f + x8;            /* 7 */
    h = g * x9;            /* 8 */
    i = h - x10;           /* 9 */
    j = i ^ a;             /* 10 */
    k = j | b;             /* 11 */
    l = k & c;             /* 12 */
    m = l + d;             /* 13 */
    n = m * e;             /* 14 */
    o = n - f;             /* 15 */
    p = o ^ g;             /* 16 */
    q = p | h;             /* 17 */
    r = q & i;             /* 18 */
    s = r + j;             /* 19 */
    t = s * k;             /* 20 */
    u = t - l;             /* 21 */
    v = u ^ m;             /* 22 */
    w = v | n;             /* 23 */
    x = w & o;             /* 24 */
    y = x + p;             /* 25 */
    z = y * q;             /* 26 */
    aa = z - r;            /* 27 */
    ab = aa ^ s;           /* 28 */
    ac = ab | t;           /* 29 */
    ad = ac & u;           /* 30 */
    ae = ad + v;           /* 31 */
    af = ae * w;           /* 32 */
    ag = af - x;           /* 33 */
    ah = ag ^ y;           /* 34 */
    ai = ah | z;           /* 35 */
    aj = ai & aa;          /* 36 */
    
    /* Use volatile to create dataflow barriers */
    if (__builtin_expect(v1 > 0, 0)) {
        a += v1;
        b -= v2;
        c ^= v3;
    }
    
    /* Mixed-type arithmetic to cause mode conversions */
    short s1 = (short)a;
    short s2 = (short)b;
    short s3 = (short)c;
    int i1 = (int)s1 * (int)s2;
    int i2 = (int)s3 + (int)s1;
    
    /* Complex switch to create control flow complexity */
    int selector = (aj & 0xF);  /* 4-bit selector */
    
    switch (selector) {
        case 0:
            /* Use subset of live values with mode mixing */
            return (int)((short)(a + b) * (short)(c - d)) + e;
        case 1:
            return (int)((short)(f * g) | (short)(h & i)) + j;
        case 2:
            return (int)((short)(k ^ l) + (short)(m | n)) + o;
        case 3:
            return (int)((short)(p - q) * (short)(r + s)) + t;
        case 4:
            return (int)((short)(u & v) | (short)(w ^ x)) + y;
        case 5:
            return (int)((short)(z * aa) - (short)(ab & ac)) + ad;
        case 6:
            return (int)((short)(ae | af) ^ (short)(ag + ah)) + ai;
        case 7:
            return (int)((short)(aj * a) & (short)(b | c)) + d;
        case 8:
            return (int)((short)(e ^ f) + (short)(g & h)) + i;
        case 9:
            return (int)((short)(j * k) - (short)(l | m)) + n;
        case 10:
            return (int)((short)(o & p) ^ (short)(q + r)) + s;
        case 11:
            return (int)((short)(t | u) * (short)(v - w)) + x;
        case 12:
            return (int)((short)(y ^ z) & (short)(aa + ab)) + ac;
        case 13:
            return (int)((short)(ac * ad) | (short)(ae & af)) + ag;
        case 14:
            return (int)((short)(ah ^ ai) + (short)(aj | a)) + b;
        case 15:
            return (int)((short)(c & d) * (short)(e - f)) + g;
        default:
            return a + b + c + d + e + f + g + h + i + j;
    }
}

/* Function with address calculations that might be rematerialized */
int __attribute__((noinline))
process_array(int *arr, int n)
{
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Multiple address calculations - potential rematerialization */
        int *ptr1 = &arr[i];
        int *ptr2 = &arr[i + 1];
        int *ptr3 = &arr[i + 2];
        
        /* Use inline asm to create complex dataflow */
        int val1, val2, val3;
        asm volatile ("movl %1, %0" : "=r"(val1) : "m"(*ptr1));
        asm volatile ("movl %1, %0" : "=r"(val2) : "m"(*ptr2));
        asm volatile ("movl %1, %0" : "=r"(val3) : "m"(*ptr3));
        
        /* Complex expression with many intermediates */
        int t1 = val1 * val2;
        int t2 = val2 + val3;
        int t3 = val1 ^ val3;
        int t4 = t1 & t2;
        int t5 = t2 | t3;
        int t6 = t3 - t1;
        int t7 = t4 * t5;
        int t8 = t5 ^ t6;
        int t9 = t6 & t7;
        int t10 = t7 | t8;
        
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        
        /* Mode conversions */
        short s_t1 = (short)t1;
        short s_t2 = (short)t2;
        int mixed = (int)s_t1 * (int)s_t2 + t3;
        sum += mixed;
    }
    
    return sum;
}

/* Main function with everything combined */
int main(void)
{
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 7;
    }
    
    /* Call compute_heavy many times with different arguments */
    for (int iter = 0; iter < 1000; iter++) {
        /* Create varying inputs */
        int base = iter * 17;
        
        result += compute_heavy(
            base + 1, base + 2, base + 3, base + 4, base + 5,
            base + 6, base + 7, base + 8, base + 9, base + 10
        );
        
        /* Process array to trigger address rematerialization */
        if (iter % 3 == 0) {
            result += process_array(global_array, 64);
        }
        
        /* Use bit-fields for sub-register accesses */
        struct {
            unsigned int a : 4;
            unsigned int b : 8;
            unsigned int c : 12;
            unsigned int d : 8;
        } bits;
        
        bits.a = (result >> 0) & 0xF;
        bits.b = (result >> 4) & 0xFF;
        bits.c = (result >> 12) & 0xFFF;
        bits.d = (result >> 24) & 0xFF;
        
        /* Operations on bit-fields cause mode changes */
        int from_bits = (int)bits.a * (int)bits.b + 
                       (int)bits.c - (int)bits.d;
        result ^= from_bits;
        
        /* Vector-like operations using GCC vector extensions */
        typedef int v4si __attribute__((vector_size(16)));
        v4si v1 = {result, result + 1, result + 2, result + 3};
        v4si v2 = {result + 4, result + 5, result + 6, result + 7};
        v4si v3 = v1 + v2;
        v4si v4 = v1 * v2;
        
        /* Extract elements - creates register pressure */
        int v3_0 = v3[0], v3_1 = v3[1], v3_2 = v3[2], v3_3 = v3[3];
        int v4_0 = v4[0], v4_1 = v4[1], v4_2 = v4[2], v4_3 = v4[3];
        
        result += v3_0 + v3_1 + v3_2 + v3_3;
        result += v4_0 + v4_1 + v4_2 + v4_3;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
