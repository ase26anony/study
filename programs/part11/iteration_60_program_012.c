/* Test case for early rematerialization virtual register creation */
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
    int result = 0;
    
    /* Initial computations creating many live values */
    a = p1 + p2;           /* 1 */
    b = p3 - p4;           /* 2 */
    c = a * b;             /* 3 */
    d = p5 ^ p6;           /* 4 */
    e = c | d;             /* 5 */
    f = p7 & p8;           /* 6 */
    g = e + f;             /* 7 */
    h = p9 * p10;          /* 8 */
    i = g - h;             /* 9 */
    j = a + c;             /* 10 */
    k = b + d;             /* 11 */
    l = j * k;             /* 12 */
    m = e & f;             /* 13 */
    n = g | h;             /* 14 */
    o = i ^ j;             /* 15 */
    p = k + l;             /* 16 */
    q = m - n;             /* 17 */
    r = o * p;             /* 18 */
    s = q & r;             /* 19 */
    t = s | v1;            /* 20 - volatile creates barrier */
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)a;
    short s2 = (short)b;
    int i1 = (int)s1 + (int)s2;  /* Mode conversion */
    
    char c1 = (char)c;
    char c2 = (char)d;
    int i2 = (int)c1 * (int)c2;  /* Another mode conversion */
    
    /* Complex loop with many live values */
    for (int iter = 0; iter < 100; iter++) {
        /* Use volatile to prevent optimization */
        if (__builtin_expect(v2 > 0, 1)) {
            /* More computations keeping values live */
            u = t + iter;          /* 21 */
            v = u * a;             /* 22 */
            w = v / (b + 1);       /* 23 */
            x = w ^ c;             /* 24 */
            y = x & d;             /* 25 */
            z = y | e;             /* 26 */
            aa = z + f;            /* 27 */
            ab = aa * g;           /* 28 */
            ac = ab - h;           /* 29 */
            ad = ac ^ i;           /* 30 */
            ae = ad & j;           /* 31 */
            af = ae | k;           /* 32 */
            ag = af + l;           /* 33 */
            ah = ag * m;           /* 34 */
            ai = ah - n;           /* 35 */
            aj = ai ^ o;           /* 36 */
            
            /* Switch statement creating complex control flow */
            switch (iter % 7) {
                case 0:
                    result += p + q + r + s1 + i1;
                    break;
                case 1:
                    result += t + u + v + s2 + i2;
                    break;
                case 2:
                    result += w + x + y + c1 + (int)c2;
                    break;
                case 3:
                    result += z + aa + ab + (short)ac;
                    break;
                case 4:
                    result += ad + ae + af + (char)ag;
                    break;
                case 5:
                    result += ah + ai + aj + (short)ah;
                    break;
                case 6:
                    /* Use inline assembly to create complex dataflow */
                    asm volatile (
                        "addl %1, %0\n\t"
                        "subl %2, %0\n\t"
                        : "+r" (result)
                        : "r" (v3), "r" (iter)
                        : "cc"
                    );
                    break;
            }
            
            /* Address calculations that might be rematerialized */
            int *ptr1 = &global_arr[iter % 256];
            int *ptr2 = &global_arr[(iter + a) % 256];
            int *ptr3 = &global_arr[(iter + b) % 256];
            
            /* Use the pointers in computations */
            *ptr1 = result;
            result += *ptr2 + *ptr3;
            
            /* Mixed-width operations */
            long long ll1 = (long long)result * (long long)v1;
            int truncated = (int)(ll1 >> 16);
            result = truncated & 0xFFFF;
        }
    }
    
    /* Final computations using all live values */
    result = (result + a + b + c + d + e + f + g + h + i + j +
              k + l + m + n + o + p + q + r + s + t) & 0xFFF;
    
    return result;
}

/* Another function with different patterns */
int __attribute__((noinline))
another_high_pressure_function(int base) {
    int vals[32];
    int sum = 0;
    
    /* Initialize with computations */
    for (int i = 0; i < 32; i++) {
        vals[i] = base * i + (i % 3);
    }
    
    /* Complex computation graph */
    for (int i = 0; i < 1000; i++) {
        int idx = i % 32;
        
        /* Many intermediate values */
        int t1 = vals[idx] * 3;
        int t2 = t1 + 5;
        int t3 = t2 / 2;
        int t4 = t3 ^ 0x55;
        int t5 = t4 & 0xFF;
        int t6 = t5 | 0xAA;
        int t7 = t6 << 2;
        int t8 = t7 >> 1;
        int t9 = t8 + vals[(idx + 1) % 32];
        int t10 = t9 - vals[(idx + 2) % 32];
        int t11 = t10 * vals[(idx + 3) % 32];
        int t12 = t11 / (vals[(idx + 4) % 32] + 1);
        int t13 = t12 ^ vals[(idx + 5) % 32];
        int t14 = t13 & vals[(idx + 6) % 32];
        int t15 = t14 | vals[(idx + 7) % 32];
        int t16 = t15 + vals[(idx + 8) % 32];
        int t17 = t16 - vals[(idx + 9) % 32];
        int t18 = t17 * vals[(idx + 10) % 32];
        int t19 = t18 / (vals[(idx + 11) % 32] + 1);
        int t20 = t19 ^ 0x1234;
        
        /* Use bitfields for sub-register accesses */
        struct {
            unsigned int low : 8;
            unsigned int mid : 8;
            unsigned int high : 16;
        } bf;
        
        bf.low = t20 & 0xFF;
        bf.mid = (t20 >> 8) & 0xFF;
        bf.high = (t20 >> 16) & 0xFFFF;
        
        sum += bf.low + bf.mid + bf.high;
        
        /* Update array creating data dependencies */
        vals[idx] = sum & 0xFF;
    }
    
    return sum;
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = i * 3;
    }
    
    int result1 = high_pressure_function(
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    );
    
    int result2 = another_high_pressure_function(result1);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    
    return (result1 + result2) > 0 ? 0 : 1;
}
