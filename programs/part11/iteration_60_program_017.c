/* Test case for GCC early rematerialization pass
 * Targets lines 930-937 in early-remat.cc
 * Compile with: gcc -O2 -funroll-loops -fno-schedule-insns -fno-schedule-insns2 test.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Mixed types for mode conversions */
typedef struct {
    unsigned short a;
    signed short b;
    int c;
    unsigned char d;
} MixedStruct;

/* Force register pressure with many live values */
__attribute__((noinline))
unsigned long long test_rematerialization(int iterations) {
    /* Many input parameters to create initial live values */
    int x1 = v1, x2 = v2, x3 = v3, x4 = v4, x5 = v5;
    int x6 = v6, x7 = v7, x8 = v8, x9 = v9, x10 = v10;
    
    /* Mixed types for mode changes */
    short s1 = 100, s2 = 200, s3 = 300;
    unsigned short us1 = 400, us2 = 500;
    char c1 = 10, c2 = 20;
    float f1 = 1.5f, f2 = 2.5f;
    
    /* Many intermediate values - at least 30 distinct ones */
    int a1 = x1 + x2;      /* 1 */
    int a2 = a1 * x3;      /* 2 */
    int a3 = a2 - x4;      /* 3 */
    int a4 = a3 ^ x5;      /* 4 */
    int a5 = a4 | x6;      /* 5 */
    int a6 = a5 & x7;      /* 6 */
    int a7 = a6 << 2;      /* 7 */
    int a8 = a7 >> 1;      /* 8 */
    int a9 = a8 + x8;      /* 9 */
    int a10 = a9 * x9;     /* 10 */
    
    int b1 = x10 * 3;      /* 11 */
    int b2 = b1 + a1;      /* 12 */
    int b3 = b2 - a2;      /* 13 */
    int b4 = b3 * a3;      /* 14 */
    int b5 = b4 / (a4 | 1);/* 15 */
    int b6 = b5 % 17;      /* 16 */
    int b7 = b6 ^ a5;      /* 17 */
    int b8 = b7 & 0xFF;    /* 18 */
    int b9 = b8 | 0x80;    /* 19 */
    int b10 = b9 << 3;     /* 20 */
    
    /* Mode conversions */
    short s4 = (short)a6;          /* 21 - truncation */
    unsigned short us3 = (unsigned short)a7; /* 22 */
    char c3 = (char)a8;            /* 23 */
    int i1 = (int)s1;              /* 24 - extension */
    int i2 = (int)us1;             /* 25 */
    int i3 = (int)c1;              /* 26 */
    
    /* More arithmetic with mixed types */
    int c1_val = i1 + i2;          /* 27 */
    int c2_val = c1_val * i3;      /* 28 */
    int c3_val = c2_val - (int)s2; /* 29 */
    int c4_val = c3_val ^ (int)us2;/* 30 */
    
    /* Address calculations for rematerialization candidates */
    int array[100];
    unsigned long long checksum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile read creates dataflow barrier */
        int barrier = v1;
        
        /* Complex expression using many live values */
        int val1 = a1 + (barrier ? x1 : x2);
        int val2 = a2 * (i % 2 ? x3 : x4);
        int val3 = a3 - (i % 3 ? x5 : x6);
        int val4 = a4 ^ (i % 4 ? x7 : x8);
        int val5 = a5 | (i % 5 ? x9 : x10);
        
        /* Switch statement with different live value subsets */
        int selector = (val1 + val2 + val3) % 7;
        
        switch (selector) {
            case 0:
                /* Use subset 1 with mode conversions */
                array[i] = (short)val1 + (char)val2 + (int)s1;
                checksum += (unsigned short)array[i] + i1;
                break;
            case 1:
                /* Use subset 2 */
                array[i] = val3 * val4 - b1;
                checksum += (unsigned char)array[i] + i2;
                break;
            case 2:
                /* Use subset 3 with different modes */
                array[i] = (val5 & 0xFFFF) | (b2 << 16);
                checksum += (short)array[i] + i3;
                break;
            case 3:
                /* Use subset 4 */
                array[i] = a6 + b3 - c1_val;
                checksum += array[i] + c2_val;
                break;
            case 4:
                /* Use subset 5 with address calculation */
                array[i] = *(volatile int*)&array[(val4 + i) % 100];
                checksum += array[i] + c3_val;
                break;
            case 5:
                /* Use subset 6 */
                array[i] = b4 / (val1 | 1) + c4_val;
                checksum += array[i] + a7;
                break;
            default:
                /* Use all values */
                array[i] = val1 + val2 + val3 + val4 + val5 +
                          a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
                          b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10 +
                          i1 + i2 + i3 + c1_val + c2_val + c3_val + c4_val;
                checksum += array[i] + s4 + us3 + c3;
                break;
        }
        
        /* More arithmetic to extend live ranges */
        a1 = a1 + array[i % 100];
        a2 = a2 - array[(i + 1) % 100];
        a3 = a3 ^ array[(i + 2) % 100];
        a4 = a4 | array[(i + 3) % 100];
        a5 = a5 & array[(i + 4) % 100];
        
        /* Mode mixing operations */
        s1 = (short)((s1 + array[i % 100]) & 0x7FFF);
        us1 = (unsigned short)((us1 + array[(i + 5) % 100]) & 0xFFFF);
        c1 = (char)((c1 + array[(i + 6) % 100]) & 0x7F);
        
        /* Floating point to force different register class */
        f1 = f1 + 0.1f;
        f2 = f2 - 0.05f;
        
        /* Use floating point in integer context */
        if (__builtin_expect((i & 0xF) == 0, 0)) {
            b1 = b1 + (int)f1;
            b2 = b2 - (int)f2;
        }
        
        /* Inline assembly to create complex dataflow */
        asm volatile (
            "addl %[v1], %[a6]\n\t"
            "subl %[v2], %[a7]\n\t"
            : [a6] "+r" (a6), [a7] "+r" (a7)
            : [v1] "r" (v1), [v2] "r" (v2)
            : "cc"
        );
    }
    
    /* Final aggregation using all values */
    checksum += a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10;
    checksum += b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9 + b10;
    checksum += i1 + i2 + i3 + c1_val + c2_val + c3_val + c4_val;
    checksum += s1 + s2 + s3 + s4 + us1 + us2 + us3 + c1 + c2 + c3;
    checksum += (int)f1 + (int)f2;
    
    return checksum;
}

/* Another function to increase compilation unit complexity */
__attribute__((noinline))
void helper_function(MixedStruct *ms, int count) {
    for (int i = 0; i < count; i++) {
        ms[i].a = (ms[i].b + ms[i].c) & 0xFFFF;
        ms[i].d = (ms[i].a ^ ms[i].b) & 0xFF;
        
        /* Bitfield operations */
        struct {
            unsigned int f1 : 4;
            unsigned int f2 : 8;
            unsigned int f3 : 12;
            unsigned int f4 : 8;
        } bf;
        
        bf.f1 = ms[i].d & 0xF;
        bf.f2 = ms[i].a & 0xFF;
        bf.f3 = ms[i].b & 0xFFF;
        bf.f4 = ms[i].c & 0xFF;
        
        ms[i].c = bf.f1 + (bf.f2 << 4) + (bf.f3 << 12) + (bf.f4 << 24);
    }
}

int main(int argc, char **argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
    }
    
    /* Initialize mixed structure array */
    MixedStruct ms[50];
    for (int i = 0; i < 50; i++) {
        ms[i].a = i * 3;
        ms[i].b = i * 5;
        ms[i].c = i * 7;
        ms[i].d = i * 11;
    }
    
    /* Call helper to create more dataflow */
    helper_function(ms, 50);
    
    /* Main test */
    unsigned long long result = test_rematerialization(iterations);
    
    printf("Result: %llu\n", result);
    
    /* Use result to prevent dead code elimination */
    if (result == 0x12345678) {
        printf("Impossible!\n");
    }
    
    return 0;
}
