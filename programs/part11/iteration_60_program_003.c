/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Mixed types for mode conversions */
typedef struct {
    int a : 5;
    int b : 7;
    int c : 10;
    int d : 10;
} packed_t;

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4 + v3;
    int d = c - x5 * v4;
    int e = d | x6 + v5;
    int f = e & x7 - v6;
    int g = f * x8 + v7;
    int h = g ^ x9 - v8;
    int i = h | x10 + v9;
    int j = i & x1 - v10;
    
    /* More intermediate values with different operations */
    int k = j << 2;
    int l = k >> 1;
    int m = l * 3;
    int n = m / 2;
    int o = n % 17;
    int p = o + 42;
    int q = p - 19;
    int r = q ^ 0xFF;
    int s = r | 0xAA;
    int t = s & 0x55;
    
    /* Mode mixing: use different sized operations */
    short s1 = (short)t;
    int u = (int)s1 * 2;  /* Mode conversion here */
    
    char c1 = (char)u;
    int v = (int)c1 + 100; /* Another mode conversion */
    
    /* Complex expression with many live values */
    int w = (a + b) * (c - d) + (e | f) - (g & h) + (i ^ j) + 
            (k << 1) - (l >> 2) + m * n - o / p + q % r + s - t;
    
    /* Use packed structure to force sub-register accesses */
    packed_t pt;
    pt.a = u & 0x1F;
    pt.b = v & 0x7F;
    pt.c = w & 0x3FF;
    pt.d = (u + v + w) & 0x3FF;
    
    /* More arithmetic creating long dependency chain */
    int x = pt.a * 2 + pt.b * 3 + pt.c * 4 + pt.d * 5;
    int y = x * x - x + 1;
    int z = y % 1000;
    
    /* Loop with high register pressure */
    int sum = 0;
    for (int iter = 0; iter < 100; iter++) {
        /* Many live values across loop iterations */
        int t1 = z + iter;
        int t2 = t1 * a;
        int t3 = t2 + b;
        int t4 = t3 - c;
        int t5 = t4 ^ d;
        int t6 = t5 | e;
        int t7 = t6 & f;
        int t8 = t7 * g;
        int t9 = t8 + h;
        int t10 = t9 - i;
        
        /* Switch to create complex control flow */
        switch (t10 & 0x7) {  /* 8 cases */
            case 0: sum += t1 + t2; break;
            case 1: sum += t3 - t4; break;
            case 2: sum += t5 | t6; break;
            case 3: sum += t7 & t8; break;
            case 4: sum += t9 ^ t10; break;
            case 5: sum += a * t1; break;
            case 6: sum += b + t2; break;
            case 7: sum += c - t3; break;
        }
        
        /* More mode mixing in loop */
        short stmp = (short)(sum & 0xFFFF);
        int itmp = (int)stmp * 2;
        char ctmp = (char)(itmp & 0xFF);
        sum += (int)ctmp;
        
        /* Address calculation that could be rematerialized */
        int* addr1 = &sum + iter;
        int* addr2 = addr1 + 1;
        int* addr3 = addr2 - 1;
        
        /* Use addresses in computations */
        sum += (int)(addr3 - addr1) * 2;
        
        /* Inline asm to create complex dataflow */
        asm volatile (
            "addl %1, %0\n\t"
            "subl %2, %0\n\t"
            : "+r" (sum)
            : "r" (t1), "r" (t2)
            : "cc"
        );
    }
    
    /* Final mixing of all values */
    int result = (a + b + c + d + e + f + g + h + i + j +
                  k + l + m + n + o + p + q + r + s + t +
                  u + v + w + x + y + z + sum);
    
    /* Use volatile to prevent optimization */
    if (v1) {
        result ^= 0xABCDEF;
    }
    
    return result;
}

/* Another function to increase compilation unit complexity */
int __attribute__((noinline))
secondary_computation(int base) {
    /* Different pattern of computations */
    int acc = base;
    
    for (int i = 0; i < 50; i++) {
        /* Many intermediate values */
        int t1 = acc + i;
        int t2 = t1 * 3;
        int t3 = t2 - i;
        int t4 = t3 ^ 0xFF;
        int t5 = t4 | 0xAA;
        int t6 = t5 & 0x55;
        int t7 = t6 << 2;
        int t8 = t7 >> 1;
        int t9 = t8 * 5;
        int t10 = t9 % 13;
        
        /* Mix types */
        short s1 = (short)t10;
        int t11 = (int)s1 * 2;
        char c1 = (char)(t11 & 0xFF);
        int t12 = (int)c1 + 100;
        
        acc += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11 + t12;
        
        /* Conditional to split live ranges */
        if (i & 1) {
            acc ^= 0x123456;
        } else {
            acc |= 0x789ABC;
        }
    }
    
    return acc;
}

int main(void) {
    int result = 0;
    
    /* Call with many different arguments to create varied dataflow */
    for (int outer = 0; outer < 10; outer++) {
        /* Vary inputs to prevent constant propagation */
        int arg1 = v1 + outer;
        int arg2 = v2 - outer;
        int arg3 = v3 * outer;
        int arg4 = v4 ^ outer;
        int arg5 = v5 | outer;
        int arg6 = v6 & outer;
        int arg7 = v7 + outer * 2;
        int arg8 = v8 - outer * 3;
        int arg9 = v9 ^ outer * 4;
        int arg10 = v10 | outer * 5;
        
        int res1 = compute_heavy(arg1, arg2, arg3, arg4, arg5,
                                 arg6, arg7, arg8, arg9, arg10);
        
        int res2 = secondary_computation(res1);
        
        /* Mix results in non-trivial way */
        result ^= res1;
        result += res2;
        result = (result << 3) | (result >> 29);  /* Rotate */
        
        /* Use volatile to create dataflow barrier */
        if (v1) {
            asm volatile ("" : : "r" (result) : "memory");
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
