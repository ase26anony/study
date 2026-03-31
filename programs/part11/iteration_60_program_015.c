/* Test case for early rematerialization with virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Mixed types to force mode conversions */
typedef struct {
    int a : 4;
    int b : 12;
    int c : 16;
} bitfield_t;

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many live variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af;
    bitfield_t bf1, bf2;
    short s1, s2, s3, s4;
    unsigned char uc1, uc2;
    
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
    j = i ^ v1;            /* 10 */
    k = j | v2;            /* 11 */
    l = k & v3;            /* 12 */
    m = l + v4;            /* 13 */
    n = m * v5;            /* 14 */
    o = n - v6;            /* 15 */
    p = o ^ v7;            /* 16 */
    q = p | v8;            /* 17 */
    r = q & v9;            /* 18 */
    s = r + v10;           /* 19 */
    t = s * x1;            /* 20 */
    u = t - x2;            /* 21 */
    v = u ^ x3;            /* 22 */
    w = v | x4;            /* 23 */
    x = w & x5;            /* 24 */
    y = x + x6;            /* 25 */
    z = y * x7;            /* 26 */
    aa = z - x8;           /* 27 */
    ab = aa ^ x9;          /* 28 */
    ac = ab | x10;         /* 29 */
    ad = ac & v1;          /* 30 */
    ae = ad + v2;          /* 31 */
    af = ae * v3;          /* 32 */
    
    /* Mixed-type operations to force mode conversions */
    s1 = (short)af;
    s2 = (short)(af >> 8);
    s3 = s1 + s2;
    s4 = s3 * (short)v4;
    
    uc1 = (unsigned char)s4;
    uc2 = (unsigned char)(s4 >> 8);
    
    /* Bitfield operations - often create subreg accesses */
    bf1.a = uc1 & 0xF;
    bf1.b = uc1 << 4;
    bf1.c = s3;
    
    bf2.a = uc2 & 0xF;
    bf2.b = uc2 << 4;
    bf2.c = s4;
    
    /* Complex loop with switch to create control flow complexity */
    int result = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Volatile read acts as dataflow barrier */
        int selector = (iter + v1) % 8;
        
        /* Switch with different live value usage in each case */
        switch (selector) {
            case 0:
                /* Use many live values in case 0 */
                result += a + b - c + d - e + f - g;
                result += (int)s1 * (int)s2;
                result += bf1.a + bf1.b;
                break;
            case 1:
                /* Different subset in case 1 */
                result += h + i - j + k - l + m - n;
                result += (int)s3 * (int)s4;
                result += bf2.a + bf2.b;
                break;
            case 2:
                /* More values in case 2 */
                result += o + p - q + r - s + t - u;
                result += uc1 * uc2;
                result += bf1.c;
                break;
            case 3:
                /* Even more in case 3 */
                result += v + w - x + y - z + aa - ab;
                result += (short)uc1 * (short)uc2;
                result += bf2.c;
                break;
            case 4:
                /* Mix of values in case 4 */
                result += ac + ad - ae + af;
                result += a * c - e * g;
                result += bf1.a * bf2.a;
                break;
            case 5:
                /* Different mix in case 5 */
                result += h * j - l * n;
                result += p * r - t * v;
                result += bf1.b * bf2.b;
                break;
            case 6:
                /* More computations in case 6 */
                result += x * z - aa * ac;
                result += s1 * s3 - s2 * s4;
                result += bf1.c * bf2.c;
                break;
            case 7:
                /* All remaining values in case 7 */
                result += a + c + e + g + i + k + m + o + q + s;
                result += u + w + y + aa + ac + ae;
                result += (int)bf1.a + (int)bf2.a + (int)bf1.c;
                break;
        }
        
        /* Inline assembly to create complex dataflow patterns */
        asm volatile (
            "addl %[val1], %[res]\n\t"
            "subl %[val2], %[res]\n\t"
            : [res] "+r" (result)
            : [val1] "r" (iter), [val2] "r" (selector)
            : "cc"
        );
        
        /* Address calculations that are good remat candidates */
        int* ptr1 = &result + iter;
        int* ptr2 = ptr1 + selector;
        int* ptr3 = ptr2 - iter;
        
        /* Use the pointers to create memory operations */
        if (iter % 3 == 0) {
            *ptr1 = result + 1;
        }
        if (iter % 5 == 0) {
            *ptr2 = result - 1;
        }
        if (iter % 7 == 0) {
            *ptr3 = result * 2;
        }
        
        /* Mode mixing operations */
        result = (result & 0xFFFF) + ((short)result * 2);
        result = (result << 4) | ((unsigned char)result & 0xF);
    }
    
    /* Final aggregation using all computed values */
    int final = result;
    final += a + b + c + d + e + f + g + h + i + j;
    final += k + l + m + n + o + p + q + r + s + t;
    final += u + v + w + x + y + z + aa + ab + ac + ad;
    final += ae + af;
    final += s1 + s2 + s3 + s4;
    final += uc1 + uc2;
    final += bf1.a + bf1.b + bf1.c;
    final += bf2.a + bf2.b + bf2.c;
    
    return final;
}

/* Another function to increase compilation unit complexity */
void __attribute__((noinline))
use_vector_types(int* result) {
    /* Vector types for different machine modes */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    typedef char v16qi __attribute__((vector_size(16)));
    
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v8hi vec3 = {9, 10, 11, 12, 13, 14, 15, 16};
    v16qi vec4 = {17, 18, 19, 20, 21, 22, 23, 24,
                  25, 26, 27, 28, 29, 30, 31, 32};
    
    /* Vector operations */
    v4si vec5 = vec1 + vec2;
    v4si vec6 = vec1 * vec2;
    v8hi vec7 = vec3 + (v8hi){1, 1, 1, 1, 1, 1, 1, 1};
    
    /* Mix vector and scalar */
    for (int i = 0; i < 4; i++) {
        *result += vec5[i] + vec6[i];
    }
    for (int i = 0; i < 8; i++) {
        *result += vec7[i];
    }
    
    /* More mode mixing */
    v16qi vec8 = vec4 * (v16qi){2, 2, 2, 2, 2, 2, 2, 2,
                                2, 2, 2, 2, 2, 2, 2, 2};
    for (int i = 0; i < 16; i++) {
        *result += vec8[i];
    }
}

int main() {
    /* Initialize with volatile reads to prevent constant propagation */
    int x1 = v1, x2 = v2, x3 = v3, x4 = v4, x5 = v5;
    int x6 = v6, x7 = v7, x8 = v8, x9 = v9, x10 = v10;
    
    /* Call compute-heavy function multiple times */
    int total = 0;
    for (int outer = 0; outer < 10; outer++) {
        /* Modify inputs slightly each iteration */
        x1 += outer; x2 -= outer; x3 ^= outer; x4 |= outer;
        
        int result = compute_heavy(x1, x2, x3, x4, x5,
                                   x6, x7, x8, x9, x10);
        total += result;
        
        /* Call vector function */
        use_vector_types(&total);
        
        /* Additional computations to increase register pressure */
        int temp = 0;
        for (int inner = 0; inner < 50; inner++) {
            /* Many temporary variables in inner loop */
            int t1 = total + inner;
            int t2 = t1 * outer;
            int t3 = t2 - inner;
            int t4 = t3 ^ outer;
            int t5 = t4 | inner;
            int t6 = t5 & outer;
            int t7 = t6 + inner;
            int t8 = t7 * outer;
            int t9 = t8 - inner;
            int t10 = t9 ^ outer;
            
            /* Mix types */
            short st1 = (short)t1;
            short st2 = (short)t2;
            char ct1 = (char)t3;
            char ct2 = (char)t4;
            
            temp += t10 + st1 + st2 + ct1 + ct2;
            
            /* Conditional with __builtin_expect */
            if (__builtin_expect((inner & 3) == 0, 0)) {
                temp += t5 + t6 + t7;
            }
        }
        total += temp;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
