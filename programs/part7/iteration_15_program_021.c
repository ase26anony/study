/* reload_coverage.c - Program to trigger GCC's reload pass initialization */
#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_seed1 = 12345;
volatile int global_seed2 = 67890;
volatile int global_seed3 = 24680;

/* Prevent inlining and IPA optimizations */
__attribute__((noinline, noipa, noicf))
int create_reload_pressure(int input1, int input2, int input3) {
    /* Declare many local variables to exhaust registers */
    int v1 = input1 + 1;
    int v2 = input2 - 1;
    int v3 = v1 * v2;
    short v4 = (short)(input3 & 0xFFFF);
    char v5 = (char)(input1 ^ input2);
    long v6 = (long)v1 * (long)v2;
    int v7 = v3 + v1;
    int v8 = v2 << 2;
    short v9 = v4 | 0x55;
    char v10 = v5 + 1;
    long v11 = v6 >> 1;
    int v12 = v7 ^ v8;
    int v13 = v12 + v3;
    short v14 = v9 & 0xAA;
    char v15 = v10 * 3;
    long v16 = v11 - v6;
    int v17 = v13 | v7;
    int v18 = v8 + v12;
    short v19 = v14 ^ v9;
    char v20 = v15 - v10;
    long v21 = v16 * 2;
    int v22 = v17 & v18;
    int v23 = v13 - v22;
    short v24 = v19 + v14;
    char v25 = v20 << 1;
    long v26 = v21 / 3;
    int v27 = v23 ^ v17;
    int v28 = v18 * v22;
    short v29 = v24 | v19;
    char v30 = v25 ^ v20;
    
    /* Complex expressions creating register pressure */
    v1 = (v2 & v3) | (v4 << (v5 & 3));
    v6 = (long)v7 * (long)v8 - (long)v9;
    v10 = (char)((v11 >> 2) & 0xFF);
    v12 = v13 * v14 - v15 + v16;
    v17 = (v18 ^ v19) | (v20 << 4);
    v21 = v22 * v23 / (v24 + 1);
    v25 = (char)((v26 & 0xFF) ^ v27);
    v28 = v29 * v30 + v1 - v2;
    
    /* More operations to keep values live */
    v3 = v4 * v5 + v6 - v7;
    v8 = v9 | v10 & v11;
    v12 = v13 << (v14 & 3);
    v15 = (char)(v16 ^ v17 ^ v18);
    v19 = v20 + v21 - v22;
    v23 = v24 * v25 / (v26 + 1);
    v27 = v28 ^ v29 ^ v30;
    
    /* Inline assembly to create register constraints */
    asm volatile("" : "+r"(v1), "+r"(v2), "+r"(v3) : : "cc", "memory");
    asm volatile("" : "+m"(v4), "+r"(v5) : : "cc");
    
    /* Complex addressing mode simulation */
    int* ptr1 = &v6;
    int* ptr2 = &v7;
    int* ptr3 = &v8;
    
    /* Force memory accesses */
    v9 = *ptr1 + *ptr2;
    v10 = (char)(*ptr3 & 0xFF);
    
    /* More arithmetic with many operands */
    v11 = (long)(v1 * v2 + v3 * v4 - v5 * v6 + v7 * v8 - v9 * v10);
    v12 = (v13 & v14) | (v15 << v16) | (v17 >> v18);
    v19 = v20 * v21 + v22 * v23 - v24 * v25 + v26 * v27 - v28 * v29;
    
    /* Create spill/reload opportunities with multiple uses */
    v30 = (char)((v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10) & 0xFF);
    int temp1 = v11 + v12;
    int temp2 = v13 + v14;
    int temp3 = v15 + v16;
    int temp4 = v17 + v18;
    
    v19 = temp1 * temp2 - temp3 * temp4;
    v20 = (char)((temp1 ^ temp2 ^ temp3 ^ temp4) & 0xFF);
    
    /* Final aggregation to prevent dead code elimination */
    int result = v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
    result ^= v11 ^ v12 ^ v13 ^ v14 ^ v15 ^ v16 ^ v17 ^ v18 ^ v19 ^ v20;
    result ^= v21 ^ v22 ^ v23 ^ v24 ^ v25 ^ v26 ^ v27 ^ v28 ^ v29 ^ v30;
    result ^= temp1 ^ temp2 ^ temp3 ^ temp4;
    
    return result & 0xFFFF;
}

/* Another high-pressure function with different patterns */
__attribute__((noinline, noipa))
int secondary_pressure(int base) {
    /* Different variable types and patterns */
    unsigned int u1 = base * 3;
    unsigned short u2 = base * 5;
    unsigned char u3 = base * 7;
    signed int s1 = -base;
    signed short s2 = -base * 2;
    signed char s3 = -base * 3;
    
    /* Mixed type operations forcing conversions */
    u1 = u1 + (unsigned int)u2 + (unsigned int)u3;
    s1 = s1 - (signed int)s2 - (signed int)s3;
    
    /* Bit manipulation operations */
    u2 = (u2 << 3) | (u2 >> 13);
    s2 = (s2 ^ 0x5555) & 0x7FFF;
    
    /* More variables */
    int a = u1 + s1;
    int b = u2 * s2;
    int c = u3 ^ s3;
    int d = a * b;
    int e = b / (c + 1);
    int f = d - e;
    int g = f << 2;
    int h = g >> 1;
    int i = h | a;
    int j = i & b;
    int k = j ^ c;
    int l = k + d;
    int m = l - e;
    int n = m * f;
    int o = n / (g + 1);
    int p = o ^ h;
    int q = p | i;
    int r = q & j;
    int s = r ^ k;
    int t = s + l;
    
    /* Force some spills with inline asm */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d) : : "cc");
    asm volatile("" : "+r"(e), "+r"(f), "+r"(g), "+r"(h) : : "cc");
    
    return (a + b + c + d + e + f + g + h + i + j + 
            k + l + m + n + o + p + q + r + s + t) & 0xFFFF;
}

int main(int argc, char *argv[]) {
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 1) iterations = 1;
    if (iterations > 1000) iterations = 1000;
    
    int total_result = 0;
    
    /* Loop to ensure function is called multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use volatile globals and argc to prevent constant folding */
        int input1 = global_seed1 + argc + i;
        int input2 = global_seed2 - argc + i * 2;
        int input3 = global_seed3 ^ argc ^ i;
        
        /* Call high-pressure functions */
        int result1 = create_reload_pressure(input1, input2, input3);
        int result2 = secondary_pressure(input1 + input2);
        
        total_result ^= result1;
        total_result += result2;
        
        /* Modify globals slightly */
        global_seed1 ^= result1;
        global_seed2 += result2;
        global_seed3 = global_seed3 * 3 + 1;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: 0x%04x\n", total_result & 0xFFFF);
    
    return (total_result & 0xFF);
}
