/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 123;
volatile int v2 = 456;
volatile int v3 = 789;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int complex_remat_test(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Mixed types for mode conversions */
    short s1, s2, s3, s4, s5;
    unsigned char uc1, uc2, uc3;
    float f1, f2, f3;
    
    /* Initial computations creating many live values */
    a = x1 + x2 + v1;           /* volatile creates barrier */
    b = a * x3 - x4;
    c = b ^ x5 + x6;
    d = c * 7 - x7;             /* Constant 7 may be rematerialized */
    e = d / (x8 + 1);
    f = e << 2;
    g = f | x9;
    h = g & x10;
    
    /* More computations with mode mixing */
    s1 = (short)(a + b);        /* int -> short conversion */
    s2 = (short)(c - d);
    uc1 = (unsigned char)(e & 0xFF);
    
    i = h * 3 + s1;             /* Mixing int and short */
    j = i - (int)s2 * 2;
    k = j + (int)uc1;
    
    /* Address calculation that may be rematerialized */
    int *ptr1 = &global_array[a % 256];
    int *ptr2 = &global_array[b % 256];
    int *ptr3 = &global_array[c % 256];
    
    /* Use addresses in computations */
    l = *ptr1 + k;
    m = *ptr2 - l;
    n = *ptr3 * m;
    
    /* More intermediate values */
    o = n / (v2 + 1);           /* Another volatile barrier */
    p = o << (x1 & 3);
    q = p ^ (x2 * 3);
    r = q + (x3 << 2);
    s = r - (x4 / 2);
    t = s | (x5 ^ 0x55);
    
    /* Floating point for different register class */
    f1 = (float)(t + u);
    f2 = f1 * 2.5f;
    f3 = f2 - (float)v;
    
    u = (int)f3 + t;
    v = u * 11;
    w = v / 7;
    x = w << 1;
    y = x ^ 0xAA;
    z = y & 0xFF;
    
    /* Complex switch to create control flow with many live values */
    int switch_val = (z + v3) % 8;  /* volatile v3 */
    
    switch (switch_val) {
        case 0:
            aa = a + b + c + d;
            ab = aa * e;
            ac = ab + f;
            break;
        case 1:
            aa = g + h + i;
            ab = aa - j;
            ac = ab * k;
            break;
        case 2:
            aa = l + m + n;
            ab = aa / o;
            ac = ab ^ p;
            break;
        case 3:
            aa = q + r + s;
            ab = aa & t;
            ac = ab | u;
            break;
        case 4:
            /* Mode mixing in switch case */
            s3 = (short)(v + w);
            s4 = (short)(x + y);
            aa = (int)s3 * (int)s4;
            ab = aa + z;
            ac = ab - (int)uc1;
            break;
        case 5:
            /* Floating point in switch */
            f1 = (float)(aa + ab);
            aa = (int)(f1 * 3.0f) + ac;
            ab = aa * 2;
            ac = ab / 4;
            break;
        case 6:
            /* Address recalculation */
            ptr1 = &global_array[z % 256];
            ptr2 = &global_array[(z+1) % 256];
            aa = *ptr1 + *ptr2;
            ab = aa * 3;
            ac = ab - 7;
            break;
        case 7:
        default:
            aa = v1 + v2 + v3;  /* All volatiles */
            ab = aa * 2;
            ac = ab / 3;
            break;
    }
    
    /* More computations after switch */
    ad = ac + aa + ab;
    ae = ad * 2 - 1;
    af = ae ^ 0xCC;
    ag = af & 0x3F;
    ah = ag << 2;
    ai = ah / 3;
    aj = ai | 0x80;
    
    /* Use inline asm to create complex dataflow */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r" (aj)
        : "r" (v1), "r" (v2)
        : "cc"
    );
    
    /* Final result aggregation */
    int result = aj + aa + ab + ac + ad + ae + af + ag + ah + ai;
    
    /* Use all variables to prevent elimination */
    result += s1 + s2 + s3 + s4 + uc1;
    result += (int)f1 + (int)f2 + (int)f3;
    
    return result;
}

/* Second function with different pattern */
int another_remat_case(int base) {
    /* Bit-field operations for sub-register accesses */
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
        unsigned int d : 8;
    } bits;
    
    bits.a = base & 0xF;
    bits.b = (base >> 4) & 0xFF;
    bits.c = (base >> 12) & 0xFFF;
    bits.d = (base >> 24) & 0xFF;
    
    /* Many computations with bit-field values */
    int val1 = bits.a * 3;
    int val2 = bits.b / 2;
    int val3 = bits.c + 7;
    int val4 = bits.d - 1;
    
    int sum = 0;
    for (int i = 0; i < 16; i++) {
        /* Loop creates many temporary values */
        int t1 = val1 + i;
        int t2 = val2 * i;
        int t3 = val3 - i;
        int t4 = val4 ^ i;
        
        /* Mode conversions in loop */
        short st1 = (short)t1;
        short st2 = (short)t2;
        unsigned char ut3 = (unsigned char)t3;
        
        /* Recomputation of same values */
        int t5 = t1 + t2;           /* May be rematerialized */
        int t6 = t3 * t4;
        int t7 = (int)st1 * (int)st2;
        int t8 = t5 + t6 + t7 + (int)ut3;
        
        sum += t8;
        
        /* Update values for next iteration */
        val1 = (val1 + 1) & 0xF;
        val2 = (val2 * 2) & 0xFF;
        val3 = (val3 - 1) & 0xFFF;
        val4 = (val4 ^ i) & 0xFF;
    }
    
    return sum;
}

/* Main function with multiple calls */
int main() {
    int total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Call with many different arguments to create varied dataflow */
    for (int i = 0; i < 100; i++) {
        /* Vary arguments to prevent constant propagation */
        int arg1 = i + v1;
        int arg2 = i * 2 + v2;
        int arg3 = i ^ 0x55;
        int arg4 = i * 3 - 1;
        int arg5 = i / 2 + 1;
        int arg6 = (i << 3) & 0xFF;
        int arg7 = i | 0xAA;
        int arg8 = i & 0x3F;
        int arg9 = i * 5 % 256;
        int arg10 = i + 100;
        
        int result = complex_remat_test(arg1, arg2, arg3, arg4, arg5,
                                        arg6, arg7, arg8, arg9, arg10);
        
        total += result;
        
        /* Also call the second function */
        total += another_remat_case(result & 0xFFFF);
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
