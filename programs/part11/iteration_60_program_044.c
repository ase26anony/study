/* Test program to trigger early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20, vs3 = 30;

/* Global array to create address calculations */
int global_array[1000];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4 + v3;
    int d = c - x5 * v4;
    int e = d | x6 + v5;
    int f = e & x7 - v1;
    int g = f * x8 + v2;
    int h = g ^ x9 - v3;
    int i = h | x10 + v4;
    int j = i & x1 - v5;
    int k = j * x2 + v1;
    int l = k ^ x3 - v2;
    int m = l | x4 + v3;
    int n = m & x5 - v4;
    int o = n * x6 + v5;
    int p = o ^ x7 - v1;
    int q = p | x8 + v2;
    int r = q & x9 - v3;
    int s = r * x10 + v4;
    int t = s ^ x1 - v5;
    
    /* Mixed-type operations to create mode conversions */
    short sa = (short)a + vs1;
    short sb = (short)b - vs2;
    short sc = (short)c * vs3;
    short sd = (short)d + vs1;
    short se = (short)e - vs2;
    
    /* More intermediate values using mixed types */
    int u = (int)sa * t + v1;
    int v = (int)sb ^ u - v2;
    int w = (int)sc | v + v3;
    int x = (int)sd & w - v4;
    int y = (int)se * x + v5;
    int z = y ^ t - v1;
    
    /* Complex control flow with switch to create dataflow complexity */
    int result = 0;
    switch (z & 7) {
        case 0:
            result = a + b + (int)sa;
            break;
        case 1:
            result = c - d + (int)sb;
            break;
        case 2:
            result = e * f + (int)sc;
            break;
        case 3:
            result = g ^ h + (int)sd;
            break;
        case 4:
            result = i | j + (int)se;
            break;
        case 5:
            result = k & l + u;
            break;
        case 6:
            result = m - n + v;
            break;
        case 7:
            result = o * p + w;
            break;
    }
    
    return result;
}

/* Function with inline assembly to create complex dataflow */
int __attribute__((noinline))
asm_constraints(int x, int y) {
    int a, b, c;
    
    /* Inline assembly with multiple constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r"(a)
        : "r"(x), "r"(y)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        : "+r"(a)
        : "r"(v1)
        : "cc"
    );
    
    return a;
}

/* Main function with loops creating register pressure */
int main(void) {
    int i, j;
    int total = 0;
    
    /* Read from globals to create initial live values */
    int base1 = v1 + v2 + v3;
    int base2 = v4 + v5 + vs1;
    int base3 = vs2 + vs3;
    
    /* Unrolled loop with many live values */
    for (i = 0; i < 100; i++) {
        /* Address calculations that are rematerialization candidates */
        int *ptr1 = &global_array[i];
        int *ptr2 = &global_array[i + 10];
        int *ptr3 = &global_array[i + 20];
        
        /* Use the pointers in independent calculations */
        int val1 = *ptr1 + base1;
        int val2 = *ptr2 * base2;
        int val3 = *ptr3 ^ base3;
        
        /* Many intermediate calculations */
        int t1 = val1 + val2;
        int t2 = val2 - val3;
        int t3 = val3 * val1;
        int t4 = t1 ^ t2;
        int t5 = t2 | t3;
        int t6 = t3 & t4;
        int t7 = t4 - t5;
        int t8 = t5 * t6;
        int t9 = t6 ^ t7;
        int t10 = t7 | t8;
        int t11 = t8 & t9;
        int t12 = t9 - t10;
        int t13 = t10 * t11;
        int t14 = t11 ^ t12;
        int t15 = t12 | t13;
        int t16 = t13 & t14;
        int t17 = t14 - t15;
        int t18 = t15 * t16;
        int t19 = t16 ^ t17;
        int t20 = t17 | t18;
        
        /* Mixed-type operations */
        short st1 = (short)t1 + (short)vs1;
        short st2 = (short)t2 - (short)vs2;
        short st3 = (short)t3 * (short)vs3;
        
        /* Use inline assembly to create dataflow complexity */
        int asm_res = asm_constraints(t4, t5);
        
        /* Complex function call with many arguments */
        int heavy_res = compute_heavy(t6, t7, t8, t9, t10,
                                      t11, t12, t13, t14, t15);
        
        /* Switch based on computed value */
        switch ((t16 + asm_res + heavy_res) & 3) {
            case 0:
                total += t17 + (int)st1;
                break;
            case 1:
                total += t18 - (int)st2;
                break;
            case 2:
                total += t19 * (int)st3;
                break;
            case 3:
                total += t20 ^ asm_res;
                break;
        }
        
        /* Volatile condition as dataflow barrier */
        if (__builtin_expect(v1 > 0, 1)) {
            total += heavy_res;
        }
    }
    
    /* Additional loop with different pattern */
    for (j = 0; j < 50; j++) {
        /* Bit-field like operations */
        struct {
            int a : 5;
            int b : 7;
            int c : 10;
            int d : 10;
        } bits;
        
        bits.a = (j & 0x1F);
        bits.b = ((j >> 5) & 0x7F);
        bits.c = ((j >> 12) & 0x3FF);
        bits.d = ((j >> 22) & 0x3FF);
        
        /* Operations on bit-fields cause mode changes */
        int bf1 = bits.a * bits.b;
        int bf2 = bits.c + bits.d;
        int bf3 = bf1 ^ bf2;
        int bf4 = bf2 - bf1;
        
        total += bf3 + bf4;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
