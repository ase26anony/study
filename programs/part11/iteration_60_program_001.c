/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 54321;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah;
    
    /* Complex expression chain with many distinct intermediate values */
    /* First chain - integer arithmetic */
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
    
    /* Second chain - mixing with volatile for dataflow complexity */
    u = t + v1;            /* 21 - volatile creates barrier */
    v = u - v2;            /* 22 */
    w = v * v3;            /* 23 */
    x = w ^ t;             /* 24 */
    y = x | u;             /* 25 */
    z = y & v;             /* 26 */
    aa = z + w;            /* 27 */
    ab = aa * x;           /* 28 */
    ac = ab - y;           /* 29 */
    ad = ac ^ z;           /* 30 */
    ae = ad | aa;          /* 31 */
    af = ae & ab;          /* 32 */
    ag = af + ac;          /* 33 */
    ah = ag * ad;          /* 34 */
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)ae;  /* Mode change: int -> short */
    short s2 = (short)af;
    int i1 = (int)s1 * (int)s2;  /* Mode change: short -> int */
    
    unsigned short us1 = (unsigned short)ag;
    unsigned short us2 = (unsigned short)ah;
    unsigned int ui1 = (unsigned int)us1 + (unsigned int)us2;
    
    /* Use in address calculations (potential rematerialization candidates) */
    int *ptr1 = &global_array[ae & 0xFF];
    int *ptr2 = &global_array[af & 0xFF];
    int *ptr3 = &global_array[ag & 0xFF];
    
    /* Force these addresses to be used */
    *ptr1 = i1;
    *ptr2 = ui1;
    *ptr3 = ae + af;
    
    /* Complex switch to create control flow with varying live sets */
    int switch_val = (ah + v1) % 8;
    
    switch (switch_val) {
        case 0:
            /* Use subset 1 */
            return a + c + e + g + i + (int)s1;
        case 1:
            /* Use subset 2 */
            return b + d + f + h + j + (int)s2;
        case 2:
            /* Use subset 3 with mode mixing */
            return k + m + o + q + s + i1;
        case 3:
            /* Use subset 4 */
            return l + n + p + r + t + ui1;
        case 4:
            /* Use subset 5 with address calculations */
            return *ptr1 + *ptr2 + u + w;
        case 5:
            /* Use subset 6 */
            return v + x + z + aa + ac;
        case 6:
            /* Use subset 7 */
            return y + ab + ad + ae + ag;
        case 7:
            /* Use subset 8 with all values */
            return (a + b + c + d + e + f + g + h + i + j +
                    k + l + m + n + o + p + q + r + s + t +
                    u + v + w + x + y + z + aa + ab + ac + ad +
                    ae + af + ag + ah + i1 + ui1) & 0xFFFF;
        default:
            return 0;
    }
}

/* Another function with inline asm to create complex DF references */
int __attribute__((noinline))
asm_constraints(int x, int y) {
    int result1, result2;
    
    /* Inline asm with multiple constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (result1)
        : "r" (x), "r" (y)
        : "cc"
    );
    
    /* Another asm with different constraints */
    asm volatile (
        "imull %1, %0\n\t"
        : "+r" (result1)
        : "r" (result1)
        : "cc"
    );
    
    /* Use volatile to force dataflow */
    result2 = result1 + v1;
    
    /* Mixed operations */
    short s_val = (short)result2;
    int extended = (int)s_val * 2;
    
    return extended + result1;
}

/* Main function with loops and high register pressure */
int main(void) {
    int i, j;
    int total = 0;
    
    /* Initialize global array */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Outer loop to increase optimization scope */
    for (i = 0; i < 100; i++) {
        /* Inner loop with high register pressure */
        for (j = 0; j < 50; j++) {
            /* Call compute_heavy with many different arguments */
            int val = compute_heavy(
                i + j, i - j, i * 3, j * 5, i ^ j,
                i | j, i & j, i + 1, j + 2, i * j
            );
            
            /* Mix with asm_constraints for complex DF */
            int asm_val = asm_constraints(val, i + j);
            
            /* Use in conditional with __builtin_expect */
            if (__builtin_expect((asm_val & 1) != 0, 0)) {
                total += asm_val * 2;
            } else {
                total += asm_val / 2;
            }
            
            /* More arithmetic to increase pressure */
            total += (val & 0xFF) - ((val >> 8) & 0xFF) + 
                    ((val >> 16) & 0xFF) - ((val >> 24) & 0xFF);
            
            /* Use bit-fields for sub-register accesses */
            struct {
                unsigned int low : 8;
                unsigned int mid : 8;
                unsigned int high : 16;
            } bits;
            
            bits.low = total & 0xFF;
            bits.mid = (total >> 8) & 0xFF;
            bits.high = (total >> 16) & 0xFFFF;
            
            total = bits.low + bits.mid * 3 + bits.high;
        }
        
        /* Additional computation between loop iterations */
        total = total ^ (v1 + i);
        total = total | (v2 - i);
        total = total & (v3 * i);
    }
    
    /* Final aggregation */
    int checksum = 0;
    for (i = 0; i < 256; i++) {
        checksum += global_array[i];
    }
    
    printf("Result: total = %d, checksum = %d\n", total, checksum);
    return total != 0 ? 0 : 1;
}
