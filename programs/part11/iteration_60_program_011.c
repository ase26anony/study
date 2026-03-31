/* Program to trigger early rematerialization in GCC's RTL optimizer */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -funroll-loops -fPIC -march=x86-64 this_file.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;

/* Global arrays to create address calculations */
int global_arr[256];
short global_short_arr[512];
char global_char_arr[1024];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5, 
              int x6, int x7, int x8, int x9, int x10) {
    /* Many local variables to create register pressure */
    int a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t;
    int u, v, w, x, y, z, aa, ab, ac, ad, ae, af, ag, ah, ai, aj;
    
    /* Complex arithmetic chain creating many def-use relationships */
    a = x1 + x2 + v1;          /* volatile creates barrier */
    b = a * x3 - v2;
    c = b ^ x4;
    d = c + (x5 << 2);
    e = d | x6;
    f = e & x7;
    g = f - x8;
    h = g * x9;
    i = h / (x10 + 1);
    
    /* More computations with mode mixing */
    short sa = (short)a;       /* Mode conversion: int -> short */
    short sb = (short)b;
    j = (int)sa * (int)sb;     /* Mode conversion: short -> int */
    
    k = j + (x1 & 0xFF);
    l = k - (x2 | 0x7F);
    m = l * (x3 ^ 0x55);
    n = m / (x4 + 256);
    
    /* Use address calculations that are rematerialization candidates */
    int *ptr1 = &global_arr[a & 0xFF];
    int *ptr2 = &global_arr[b & 0xFF];
    int *ptr3 = &global_arr[c & 0xFF];
    
    o = *ptr1 + *ptr2 + *ptr3;
    
    /* More computations with different modes */
    char ca = (char)o;
    char cb = (char)n;
    p = (int)ca * (int)cb * 100;
    
    q = p + (x5 % 17);
    r = q - (x6 % 23);
    s = r * (x7 % 29);
    t = s / (x8 % 31);
    
    /* Complex switch to create control flow complexity */
    int switch_val = t & 0x7;
    switch (switch_val) {
        case 0:
            u = a + b + c;
            v = u * d;
            w = v ^ e;
            break;
        case 1:
            u = f + g + h;
            v = u * i;
            w = v ^ j;
            break;
        case 2:
            u = k + l + m;
            v = u * n;
            w = v ^ o;
            break;
        case 3:
            u = p + q + r;
            v = u * s;
            w = v ^ t;
            break;
        case 4:
            u = (short)a + (short)b;  /* Mode mixing */
            v = u * (short)c;
            w = v ^ (short)d;
            break;
        case 5:
            u = (char)e + (char)f;    /* More mode mixing */
            v = u * (char)g;
            w = v ^ (char)h;
            break;
        case 6:
            u = i + j + k;
            v = u * l;
            w = v ^ m;
            break;
        case 7:
            u = n + o + p;
            v = u * q;
            w = v ^ r;
            break;
    }
    
    /* More computations using the switch results */
    x = w + u + v;
    y = x * (a ^ b);
    z = y / (c + 1);
    
    /* Additional computations to increase register pressure */
    aa = z + (d & e);
    ab = aa * (f | g);
    ac = ab - (h ^ i);
    ad = ac + (j & k);
    ae = ad * (l | m);
    af = ae - (n ^ o);
    ag = af + (p & q);
    ah = ag * (r | s);
    ai = ah - (t ^ u);
    aj = ai + (v & w);
    
    /* Use inline assembly to create complex dataflow patterns */
    asm volatile (
        "addl %[in1], %[out1]\n\t"
        "subl %[in2], %[out2]\n\t"
        : [out1] "+r" (x), [out2] "+r" (y)
        : [in1] "r" (z), [in2] "r" (aa)
        : "cc"
    );
    
    /* Final computation using all values */
    int result = (x + y + z + aa + ab + ac + ad + ae + af + ag + ah + ai + aj) 
                 ^ (a + b + c + d + e + f + g + h + i + j)
                 ^ (k + l + m + n + o + p + q + r + s + t)
                 ^ (u + v + w);
    
    return result;
}

/* Another function with loop-based register pressure */
int __attribute__((noinline))
loop_compute(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values within the loop */
        int t1 = i * 3;
        int t2 = i + 5;
        int t3 = i ^ 0xAA;
        int t4 = i & 0x55;
        int t5 = i | 0x33;
        int t6 = i << 2;
        int t7 = i >> 1;
        int t8 = i % 17;
        int t9 = i * i;
        int t10 = i + i;
        
        /* Use different modes */
        short st1 = (short)t1;
        short st2 = (short)t2;
        char ct3 = (char)t3;
        char ct4 = (char)t4;
        
        /* Complex expression with many intermediates */
        int r1 = t1 + t2;
        int r2 = r1 * t3;
        int r3 = r2 - t4;
        int r4 = r3 ^ t5;
        int r5 = r4 | t6;
        int r6 = r5 & t7;
        int r7 = r6 + t8;
        int r8 = r7 * t9;
        int r9 = r8 / (t10 + 1);
        
        /* Mode conversions */
        int r10 = (int)st1 * (int)st2;
        int r11 = (int)ct3 + (int)ct4;
        
        /* Address calculations (rematerialization candidates) */
        short *sptr = &global_short_arr[i & 0x1FF];
        char *cptr = &global_char_arr[i & 0x3FF];
        
        int r12 = *sptr + *cptr;
        
        /* Conditional to create control flow */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            r9 = r9 * 2;
            r10 = r10 + 100;
        }
        
        /* Use volatile to prevent optimization */
        if (v3 > 0) {
            r11 = r11 * v4;
        }
        
        sum += r9 + r10 + r11 + r12;
        
        /* Rotate values to keep them live */
        t1 = t2;
        t2 = t3;
        t3 = t4;
        t4 = t5;
        t5 = t6;
    }
    
    return sum;
}

/* Main function that creates maximum register pressure */
int main(int argc, char **argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_arr[i] = i * 3;
    }
    for (int i = 0; i < 512; i++) {
        global_short_arr[i] = (short)(i * 5);
    }
    for (int i = 0; i < 1024; i++) {
        global_char_arr[i] = (char)(i * 7);
    }
    
    /* Get some input values */
    int input1 = argc > 1 ? atoi(argv[1]) : 12345;
    int input2 = argc > 2 ? atoi(argv[2]) : 67890;
    int input3 = argc > 3 ? atoi(argv[3]) : 11111;
    int input4 = argc > 4 ? atoi(argv[4]) : 22222;
    int input5 = argc > 5 ? atoi(argv[5]) : 33333;
    int input6 = argc > 6 ? atoi(argv[6]) : 44444;
    int input7 = argc > 7 ? atoi(argv[7]) : 55555;
    int input8 = argc > 8 ? atoi(argv[8]) : 66666;
    int input9 = argc > 9 ? atoi(argv[9]) : 77777;
    int input10 = argc > 10 ? atoi(argv[10]) : 88888;
    
    /* Call compute-heavy function multiple times */
    int total = 0;
    for (int i = 0; i < 100; i++) {
        int result = compute_heavy(
            input1 + i, input2 - i, input3 * i, input4 ^ i, input5 & i,
            input6 | i, input7 + i * 2, input8 - i * 3, input9 ^ i * 4, 
            input10 & i * 5
        );
        total += result;
        
        /* Also call loop compute */
        int loop_result = loop_compute(50 + (i % 10));
        total += loop_result;
    }
    
    printf("Result: %d\n", total);
    return total & 0xFF;
}
