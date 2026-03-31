/* Test case for early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 0xABCDEF;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af;
    
    /* Complex arithmetic chain with many distinct intermediates */
    a = x1 + x2;                    /* 1 */
    b = a * x3;                     /* 2 */
    c = b - x4;                     /* 3 */
    d = c ^ x5;                     /* 4 */
    e = d | x6;                     /* 5 */
    f = e & x7;                     /* 6 */
    g = f + x8;                     /* 7 */
    h = g * x9;                     /* 8 */
    i = h - x10;                    /* 9 */
    j = i ^ a;                      /* 10 */
    k = j * b;                      /* 11 */
    l = k + c;                      /* 12 */
    m = l - d;                      /* 13 */
    n = m ^ e;                      /* 14 */
    o = n | f;                      /* 15 */
    p = o & g;                      /* 16 */
    q = p + h;                      /* 17 */
    r = q * i;                      /* 18 */
    s = r - j;                      /* 19 */
    t = s ^ k;                      /* 20 */
    u = t | l;                      /* 21 */
    v = u & m;                      /* 22 */
    w = v + n;                      /* 23 */
    x = w * o;                      /* 24 */
    y = x - p;                      /* 25 */
    z = y ^ q;                      /* 26 */
    aa = z | r;                     /* 27 */
    ab = aa & s;                    /* 28 */
    ac = ab + t;                    /* 29 */
    ad = ac * u;                    /* 30 */
    ae = ad - v;                    /* 31 */
    af = ae ^ w;                    /* 32 */
    
    /* Mixed-type operations to create mode changes */
    short s1 = (short)af;
    short s2 = (short)y;
    int mixed1 = (int)s1 * (int)s2;  /* Mode conversion: HImode to SImode */
    
    char c1 = (char)(mixed1 & 0xFF);
    int mixed2 = (int)c1 + z;        /* Mode conversion: QImode to SImode */
    
    /* Use volatile to create dataflow barriers */
    if (__builtin_expect(v1 > 10000, 0)) {
        mixed2 += v2;
    }
    
    /* Complex switch with different variable usage patterns */
    int selector = mixed2 & 0x7;
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + c + mixed1;
            /* Use address calculation that could be rematerialized */
            global_array[result & 0xFF] = d + e;
            break;
        case 1:
            result = d ^ e ^ f ^ mixed2;
            global_array[(result >> 8) & 0xFF] = g + h;
            break;
        case 2:
            result = g | h | i | (mixed1 & 0xFFFF);
            global_array[(result >> 16) & 0xFF] = j + k;
            break;
        case 3:
            result = j & k & l & (mixed2 & 0xFF);
            global_array[result & 0xFF] = m + n;
            break;
        case 4:
            result = m + n + o + mixed1;
            global_array[(result >> 8) & 0xFF] = p + q;
            break;
        case 5:
            result = p ^ q ^ r ^ mixed2;
            global_array[(result >> 16) & 0xFF] = s + t;
            break;
        case 6:
            result = s | t | u | (mixed1 & 0xFFFF);
            global_array[result & 0xFF] = v + w;
            break;
        case 7:
            result = v & w & x & (mixed2 & 0xFF);
            global_array[(result >> 8) & 0xFF] = y + z;
            break;
    }
    
    /* More arithmetic to keep values live */
    int final1 = result + aa + ab;
    int final2 = final1 * ac + ad;
    int final3 = final2 ^ ae ^ af;
    
    /* Another volatile barrier */
    if (__builtin_expect(v3 != 0, 1)) {
        final3 += v3;
    }
    
    /* Inline assembly to create complex dataflow patterns */
    asm volatile (
        "addl %1, %0\n\t"
        "imull %2, %0\n\t"
        : "+r" (final3)
        : "r" (mixed1), "r" (mixed2)
        : "cc"
    );
    
    return final3;
}

/* Second function with loop-based register pressure */
int __attribute__((noinline))
loop_pressure(int iterations) {
    int sum = 0;
    int i, j, k;
    
    /* Nested loops with many induction variables */
    for (i = 0; i < iterations; i++) {
        int base = i * 7;
        
        for (j = 0; j < 8; j++) {
            int idx1 = base + j;
            int idx2 = idx1 * 3;
            int idx3 = idx2 + 5;
            int idx4 = idx3 ^ 0xAA;
            int idx5 = idx4 | 0x55;
            
            /* Address calculations that could be rematerialized */
            int *ptr1 = &global_array[idx1 & 0xFF];
            int *ptr2 = &global_array[idx2 & 0xFF];
            int *ptr3 = &global_array[idx3 & 0xFF];
            
            /* Use the pointers in computations */
            int val1 = *ptr1 + idx4;
            int val2 = *ptr2 * idx5;
            int val3 = *ptr3 ^ idx1;
            
            /* More intermediate values */
            int tmp1 = val1 + val2;
            int tmp2 = tmp1 - val3;
            int tmp3 = tmp2 * idx2;
            int tmp4 = tmp3 | idx3;
            int tmp5 = tmp4 ^ idx4;
            int tmp6 = tmp5 & idx5;
            
            /* Mixed types again */
            short stmp = (short)tmp6;
            int itmp = (int)stmp * 2;
            
            sum += itmp + tmp1 + tmp2 + tmp3 + tmp4 + tmp5;
            
            /* Volatile read to prevent optimization */
            if (__builtin_expect((v1 & 1) == 0, 0)) {
                sum += v2;
            }
        }
        
        /* Switch inside loop for complex control flow */
        switch (i & 3) {
            case 0:
                sum += base * 2;
                break;
            case 1:
                sum += base ^ 0xFF;
                break;
            case 2:
                sum += base | 0xAA;
                break;
            case 3:
                sum += base & 0x55;
                break;
        }
    }
    
    return sum;
}

int main(void) {
    int i;
    
    /* Initialize global array */
    for (i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Call compute-heavy function multiple times with different args */
    int total = 0;
    
    for (i = 0; i < 100; i++) {
        /* Vary arguments to prevent constant propagation */
        int arg1 = i + v1;
        int arg2 = i * 2 + v2;
        int arg3 = i ^ 0x1234;
        int arg4 = i | 0xABCD;
        int arg5 = i & 0x5678;
        int arg6 = i * 3;
        int arg7 = i + 7;
        int arg8 = i ^ 0x9ABC;
        int arg9 = i | 0xDEF0;
        int arg10 = i & 0x2468;
        
        total += compute_heavy(arg1, arg2, arg3, arg4, arg5,
                              arg6, arg7, arg8, arg9, arg10);
        
        /* Also call loop pressure function periodically */
        if (i % 10 == 0) {
            total += loop_pressure(5);
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
