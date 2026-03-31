/* Test case for early rematerialization virtual register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 0xABCDEF;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5, 
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to increase register pressure */
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
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)r;
    short s2 = (short)s;
    int i1 = (int)s1 * (int)s2;  /* Promotions to int */
    
    char c1 = (char)t;
    unsigned char uc1 = (unsigned char)u;
    int i2 = (int)c1 + (int)uc1;
    
    /* Use volatile as dataflow barrier */
    if (__builtin_expect(v3 != 0, 0)) {
        a += v1;
        b += v2;
    }
    
    /* Complex switch with different live value usage */
    int selector = (z & 0x7);  /* 0-7 */
    int result = 0;
    
    switch (selector) {
        case 0:
            result = a + b + i1;
            /* Mode mixing */
            result += (short)c * (int)d;
            break;
        case 1:
            result = c - d - i2;
            result += (char)e * (unsigned short)f;
            break;
        case 2:
            result = e ^ f ^ (int)s1;
            result += (signed char)g * (unsigned int)h;
            break;
        case 3:
            result = g | h | (int)uc1;
            result += (short)i * (long)j;
            break;
        case 4:
            result = i & j & (int)c1;
            result += (int)k * (short)l;
            break;
        case 5:
            result = k + l + (int)s2;
            result += (char)m * (unsigned char)n;
            break;
        case 6:
            result = m - n - i1;
            result += (short)o * (int)p;
            break;
        case 7:
            result = o ^ p ^ i2;
            result += (signed char)q * (unsigned short)r;
            break;
    }
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Multiple uses of address calculations */
    *ptr1 += result;
    *ptr2 += result + i1;
    *ptr3 += result - i2;
    
    /* Inline assembly to create complex dataflow */
    asm volatile (
        "addl %[val1], %[val2]\n\t"
        "subl %[val3], %[val2]\n\t"
        : [val2] "+r" (result)
        : [val1] "r" (a), [val3] "r" (b)
        : "cc"
    );
    
    /* More arithmetic to keep values live */
    int aa = result * a;
    int bb = result / (b + 1);
    int cc = aa ^ bb;
    int dd = cc | result;
    int ee = dd & aa;
    int ff = ee + bb;
    int gg = ff - cc;
    int hh = gg * dd;
    int ii = hh | ee;
    int jj = ii & ff;
    
    /* Final aggregation */
    return result + aa + bb + cc + dd + ee + ff + gg + hh + ii + jj;
}

/* Another function with loop-based register pressure */
int __attribute__((noinline))
loop_pressure(int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many live values across loop iterations */
        int v1 = i * 3;
        int v2 = i + 5;
        int v3 = v1 ^ v2;
        int v4 = v3 * 7;
        int v5 = v4 - i;
        int v6 = v5 | v1;
        int v7 = v6 & v2;
        int v8 = v7 + v3;
        int v9 = v8 - v4;
        int v10 = v9 * v5;
        int v11 = v10 ^ v6;
        int v12 = v11 + v7;
        int v13 = v12 - v8;
        int v14 = v13 * v9;
        int v15 = v14 | v10;
        int v16 = v15 & v11;
        int v17 = v16 ^ v12;
        int v18 = v17 + v13;
        int v19 = v18 - v14;
        int v20 = v19 * v15;
        
        /* Use volatile to prevent optimization */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            v1 += v3;
            v2 += v4;
            v3 += v5;
        }
        
        /* Mixed types in loop */
        short sv1 = (short)v1;
        short sv2 = (short)v2;
        int mixed = (int)sv1 * (int)sv2;
        
        /* Address calculation that might be rematerialized */
        int *ptr = &global_array[(v1 + v2) & 0xFF];
        *ptr += mixed;
        
        sum += v20 + mixed;
    }
    
    return sum;
}

/* Main function with maximum register pressure */
int main(int argc, char **argv) {
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Create many input values from arguments or computation */
    int base = argc > 1 ? atoi(argv[1]) : 1000;
    
    /* Chain computations to create long live ranges */
    int total = 0;
    
    /* First heavy computation */
    total += compute_heavy(
        base + 1, base + 2, base + 3, base + 4, base + 5,
        base + 6, base + 7, base + 8, base + 9, base + 10
    );
    
    /* Loop with pressure */
    total += loop_pressure(100);
    
    /* Another heavy computation with different values */
    total += compute_heavy(
        base + 11, base + 12, base + 13, base + 14, base + 15,
        base + 16, base + 17, base + 18, base + 19, base + 20
    );
    
    /* Use bit-fields for sub-register accesses */
    struct {
        unsigned int a : 4;
        unsigned int b : 8;
        unsigned int c : 12;
        unsigned int d : 8;
    } bits;
    
    bits.a = total & 0xF;
    bits.b = (total >> 4) & 0xFF;
    bits.c = (total >> 12) & 0xFFF;
    bits.d = (total >> 24) & 0xFF;
    
    /* Operations on bit-fields cause mode changes */
    int from_bits = bits.a + bits.b + bits.c + bits.d;
    
    /* Vector-like operations using arrays (may use vector modes) */
    int vec1[4] = {total, total + 1, total + 2, total + 3};
    int vec2[4] = {from_bits, from_bits + 1, from_bits + 2, from_bits + 3};
    int vec_result[4];
    
    for (int i = 0; i < 4; i++) {
        vec_result[i] = vec1[i] * vec2[i];
        total += vec_result[i];
    }
    
    /* Final result depends on everything */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
