/* Test program to trigger early rematerialization with virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to create dataflow barriers */
volatile int v1 = 123;
volatile int v2 = 456;
volatile int v3 = 789;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) compute_heavy(int x1, int x2, int x3, int x4, int x5,
                                           int x6, int x7, int x8, int x9, int x10) {
    /* Many intermediate values to create register pressure */
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
    int k = j | b;
    int l = k & c;
    int m = l + d;
    int n = m - e;
    int o = n * f;
    int p = o ^ g;
    int q = p | h;
    int r = q & i;
    int s = r + j;
    int t = s - k;
    int u = t * l;
    int v = u ^ m;
    int w = v | n;
    int x = w & o;
    int y = x + p;
    int z = y - q;
    
    /* Mode conversions to trigger different register modes */
    short sa = (short)a;
    short sb = (short)b;
    short sc = (short)c;
    short sd = (short)d;
    
    /* Mixed-type arithmetic */
    long la = (long)a + (long)sa;
    long lb = (long)b + (long)sb;
    long lc = (long)c + (long)sc;
    long ld = (long)d + (long)sd;
    
    /* Complex control flow with switch */
    int selector = (z & 7); /* 8 cases */
    
    /* Use volatile to prevent optimization and create dataflow edges */
    if (v1 > 100) {
        selector = (selector + v2) & 7;
    }
    
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + sa + la;
            break;
        case 1:
            result = b + sb + lb;
            break;
        case 2:
            result = c + sc + lc;
            /* More operations to increase pressure */
            result += d + sd + ld;
            break;
        case 3:
            result = e + (int)sa + (int)la;
            /* Address calculation that might be rematerialized */
            result += global_array[a & 255];
            break;
        case 4:
            result = f + (int)sb + (int)lb;
            result += global_array[b & 255];
            break;
        case 5:
            result = g + (int)sc + (int)lc;
            result += global_array[c & 255];
            break;
        case 6:
            result = h + (int)sd + (int)ld;
            result += global_array[d & 255];
            break;
        case 7:
            result = i + a + b + c + d;
            result += global_array[e & 255];
            break;
    }
    
    /* More arithmetic to keep values live */
    if (v3 < 800) {
        result += u + v + w + x + y + z;
    }
    
    return result;
}

/* Another function with loop-based pressure */
int __attribute__((noinline)) loop_pressure(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values in loop */
        int t1 = i * 3;
        int t2 = t1 + 5;
        int t3 = t2 * 7;
        int t4 = t3 - 11;
        int t5 = t4 ^ 13;
        int t6 = t5 | 17;
        int t7 = t6 & 19;
        int t8 = t7 + 23;
        int t9 = t8 - 29;
        int t10 = t9 * 31;
        
        /* Mode mixing */
        short st1 = (short)t1;
        short st3 = (short)t3;
        short st5 = (short)t5;
        short st7 = (short)t7;
        short st9 = (short)t9;
        
        /* Use inline asm to create complex dataflow */
        asm volatile ("# Dummy asm" : : "r"(t1), "r"(t3), "r"(t5));
        
        /* Complex expression with many operands */
        sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        sum += (int)st1 + (int)st3 + (int)st5 + (int)st7 + (int)st9;
        
        /* Address calculations that might be rematerialized */
        sum += global_array[t1 & 255];
        sum += global_array[t3 & 255];
        sum += global_array[t5 & 255];
        
        /* Conditional to create control flow complexity */
        if (__builtin_expect((i & 15) == 0, 0)) {
            sum -= t2 + t4 + t6 + t8 + t10;
        }
    }
    
    return sum;
}

/* Function with packed structure for sub-register accesses */
struct __attribute__((packed)) packed_data {
    unsigned int a : 5;
    unsigned int b : 7;
    unsigned int c : 9;
    unsigned int d : 11;
};

int __attribute__((noinline)) use_packed(struct packed_data* pd, int count) {
    int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Bitfield accesses create sub-register operations */
        int val1 = pd[i].a;
        int val2 = pd[i].b;
        int val3 = pd[i].c;
        int val4 = pd[i].d;
        
        /* Operations that might need mode changes */
        result += (val1 << 16) | (val2 << 8) | val3;
        result += val4 * 3;
        
        /* More intermediate values */
        int tmp1 = val1 * 2;
        int tmp2 = val2 * 3;
        int tmp3 = val3 * 4;
        int tmp4 = val4 * 5;
        
        result += tmp1 + tmp2 + tmp3 + tmp4;
    }
    
    return result;
}

/* Main function that combines everything */
int main(int argc, char** argv) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Create packed data */
    struct packed_data pd[100];
    for (int i = 0; i < 100; i++) {
        pd[i].a = i & 31;
        pd[i].b = (i * 2) & 127;
        pd[i].c = (i * 3) & 511;
        pd[i].d = (i * 5) & 2047;
    }
    
    int result = 0;
    
    /* Call compute_heavy multiple times with different args */
    for (int i = 0; i < 10; i++) {
        result += compute_heavy(
            i + 1, i + 2, i + 3, i + 4, i + 5,
            i + 6, i + 7, i + 8, i + 9, i + 10
        );
    }
    
    /* Add loop pressure */
    result += loop_pressure(50);
    
    /* Add packed structure usage */
    result += use_packed(pd, 100);
    
    /* Final computation with many live values */
    int final = 0;
    for (int i = 0; i < 20; i++) {
        int x1 = result + i;
        int x2 = x1 * 2;
        int x3 = x2 - i;
        int x4 = x3 ^ result;
        int x5 = x4 | i;
        int x6 = x5 & result;
        int x7 = x6 + x1;
        int x8 = x7 - x2;
        int x9 = x8 * x3;
        int x10 = x9 ^ x4;
        
        /* Mode conversions */
        short sx1 = (short)x1;
        short sx3 = (short)x3;
        short sx5 = (short)x5;
        short sx7 = (short)x7;
        short sx9 = (short)x9;
        
        final += x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9 + x10;
        final += (int)sx1 + (int)sx3 + (int)sx5 + (int)sx7 + (int)sx9;
    }
    
    printf("Result: %d\n", final);
    return 0;
}
