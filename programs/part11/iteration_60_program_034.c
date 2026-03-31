/* Test case for GCC early rematerialization pass - targeting lines 930-937 in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 0xABCD;
volatile short v4 = 1000;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) compute_heavy(int seed) {
    /* Declare many local variables to create register pressure */
    int x1 = seed + v1;
    int x2 = seed * v2;
    int x3 = x1 ^ x2;
    short s1 = (short)(x3 & 0xFFFF);  /* Mode conversion: int -> short */
    int x4 = x2 - x1;
    int x5 = x3 * x4;
    int x6 = x5 ^ seed;
    int x7 = x6 + v3;
    int x8 = x7 * 3;
    int x9 = x8 / 5;
    int x10 = x9 | 0xFF00;
    short s2 = (short)(x10 >> 8);     /* Another mode conversion */
    int x11 = x10 & 0xFF;
    int x12 = x11 * x7;
    int x13 = x12 ^ x8;
    int x14 = x13 + x9;
    int x15 = x14 * 11;
    int x16 = x15 - x10;
    int x17 = x16 | x11;
    int x18 = x17 ^ 0x1234;
    int x19 = x18 * 19;
    int x20 = x19 / 7;
    
    /* Use inline assembly to create complex dataflow patterns */
    int asm_out1, asm_out2;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (asm_out1)
        : "r" (x20), "r" (x19)
        : "cc"
    );
    
    asm volatile (
        "imull %1, %0\n\t"
        : "=r" (asm_out2)
        : "r" (asm_out1), "0" (x18)
        : "cc"
    );
    
    /* Complex switch statement with different live value usage */
    int switch_val = (asm_out2 & 0x7);  /* 0-7 */
    int result = 0;
    
    switch (switch_val) {
        case 0:
            result = x1 + x2 + (int)s1;  /* Mix int and short */
            break;
        case 1:
            result = x3 * x4 * (int)s2;  /* Another mode mix */
            break;
        case 2:
            result = x5 ^ x6 ^ x7;
            break;
        case 3:
            result = x8 + x9 + x10;
            break;
        case 4:
            result = x11 * x12 - x13;
            break;
        case 5:
            result = x14 | x15 | x16;
            break;
        case 6:
            result = x17 ^ x18 ^ x19;
            break;
        case 7:
            result = x20 + asm_out1 + asm_out2;
            break;
        default:
            result = seed;
    }
    
    /* More arithmetic to extend live ranges */
    int y1 = result * 2;
    int y2 = y1 + x1;
    int y3 = y2 - x2;
    int y4 = y3 ^ x3;
    int y5 = y4 | x4;
    int y6 = y5 * x5;
    int y7 = y6 / (x6 + 1);
    int y8 = y7 & x7;
    int y9 = y8 + x8;
    int y10 = y9 - x9;
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_array[x10 & 0xFF];
    int *ptr2 = &global_array[x11 & 0xFF];
    int *ptr3 = &global_array[x12 & 0xFF];
    
    /* Use the pointers in computations */
    y10 += *ptr1;
    y10 += *ptr2;
    y10 += *ptr3;
    
    /* More mode conversions */
    short s3 = (short)(y10 & 0xFFFF);
    int y11 = (int)s3 * y9;
    char c1 = (char)(y11 & 0xFF);
    int y12 = (int)c1 + y8;
    
    /* Conditional based on volatile to prevent optimization */
    if (__builtin_expect(v4 > 500, 1)) {
        y12 += v1;
    } else {
        y12 -= v2;
    }
    
    /* Final computation mixing all types */
    return y12 + result + (int)s1 + (int)s2 + (int)s3 + (int)c1;
}

/* Another function with different patterns */
int __attribute__((noinline)) compute_heavy2(int base) {
    /* Create many intermediate values in a loop */
    int sum = 0;
    
    for (int i = 0; i < 100; i++) {
        /* Many live values across loop iterations */
        int t1 = base + i;
        int t2 = t1 * 3;
        int t3 = t2 - i;
        int t4 = t3 ^ 0x55;
        int t5 = t4 | t1;
        int t6 = t5 & 0xFF;
        short st1 = (short)t6;  /* Mode conversion */
        int t7 = (int)st1 * 7;
        int t8 = t7 + t2;
        int t9 = t8 - t3;
        int t10 = t9 ^ t4;
        
        /* Use volatile as barrier */
        if (v3 != 0) {
            t10 += v3;
        }
        
        sum += t10;
        
        /* Complex expression with many operands */
        base = ((base * 1103515245 + 12345) & 0x7FFFFFFF);
    }
    
    return sum;
}

/* Main function with maximum register pressure */
int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3;
    }
    
    /* Chain computations to create long live ranges */
    int result1 = compute_heavy(seed);
    int result2 = compute_heavy2(result1);
    int result3 = compute_heavy(result2);
    int result4 = compute_heavy2(result3);
    
    /* Final mixing */
    int final_result = result1 ^ result2 ^ result3 ^ result4;
    
    /* Use all results to prevent dead code elimination */
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}
