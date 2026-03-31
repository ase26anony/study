/* Test case for early rematerialization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Complex expression chain with many intermediate values */
    a = x1 + x2;                    /* 1 */
    b = a * x3;                     /* 2 */
    c = b - x4;                     /* 3 */
    d = c ^ x5;                     /* 4 */
    e = d | x6;                     /* 5 */
    f = e & x7;                     /* 6 */
    g = f + x8;                     /* 7 */
    h = g - x9;                     /* 8 */
    i = h * x10;                    /* 9 */
    j = i + v1;                     /* 10 - volatile barrier */
    
    /* More arithmetic creating def-use chains */
    k = j << 2;                     /* 11 */
    l = k >> 1;                     /* 12 */
    m = l & 0xFF;                   /* 13 */
    n = m | 0x80;                   /* 14 */
    o = n ^ 0x55;                   /* 15 */
    p = o + 42;                     /* 16 - constant for rematerialization */
    q = p - 17;                     /* 17 */
    r = q * 3;                      /* 18 */
    s = r / 2;                      /* 19 */
    t = s % 7;                      /* 20 */
    
    /* Mode mixing: use different integer sizes */
    short s1 = (short)t;
    unsigned short us1 = (unsigned short)(t + 100);
    char c1 = (char)(t & 0xFF);
    unsigned char uc1 = (unsigned char)(t | 0x80);
    
    /* More computations with mixed modes */
    u = (int)s1 + (int)us1;         /* 21 */
    v = (int)c1 * (int)uc1;         /* 22 */
    w = u - v;                      /* 23 */
    x = w << (c1 & 3);              /* 24 */
    y = x >> (uc1 & 3);             /* 25 */
    z = y + p;                      /* 26 - reuse constant p */
    
    /* Create address calculation for rematerialization */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Use addresses in computations */
    aa = (*ptr1) + z;               /* 27 */
    ab = (*ptr2) - y;               /* 28 */
    ac = (*ptr3) * x;               /* 29 */
    
    /* More values for register pressure */
    ad = aa ^ ab;                   /* 30 */
    ae = ad | ac;                   /* 31 */
    af = ae & 0xFFFFFFFF;           /* 32 */
    ag = af + v2;                   /* 33 - volatile */
    ah = ag - v3;                   /* 34 - volatile */
    ai = ah * v4;                   /* 35 - volatile */
    aj = ai / (v5 + 1);             /* 36 - volatile */
    
    /* Complex control flow with switch */
    int result = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Recompute some values inside loop to encourage rematerialization */
        int loop_a = a + iter;
        int loop_b = b - iter;
        int loop_c = c * iter;
        int loop_d = d ^ iter;
        int loop_e = e | iter;
        
        /* Switch creates complex dataflow */
        switch (iter & 7) {
            case 0:
                result += loop_a + aa + (short)aj;
                break;
            case 1:
                result += loop_b - ab + (unsigned short)aj;
                break;
            case 2:
                result += loop_c * ac + (char)aj;
                break;
            case 3:
                result += loop_d ^ ad + (unsigned char)aj;
                break;
            case 4:
                result += loop_e | ae + aj;
                break;
            case 5:
                result += (loop_a & loop_b) + af;
                break;
            case 6:
                result += (loop_c | loop_d) + ag;
                break;
            case 7:
                result += (loop_e ^ loop_a) + ah;
                break;
        }
        
        /* Inline asm to create complex dataflow patterns */
        asm volatile (
            "addl %[val1], %[val2]\n\t"
            "subl %[val3], %[val4]\n\t"
            : [val2] "+r" (loop_b), [val4] "+r" (loop_d)
            : [val1] "r" (loop_a), [val3] "r" (loop_c)
            : "cc"
        );
        
        /* Use volatile as condition */
        if (v6) {
            result += loop_b * 2;
        }
        if (v7) {
            result += loop_d / 2;
        }
    }
    
    /* Final mixing of all values */
    return result + a + b + c + d + e + f + g + h + i + j +
           k + l + m + n + o + p + q + r + s + t +
           u + v + w + x + y + z + aa + ab + ac + ad +
           ae + af + ag + ah + ai + aj;
}

/* Another function with vector operations for mode mixing */
void __attribute__((noinline))
vector_ops(int *result) {
    /* Use GCC vector extensions for different modes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    typedef char v16qi __attribute__((vector_size(16)));
    
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    /* Convert between vector types (mode changes) */
    v8hi vec5 = __builtin_convertvector(vec3, v8hi);
    v16qi vec6 = __builtin_convertvector(vec4, v16qi);
    
    /* Extract elements to scalar values */
    for (int i = 0; i < 4; i++) {
        result[i] = vec3[i] + vec5[i*2] + vec6[i*4];
    }
}

/* Main function with high register pressure */
int main() {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Create many input values */
    int inputs[20];
    for (int i = 0; i < 20; i++) {
        inputs[i] = i * 7 + 3;
    }
    
    /* Call compute-heavy function multiple times */
    int total = 0;
    for (int outer = 0; outer < 50; outer++) {
        /* Vary inputs slightly each iteration */
        inputs[outer % 20] += outer;
        
        total += compute_heavy(
            inputs[0], inputs[1], inputs[2], inputs[3], inputs[4],
            inputs[5], inputs[6], inputs[7], inputs[8], inputs[9]
        );
        
        /* Use vector operations */
        int vec_result[4];
        vector_ops(vec_result);
        total += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
        
        /* Bitfield operations for sub-register accesses */
        struct {
            unsigned int a : 4;
            unsigned int b : 8;
            unsigned int c : 12;
            unsigned int d : 8;
        } bitfield;
        
        bitfield.a = (total >> 0) & 0xF;
        bitfield.b = (total >> 4) & 0xFF;
        bitfield.c = (total >> 12) & 0xFFF;
        bitfield.d = (total >> 24) & 0xFF;
        
        total = bitfield.a + bitfield.b + bitfield.c + bitfield.d;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
