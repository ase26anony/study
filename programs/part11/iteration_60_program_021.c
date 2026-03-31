/* Test case for GCC early rematerialization pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Global array to force address calculations */
int global_array[256];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Use volatile inputs to prevent constant propagation */
    a = x1 + v1;
    b = x2 + v2;
    c = x3 + v3;
    d = x4 + v4;
    e = x5 + v5;
    
    /* Long chain of arithmetic operations creating many intermediate values */
    f = a * b + c;
    g = b * c + d;
    h = c * d + e;
    i = d * e + a;
    j = e * a + b;
    
    k = (f ^ g) | (h & i);
    l = (g ^ h) | (i & j);
    m = (h ^ i) | (j & f);
    n = (i ^ j) | (f & g);
    o = (j ^ f) | (g & h);
    
    p = k + l - m;
    q = l + m - n;
    r = m + n - o;
    s = n + o - p;
    t = o + p - q;
    
    u = p * q / (r + 1);
    v = q * r / (s + 1);
    w = r * s / (t + 1);
    x = s * t / (u + 1);
    y = t * u / (v + 1);
    
    /* Mixed-type operations to trigger mode changes */
    short sa = (short)a;
    short sb = (short)b;
    short sc = (short)c;
    
    z = (int)sa * (int)sb + (int)sc;
    aa = z << 2;
    ab = aa >> 1;
    
    /* Complex control flow with switch */
    int selector = (z + aa + ab) % 8;
    
    switch (selector) {
        case 0:
            ac = a + b + c + f + g;
            ad = ac * 2;
            break;
        case 1:
            ac = d + e + h + i;
            ad = ac / 2;
            break;
        case 2:
            ac = j + k + l + m;
            ad = ac ^ 0xFF;
            break;
        case 3:
            ac = n + o + p + q;
            ad = ac | 0x0F;
            break;
        case 4:
            ac = r + s + t + u;
            ad = ac & 0xF0;
            break;
        case 5:
            ac = v + w + x + y;
            ad = ~ac;
            break;
        case 6:
            ac = z + aa + ab;
            ad = ac * 3;
            break;
        case 7:
            ac = (a << 3) | (b << 2) | (c << 1);
            ad = ac % 17;
            break;
    }
    
    /* More arithmetic with mode mixing */
    ae = (short)ad + (char)ac;
    af = ae * 7;
    ag = af / 3;
    ah = ag << 4;
    ai = ah >> 2;
    
    /* Loop with address calculations (potential rematerialization) */
    int sum = 0;
    for (int idx = 0; idx < 16; idx++) {
        /* Base address calculation - may be rematerialized */
        int* addr = &global_array[idx * 4];
        
        /* Multiple uses of the address with different offsets */
        addr[0] = a + idx;
        addr[1] = b + idx * 2;
        addr[2] = c + idx * 3;
        addr[3] = d + idx * 4;
        
        /* Complex expression using many live values */
        int temp = (addr[0] * addr[1]) + (addr[2] * addr[3]) +
                   (e * idx) + (f * (idx + 1)) + (g * (idx + 2));
        
        /* Conditional with __builtin_expect */
        if (__builtin_expect((temp & 1) == 0, 0)) {
            temp += ac + ad + ae;
        } else {
            temp -= af + ag + ah;
        }
        
        sum += temp;
        
        /* Inline assembly to create complex dataflow */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "+r" (sum)
            : "r" (ai), "r" (idx)
            : "cc"
        );
    }
    
    /* Final aggregation using all computed values */
    aj = a + b + c + d + e + f + g + h + i + j +
         k + l + m + n + o + p + q + r + s + t +
         u + v + w + x + y + z + aa + ab + ac +
         ad + ae + af + ag + ah + ai + sum;
    
    return aj;
}

/* Another function with different patterns */
int __attribute__((noinline))
compute_heavy2(int x1, int x2, int x3, int x4) {
    /* Bit-field operations */
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
        unsigned int d : 8;
    } bits;
    
    bits.a = x1 & 0xF;
    bits.b = x2 & 0xFF;
    bits.c = x3 & 0xFFF;
    bits.d = x4 & 0xFF;
    
    /* Operations causing mode changes */
    int val1 = bits.a;
    int val2 = bits.b;
    int val3 = bits.c;
    int val4 = bits.d;
    
    /* Vector-like operations using arrays */
    int arr[8] = {val1, val2, val3, val4, val1^val2, val2^val3, val3^val4, val4^val1};
    
    int result = 0;
    for (int i = 0; i < 8; i++) {
        /* Complex addressing modes */
        result += arr[i] * (i + 1);
        result -= arr[7 - i] * (8 - i);
        
        /* Mode conversion */
        short s = (short)arr[i];
        result += (int)s * 2;
    }
    
    return result;
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Call compute-heavy functions multiple times with different args */
    int total = 0;
    
    for (int iter = 0; iter < 100; iter++) {
        /* Vary inputs to prevent optimization */
        int base = iter * 7;
        
        total += compute_heavy(
            base + 1, base + 2, base + 3, base + 4, base + 5,
            base + 6, base + 7, base + 8, base + 9, base + 10
        );
        
        total += compute_heavy2(
            base + 11, base + 12, base + 13, base + 14
        );
        
        /* Additional complex expression */
        int temp = 0;
        for (int j = 0; j < 8; j++) {
            temp = (temp << 3) | (iter & 7);
            
            /* Mixed-type operation */
            short stemp = (short)temp;
            int itemp = (int)stemp * j;
            
            /* Use inline asm with multiple operands */
            asm volatile (
                "imull %%ebx, %%eax\n\t"
                "addl %%ecx, %%eax\n\t"
                : "+a" (itemp)
                : "b" (j), "c" (temp)
                : "cc"
            );
            
            total += itemp;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
