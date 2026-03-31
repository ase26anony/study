/* Test program to trigger early rematerialization with virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Global array to create address calculations */
int global_array[1000];

/* Complex function with high register pressure */
int complex_remat_test(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    int result = 0;
    
    /* Initial computations creating many live values */
    a = x1 + x2 + v1;          /* volatile creates barrier */
    b = x3 - x4 * v2;
    c = (x5 ^ x6) | v3;
    d = (x7 << 2) + v4;
    e = (x8 >> 1) * v5;
    f = x9 % (v6 + 1);
    g = x10 & v7;
    
    /* More intermediate values with arithmetic */
    h = a * b + c;
    i = d - e ^ f;
    j = g & h | i;
    k = (a + b) * (c - d);
    l = (e ^ f) + (g & h);
    m = i * j - k;
    n = l ^ m + a;
    o = b * c / (d + 1);
    p = e | f ^ g;
    q = h & i | j;
    r = k + l - m;
    s = n * o % (p + 1);
    t = q ^ r | s;
    
    /* Additional values to increase pressure */
    u = t * a + b;
    v = c * d - e;
    w = f ^ g + h;
    x = i | j & k;
    y = l + m * n;
    z = o - p ^ q;
    aa = r & s | t;
    ab = u * v + w;
    ac = x - y ^ z;
    ad = aa & ab | ac;
    ae = (u + v) * (w - x);
    af = (y ^ z) + (aa & ab);
    ag = ac * ad - ae;
    ah = af ^ ag + u;
    ai = v * w / (x + 1);
    aj = y | z ^ aa;
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)a;
    short s2 = (short)b;
    short s3 = s1 + s2;  /* Promoted to int */
    int s4 = (int)s3 * c;
    
    char c1 = (char)d;
    char c2 = (char)e;
    int c3 = (int)c1 * (int)c2;
    
    /* Loop with high register pressure */
    for (int iter = 0; iter < 100; iter++) {
        /* Volatile read creates dataflow complexity */
        volatile int barrier = v8 + iter;
        
        /* Complex switch to create control flow complexity */
        switch (iter % 8) {
            case 0:
                result += a + b + barrier;
                /* Mode mixing */
                result += (short)c + (char)d;
                break;
            case 1:
                result += c * d - barrier;
                result += (int)s1 * (int)s2;
                break;
            case 2:
                result += e ^ f | barrier;
                result += c3 + s4;
                break;
            case 3:
                result += g & h + barrier;
                result += (short)i + (char)j;
                break;
            case 4:
                result += k - l * barrier;
                result += (int)s3 * m;
                break;
            case 5:
                result += m | n ^ barrier;
                result += c3 - s4;
                break;
            case 6:
                result += o & p + barrier;
                result += (short)q + (char)r;
                break;
            case 7:
                result += s * t - barrier;
                result += (int)s1 * (int)s2 + c3;
                break;
        }
        
        /* Address calculations that might be rematerialized */
        int *ptr1 = &global_array[a % 1000];
        int *ptr2 = &global_array[b % 1000];
        int *ptr3 = &global_array[c % 1000];
        int *ptr4 = &global_array[d % 1000];
        
        /* Use the pointers in computations */
        *ptr1 += u + v;
        *ptr2 += w * x;
        *ptr3 += y ^ z;
        *ptr4 += aa & ab;
        
        /* More intermediate values inside loop */
        int tmp1 = u * v + w;
        int tmp2 = x - y ^ z;
        int tmp3 = aa & ab | ac;
        int tmp4 = (u + v) * (w - x);
        int tmp5 = (y ^ z) + (aa & ab);
        
        /* Use all these temporaries */
        result += tmp1 + tmp2 + tmp3 + tmp4 + tmp5;
        
        /* Mode conversions inside loop */
        short loop_s1 = (short)tmp1;
        char loop_c1 = (char)tmp2;
        int loop_i1 = (int)loop_s1 * (int)loop_c1;
        result += loop_i1;
        
        /* Inline assembly to create complex dataflow */
        asm volatile (
            "addl %[val1], %[res]\n\t"
            "subl %[val2], %[res]\n\t"
            : [res] "+r" (result)
            : [val1] "r" (tmp3), [val2] "r" (tmp4)
            : "cc"
        );
    }
    
    /* Final computations using all variables */
    result += a + b + c + d + e + f + g + h + i + j;
    result += k + l + m + n + o + p + q + r + s + t;
    result += u + v + w + x + y + z + aa + ab + ac + ad;
    result += ae + af + ag + ah + ai + aj;
    result += s4 + c3;
    
    return result;
}

/* Another function with different patterns */
int secondary_test(int base) {
    int sum = 0;
    
    /* Create many similar computations */
    for (int i = 0; i < 50; i++) {
        /* Constants that might be rematerialized */
        const int C1 = 42;
        const int C2 = 137;
        const int C3 = 255;
        const int C4 = 1024;
        
        /* Use constants in different ways */
        int val1 = base + C1 * i;
        int val2 = base - C2 / (i + 1);
        int val3 = base ^ C3;
        int val4 = base & C4;
        
        /* Mixed width operations */
        short sval1 = (short)val1;
        short sval2 = (short)val2;
        int ival1 = (int)sval1 * (int)sval2;
        
        char cval1 = (char)val3;
        char cval2 = (char)val4;
        int ival2 = (int)cval1 + (int)cval2;
        
        sum += val1 + val2 + val3 + val4 + ival1 + ival2;
        
        /* Volatile access */
        sum += v9 * v10;
    }
    
    return sum;
}

int main() {
    /* Initialize global array */
    for (int i = 0; i < 1000; i++) {
        global_array[i] = i;
    }
    
    /* Call with many arguments to create parameter passing pressure */
    int result1 = complex_remat_test(1, 2, 3, 4, 5, 6, 7, 8, 9, 10);
    int result2 = complex_remat_test(11, 12, 13, 14, 15, 16, 17, 18, 19, 20);
    int result3 = secondary_test(100);
    int result4 = secondary_test(200);
    
    /* Use results to prevent optimization */
    volatile int final_result = result1 + result2 + result3 + result4;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
