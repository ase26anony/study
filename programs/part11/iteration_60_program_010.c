/* Test case for early-remat.cc lines 930-937 */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 54321;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2;
    int b = a * x3;
    int c = b - x4;
    int d = c ^ x5;
    int e = d | x6;
    int f = e & x7;
    int g = f + x8;
    int h = g - x9;
    int i = h * x10;
    int j = i ^ a;
    int k = j + b;
    int l = k - c;
    int m = l * d;
    int n = m | e;
    int o = n & f;
    int p = o ^ g;
    int q = p + h;
    int r = q - i;
    int s = r * j;
    int t = s | k;
    int u = t & l;
    int v = u ^ m;
    int w = v + n;
    int x = w - o;
    int y = x * p;
    int z = y | q;
    
    /* Mixed-type operations to create mode conversions */
    short s1 = (short)r;
    short s2 = (short)s;
    int i1 = (int)s1 * (int)s2;  /* Promotions to int */
    
    char c1 = (char)t;
    char c2 = (char)u;
    int i2 = (int)c1 + (int)c2;  /* Sign extensions */
    
    /* Use volatile as barrier */
    if (v1 > 10000) {
        a += v1;
        b += v2;
    }
    
    /* Complex switch to create control flow with different live sets */
    int selector = (z & 0x7);  /* 0-7 */
    
    switch (selector) {
        case 0:
            a = b + c;
            i1 = i1 << 2;
            break;
        case 1:
            d = e - f;
            i2 = i2 >> 1;
            break;
        case 2:
            g = h * i;
            s1 = (short)(i1 & 0xFFFF);
            break;
        case 3:
            j = k ^ l;
            s2 = (short)(i2 & 0xFFFF);
            break;
        case 4:
            m = n | o;
            c1 = (char)(i1 & 0xFF);
            break;
        case 5:
            p = q + r;
            c2 = (char)(i2 & 0xFF);
            break;
        case 6:
            s = t - u;
            /* Force address calculation rematerialization */
            int *ptr1 = &global_array[a & 0xFF];
            int *ptr2 = &global_array[b & 0xFF];
            *ptr1 = c;
            *ptr2 = d;
            break;
        case 7:
            v = w * x;
            /* More address calculations */
            int *ptr3 = &global_array[c & 0xFF];
            int *ptr4 = &global_array[d & 0xFF];
            *ptr3 = e;
            *ptr4 = f;
            break;
    }
    
    /* Use all values in final computation to keep them live */
    int result = a + b + c + d + e + f + g + h + i + j +
                 k + l + m + n + o + p + q + r + s + t +
                 u + v + w + x + y + z + i1 + i2 + s1 + s2 + c1 + c2;
    
    return result;
}

/* Second function with loop-based register pressure */
int __attribute__((noinline))
loop_pressure(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values across loop iterations */
        int t1 = i * 2;
        int t2 = t1 + v1;
        int t3 = t2 - v2;
        int t4 = t3 ^ v3;
        int t5 = t4 | i;
        int t6 = t5 & 0xFF;
        int t7 = t6 << 2;
        int t8 = t7 >> 1;
        int t9 = t8 * 3;
        int t10 = t9 + 7;
        
        /* Mixed types in loop */
        short st1 = (short)t1;
        short st2 = (short)t2;
        int mixed = (int)st1 * (int)st2;
        
        /* Address calculation that might be rematerialized */
        int idx = (t3 + t4) & 0xFF;
        int *addr = &global_array[idx];
        int loaded = *addr;
        
        /* Complex condition with many live values */
        if (__builtin_expect((t5 & 0xF) == 0, 0)) {
            t6 = t7 + t8;
            t9 = t10 * loaded;
        } else {
            t6 = t7 - t8;
            t9 = t10 / (loaded + 1);
        }
        
        /* Use inline asm to create hard register constraints */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "+r" (sum)
            : "r" (t6), "r" (t9)
            : "cc"
        );
        
        /* More arithmetic to increase pressure */
        sum += mixed;
        sum ^= t1;
        sum |= t2;
        sum &= 0xFFFFFF;
    }
    
    return sum;
}

/* Function with vector-like operations */
typedef int v4si __attribute__((vector_size(16)));
int __attribute__((noinline))
vector_ops(int a, int b, int c, int d) {
    v4si v1 = {a, b, c, d};
    v4si v2 = {b, c, d, a};
    v4si v3 = {c, d, a, b};
    
    /* Vector operations create specific RTL patterns */
    v4si r1 = v1 + v2;
    v4si r2 = v2 * v3;
    v4si r3 = r1 - r2;
    v4si r4 = r2 ^ r3;
    
    /* Extract elements to scalar */
    int sum = r4[0] + r4[1] + r4[2] + r4[3];
    
    /* Mode mixing */
    short s1 = (short)r4[0];
    short s2 = (short)r4[1];
    int ext1 = (int)s1;
    int ext2 = (int)s2;
    
    return sum + ext1 + ext2;
}

int main(void) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Create many input values from volatiles */
    int base1 = v1;
    int base2 = v2;
    int base3 = v3;
    
    /* Chain computations to create complex dataflow */
    int result1 = compute_heavy(
        base1, base2, base3, base1 ^ base2, base2 ^ base3,
        base3 ^ base1, base1 + 1, base2 + 2, base3 + 3,
        base1 + base2 + base3
    );
    
    int result2 = loop_pressure(100);
    
    int result3 = vector_ops(result1 & 0xFF, result2 & 0xFF, 
                            (result1 >> 8) & 0xFF, (result2 >> 8) & 0xFF);
    
    /* Final result to prevent optimization */
    int final_result = result1 + result2 + result3;
    
    printf("Result: %d\n", final_result);
    
    return 0;
}
