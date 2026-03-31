/* Test case for early rematerialization coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;

/* Packed structure to force sub-register accesses */
struct __attribute__((packed)) PackedStruct {
    short a;
    int b;
    char c;
    long d;
};

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int t1 = x1 + x2 + v1;
    int t2 = x3 - x4 * v2;
    int t3 = t1 ^ t2;
    int t4 = t3 * x5 + v3;
    int t5 = t4 / (x6 + 1);
    int t6 = t5 | (x7 << 2);
    int t7 = t6 & (x8 * v4);
    int t8 = t7 + x9 - v5;
    int t9 = t8 * x10;
    int t10 = t9 ^ (x1 * x2);
    int t11 = t10 + x3 - v6;
    int t12 = t11 * x4 / (v7 + 1);
    int t13 = t12 | (x5 << 3);
    int t14 = t13 & (x6 * v8);
    int t15 = t14 + x7 - v9;
    int t16 = t15 * x8;
    int t17 = t16 ^ (x9 * x10);
    int t18 = t17 + x1 - v10;
    int t19 = t18 * x2 / (v1 + 1);
    int t20 = t19 | (x3 << 4);
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)t1;
    short s2 = (short)t2;
    int t21 = (int)s1 * (int)s2 + t3;
    
    char c1 = (char)t4;
    char c2 = (char)t5;
    int t22 = (int)c1 - (int)c2 * t6;
    
    /* Complex expression with many live values */
    int t23 = t7 + t8 - t9 * t10 / (t11 + 1) | t12 & t13 ^ t14;
    
    /* Use inline assembly to create complex dataflow */
    int asm_out1, asm_out2;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (asm_out1)
        : "r" (t15), "r" (t16)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        : "+r" (asm_out2)
        : "r" (t17)
        : "cc"
    );
    
    /* Switch statement to create complex control flow */
    int result = 0;
    switch (t18 & 0x7) {
        case 0:
            result = t1 + t2 + t3 + asm_out1;
            break;
        case 1:
            result = t4 - t5 * t6 + asm_out2;
            break;
        case 2:
            result = t7 | t8 ^ t9 & t10;
            break;
        case 3:
            result = t11 * t12 / (t13 + 1);
            break;
        case 4:
            result = t14 + t15 - t16;
            break;
        case 5:
            result = t17 & t18 | t19;
            break;
        case 6:
            result = t20 ^ t21 * t22;
            break;
        case 7:
            result = t23 + asm_out1 - asm_out2;
            break;
    }
    
    /* More mixed-type operations */
    long l1 = (long)result * (long)t1;
    short s3 = (short)(l1 >> 16);
    int t24 = (int)s3 + t2;
    
    /* Packed structure operations */
    struct PackedStruct ps;
    ps.a = (short)t3;
    ps.b = t4;
    ps.c = (char)t5;
    ps.d = (long)t6;
    
    int t25 = (int)ps.a + ps.b + (int)ps.c + (int)ps.d;
    
    /* Another sequence to maintain pressure */
    int t26 = t24 * t25 / (result + 1);
    int t27 = t26 | (t7 << 5);
    int t28 = t27 & (t8 * 0x55AA55AA);
    int t29 = t28 + t9 - t10;
    int t30 = t29 * t11;
    
    /* Use __builtin_expect to create conditional blocks */
    if (__builtin_expect((t30 & 0xFF) == 0, 0)) {
        result += t12 + t13 + t14;
    } else {
        result += t15 + t16 + t17;
    }
    
    /* Final aggregation */
    return result + t18 + t19 + t20 + t21 + t22 + t23 + t24 + t25 
           + t26 + t27 + t28 + t29 + t30;
}

/* Main function with loop to increase pressure */
int main() {
    int i, j;
    int total = 0;
    
    /* Read from volatiles to create initial live values */
    int base1 = v1, base2 = v2, base3 = v3, base4 = v4, base5 = v5;
    int base6 = v6, base7 = v7, base8 = v8, base9 = v9, base10 = v10;
    
    /* Outer loop */
    for (i = 0; i < 100; i++) {
        /* Modify base values */
        base1 += i;
        base2 -= i;
        base3 ^= i;
        base4 *= (i & 0xF) + 1;
        base5 /= (i & 0x7) + 1;
        
        /* Inner loop with heavy computation */
        for (j = 0; j < 50; j++) {
            /* Create many distinct arguments */
            int arg1 = base1 + j;
            int arg2 = base2 - j;
            int arg3 = base3 ^ j;
            int arg4 = base4 * ((j & 0x3) + 1);
            int arg5 = base5 / ((j & 0x1) + 1);
            int arg6 = base6 + (j << 1);
            int arg7 = base7 - (j >> 1);
            int arg8 = base8 ^ (j << 2);
            int arg9 = base9 * ((j & 0x7) + 1);
            int arg10 = base10 / ((j & 0x3) + 1);
            
            /* Call computation with many live values */
            int res = compute_heavy(arg1, arg2, arg3, arg4, arg5,
                                   arg6, arg7, arg8, arg9, arg10);
            
            /* Use result to prevent elimination */
            total += res;
            
            /* Address calculation that might be rematerialized */
            int* dummy_array = (int*)malloc(100 * sizeof(int));
            if (dummy_array) {
                for (int k = 0; k < 10; k++) {
                    /* Multiple uses of address calculation */
                    int idx1 = (arg1 + k) % 100;
                    int idx2 = (arg2 + k * 2) % 100;
                    int idx3 = (arg3 + k * 3) % 100;
                    
                    dummy_array[idx1] = arg4 + k;
                    dummy_array[idx2] = arg5 - k;
                    dummy_array[idx3] = arg6 ^ k;
                    
                    /* Use values to create data dependencies */
                    total += dummy_array[idx1] + dummy_array[idx2] + dummy_array[idx3];
                }
                free(dummy_array);
            }
        }
        
        /* Mode mixing at loop boundaries */
        short short_val = (short)(total & 0xFFFF);
        char char_val = (char)(total & 0xFF);
        long long_val = (long)total * (long)short_val;
        
        total = (int)(long_val >> 16) + (int)char_val;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
