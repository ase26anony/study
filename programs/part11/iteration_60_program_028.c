/* Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops -fno-schedule-insns -fno-schedule-insns2 -march=x86-64 -mtune=generic -fPIC test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 54321;

/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) MixedData {
    char c;
    short s;
    int i;
    long long ll;
};

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) compute_heavy(int x1, int x2, int x3, int x4, 
                                           int x5, int x6, int x7, int x8) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    long long la, lb, lc, ld;
    short sa, sb, sc, sd;
    
    /* Initial computations creating many def-use chains */
    a = x1 + x2;          /* 1 */
    b = x3 - x4;          /* 2 */
    c = a * b;            /* 3 */
    d = x5 ^ x6;          /* 4 */
    e = c | d;            /* 5 */
    f = x7 & x8;          /* 6 */
    g = e + f;            /* 7 */
    h = g << 2;           /* 8 */
    i = h >> 1;           /* 9 */
    j = i * 3;            /* 10 */
    k = j / 2;            /* 11 */
    l = k % 7;            /* 12 */
    m = l + x1;           /* 13 */
    n = m - x2;           /* 14 */
    o = n * x3;           /* 15 */
    p = o / x4;           /* 16 */
    q = p ^ x5;           /* 17 */
    r = q | x6;           /* 18 */
    s = r & x7;           /* 19 */
    t = s + x8;           /* 20 */
    
    /* More computations with different modes */
    u = (short)t * 2;     /* Mode conversion: int -> short -> int */
    v = (char)u + 65;     /* Mode conversion: int -> char -> int */
    w = v * 1000LL;       /* Mode conversion: int -> long long */
    x = (int)(w / 3);     /* Mode conversion: long long -> int */
    
    /* Use volatile to create dataflow barrier */
    if (__builtin_expect(v1 > 10000, 0)) {
        y = x + v1;
    } else {
        y = x - v1;
    }
    
    /* Complex expression with many live values */
    z = (a + b + c + d + e + f + g + h + i + j + 
         k + l + m + n + o + p + q + r + s + t) / 20;
    
    /* Mixed-type arithmetic causing mode conversions */
    la = (long long)z * 1000000000LL;
    lb = la + (long long)y;
    lc = lb / (long long)(x + 1);
    ld = lc ^ 0xAAAAAAAAAAAAAAAAULL;
    
    /* Convert back to int with truncation */
    aa = (int)ld;
    ab = (int)(ld >> 32);
    
    /* More intermediate values */
    ac = aa * ab;
    ad = ac + z;
    ae = ad - y;
    af = ae * 2;
    ag = af / 3;
    ah = ag % 5;
    ai = ah << 4;
    aj = ai >> 2;
    
    /* Switch statement creating complex control flow */
    int result = 0;
    switch (aj & 0x7) {  /* 8 cases */
        case 0:
            result = a + b + c;
            break;
        case 1:
            result = d + e + f + g;
            break;
        case 2:
            result = h + i + j + k + l;
            break;
        case 3:
            result = m + n + o + p;
            break;
        case 4:
            result = q + r + s + t + u;
            break;
        case 5:
            result = v + w + x + y + z;
            break;
        case 6:
            result = aa + ab + ac + ad;
            break;
        case 7:
            result = ae + af + ag + ah + ai + aj;
            break;
    }
    
    /* Use inline assembly to create complex dataflow */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        "imull %3, %0"
        : "+r" (result)
        : "r" (v2), "r" (v3), "r" (aj)
        : "cc"
    );
    
    return result;
}

/* Another function with address calculations that might be rematerialized */
int __attribute__((noinline)) process_array(int *arr, int size) {
    int sum = 0;
    
    /* Loop with address calculations that could be rematerialized */
    for (int i = 0; i < size; i++) {
        /* &arr[i] calculation might be rematerialized */
        int *ptr = &arr[i];
        
        /* Complex expression using the pointer */
        int val = *ptr + *(ptr + (i % 4)) - *(ptr - (i % 3 + 1));
        
        /* Use val in further computations */
        sum += val * i;
        
        /* Mode conversions */
        short sval = (short)val;
        sum += (int)sval * 2;
        
        /* Another address calculation */
        int *ptr2 = arr + (i * 2) % size;
        sum += *ptr2;
    }
    
    return sum;
}

/* Main function creating maximum register pressure */
int main(int argc, char **argv) {
    /* Initialize array for processing */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3 + argc;
    }
    
    /* Call compute_heavy many times with different arguments */
    int total = 0;
    for (int iter = 0; iter < 1000; iter++) {
        /* Create many live values across loop iterations */
        int x1 = iter * 2 + argc;
        int x2 = iter * 3 + v1;
        int x3 = iter * 5 + v2;
        int x4 = iter * 7 + v3;
        int x5 = iter * 11 + total;
        int x6 = iter * 13 + array[iter % 100];
        int x7 = iter * 17 + x1;
        int x8 = iter * 19 + x2;
        
        /* This call creates high register pressure */
        int result = compute_heavy(x1, x2, x3, x4, x5, x6, x7, x8);
        
        /* Use result in further computation to keep it live */
        total += result;
        
        /* Process array with address calculations */
        if (iter % 10 == 0) {
            total += process_array(array, 100);
        }
        
        /* Complex conditional with many live values */
        if (__builtin_expect((total & 0xFF) == 0, 0)) {
            /* Use packed structure to force sub-register modes */
            struct MixedData md;
            md.c = (char)(total & 0xFF);
            md.s = (short)(total >> 8);
            md.i = total >> 16;
            md.ll = (long long)total * 1000LL;
            
            /* Access structure members causing mode changes */
            total += (int)md.c + (int)md.s + md.i + (int)(md.ll & 0xFFFFFFFF);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0;
}
