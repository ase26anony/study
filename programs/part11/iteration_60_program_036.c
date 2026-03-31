/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Mixed types to force mode conversions */
typedef struct {
    short s1, s2;
    int i1;
    long long ll1;
} mixed_t;

/* Packed structure for sub-register accesses */
struct __attribute__((packed)) packed_struct {
    char c;
    int i;
    short s;
};

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Mixed type variables for mode changes */
    short s1, s2, s3, s4, s5;
    long long ll1, ll2, ll3;
    
    /* Start with many arithmetic operations creating def-use chains */
    a = x1 + x2 + v1;          /* volatile forces register retention */
    b = a * x3 - v2;
    c = b ^ x4 + v3;
    d = c - x5 * v4;
    e = d | x6 & v5;
    f = e + x7 - v6;
    g = f * x8 + v7;
    h = g ^ x9 & v8;
    i = h - x10 * v9;
    j = i | x1 + v10;
    
    /* More operations creating intermediate values */
    k = (j << 3) | (a >> 2);
    l = k * b - c;
    m = l ^ d + e;
    n = m - f * g;
    o = n | h + i;
    p = o * j - k;
    q = p ^ l + m;
    r = q - n * o;
    s = r | p + q;
    t = s * r - q;
    
    /* Additional intermediate values */
    u = t ^ s + r;
    v = u - t * s;
    w = v | u + t;
    x = w * v - u;
    y = x ^ w + v;
    z = y - x * w;
    aa = z | y + x;
    ab = aa * z - y;
    ac = ab ^ aa + z;
    ad = ac - ab * aa;
    ae = ad | ac + ab;
    af = ae * ad - ac;
    ag = af ^ ae + ad;
    ah = ag - af * ae;
    ai = ah | ag + af;
    aj = ai * ah - ag;
    
    /* Mixed type operations forcing mode conversions */
    s1 = (short)a;     /* truncation to short */
    s2 = (short)b + s1;
    s3 = (short)c * s2;
    s4 = (short)d | s3;
    s5 = (short)e ^ s4;
    
    /* Promote back to int with sign extension */
    int s1_ext = (int)s1;
    int s2_ext = (int)s2;
    int s3_ext = (int)s3;
    int s4_ext = (int)s4;
    int s5_ext = (int)s5;
    
    /* Long long operations for different mode */
    ll1 = (long long)a * (long long)b;
    ll2 = ll1 + (long long)c;
    ll3 = ll2 * (long long)d;
    
    /* Complex switch with different variable usage patterns */
    int switch_val = aj & 0xF;  /* 0-15 range */
    int result = 0;
    
    switch (switch_val) {
        case 0:
            result = a + b + s1_ext + (int)(ll1 & 0xFFFFFFFF);
            break;
        case 1:
            result = c - d + s2_ext + (int)(ll1 >> 32);
            break;
        case 2:
            result = e * f + s3_ext + (int)(ll2 & 0xFFFFFFFF);
            break;
        case 3:
            result = g ^ h + s4_ext + (int)(ll2 >> 32);
            break;
        case 4:
            result = i | j + s5_ext + (int)(ll3 & 0xFFFFFFFF);
            break;
        case 5:
            result = k - l + s1_ext - s2_ext;
            break;
        case 6:
            result = m * n + s3_ext - s4_ext;
            break;
        case 7:
            result = o ^ p + s5_ext * s1_ext;
            break;
        case 8:
            result = q | r + s2_ext * s3_ext;
            break;
        case 9:
            result = s - t + s4_ext * s5_ext;
            break;
        case 10:
            result = u * v + (s1_ext << 2);
            break;
        case 11:
            result = w ^ x + (s2_ext >> 1);
            break;
        case 12:
            result = y | z + (s3_ext & 0xFF);
            break;
        case 13:
            result = aa - ab + (s4_ext | 0x55);
            break;
        case 14:
            result = ac * ad + (s5_ext ^ 0xAA);
            break;
        case 15:
            result = ae ^ af + (aj & 0xFF);
            break;
        default:
            result = ag + ah;
    }
    
    /* Use inline assembly to create complex dataflow patterns */
    int asm_result1, asm_result2;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (asm_result1)
        : "r" (result), "r" (ai)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r" (asm_result2)
        : "r" (asm_result1), "r" (aj)
        : "cc"
    );
    
    /* Packed structure access for sub-register modes */
    struct packed_struct ps;
    ps.c = (char)(result & 0xFF);
    ps.i = asm_result2;
    ps.s = (short)(aj & 0xFFFF);
    
    /* More arithmetic using packed structure members */
    int final1 = ps.i + (int)ps.c;
    int final2 = final1 * (int)ps.s;
    int final3 = final2 ^ ps.i;
    
    /* Loop with address calculations that might be rematerialized */
    int array[16];
    for (int idx = 0; idx < 16; idx++) {
        /* Base address calculation that could be rematerialized */
        int *addr = &array[idx];
        
        /* Use different subsets of live variables in each iteration */
        switch (idx % 4) {
            case 0:
                *addr = final1 + idx * a;
                break;
            case 1:
                *addr = final2 + idx * b;
                break;
            case 2:
                *addr = final3 + idx * c;
                break;
            case 3:
                *addr = ps.i + idx * d;
                break;
        }
        
        /* More computations to increase register pressure in loop */
        array[idx] += (e + f + g + h) * idx;
        array[idx] ^= (i | j | k | l) & idx;
        array[idx] *= (m - n - o - p) + idx;
    }
    
    /* Final aggregation */
    int checksum = 0;
    for (int idx = 0; idx < 16; idx++) {
        checksum ^= array[idx];
        checksum += idx * array[idx];
        checksum = (checksum << 3) | (checksum >> 29); /* rotate */
    }
    
    checksum += asm_result2 + final3 + result;
    return checksum;
}

/* Multiple similar functions to increase overall compilation complexity */
int __attribute__((noinline))
compute_medium(int x1, int x2, int x3) {
    int a = x1 + v1;
    int b = x2 * v2;
    int c = x3 ^ v3;
    
    /* Mixed modes */
    short s1 = (short)a;
    short s2 = (short)b;
    int d = (int)s1 * (int)s2 + c;
    
    /* Inline asm with multiple operands */
    int result;
    asm volatile (
        "leal (%1, %2, 2), %0\n\t"
        "addl %3, %0\n\t"
        : "=r" (result)
        : "r" (a), "r" (b), "r" (d)
        : "cc"
    );
    
    return result;
}

/* Main function with multiple calls to create complex call graph */
int main() {
    int total = 0;
    
    /* Call with many arguments to create register pressure in call setup */
    total += compute_heavy(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    total += compute_heavy(11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
    total += compute_heavy(21, 22, 23, 24, 25, 26, 27, 28, 29, 30);
    
    /* More calls with different patterns */
    for (int i = 0; i < 100; i++) {
        total += compute_medium(i, i*2, i*3);
        
        /* Conditional with many live values */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            total += compute_medium(total & 0xFF, (total >> 8) & 0xFF, 
                                   (total >> 16) & 0xFF);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
