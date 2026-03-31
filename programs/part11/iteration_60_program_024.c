/* Test case for early rematerialization pass coverage */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 0xABCDEF;

/* Global array to create address calculations */
int global_array[256];

/* Complex function with high register pressure */
int __attribute__((noinline)) 
compute_heavy_function(int p1, int p2, int p3, int p4, int p5,
                       int p6, int p7, int p8, int p9, int p10)
{
    /* Many local variables to create register pressure */
    int a1 = p1 + p2;
    int a2 = p3 - p4;
    int a3 = p5 * p6;
    int a4 = p7 ^ p8;
    int a5 = p9 | p10;
    
    /* Force rematerialization of constants */
    int b1 = a1 + 42;           /* Constant 42 may be rematerialized */
    int b2 = a2 - 314;
    int b3 = a3 * 2718;
    int b4 = a4 ^ 0xDEADBEEF;
    int b5 = a5 & 0xCAFEBABE;
    
    /* More intermediate values */
    int c1 = b1 * b2;
    int c2 = b3 + b4;
    int c3 = b5 - b1;
    int c4 = b2 ^ b3;
    int c5 = b4 | b5;
    
    /* Mixed-type operations to trigger mode changes */
    short s1 = (short)c1;       /* Truncation to smaller mode */
    short s2 = (short)c2;
    int d1 = (int)s1 * c3;      /* Extension back to int */
    int d2 = (int)s2 + c4;
    
    /* Complex expression chain */
    int e1 = d1 * d2;
    int e2 = e1 ^ c5;
    int e3 = e2 - d1;
    int e4 = e3 | d2;
    int e5 = e4 & e1;
    
    /* Use volatile to prevent optimization and create dataflow edges */
    if (__builtin_expect(v1 > 10000, 0)) {
        e1 += v2;
    } else {
        e2 -= v3;
    }
    
    /* Switch statement to create complex control flow */
    int selector = e5 & 0x7;  /* 0-7 */
    int result = 0;
    
    switch (selector) {
        case 0:
            result = e1 + e2 + e3;
            /* Use address calculation that may be rematerialized */
            global_array[0] = result;
            break;
        case 1:
            result = e2 - e3 - e4;
            global_array[1] = result;
            break;
        case 2:
            result = e3 * e4 * e5;
            global_array[2] = result;
            break;
        case 3:
            result = e4 ^ e5 ^ e1;
            global_array[3] = result;
            break;
        case 4:
            result = e5 | e1 | e2;
            global_array[4] = result;
            break;
        case 5:
            /* Mixed modes again */
            result = (short)e1 * (short)e2 + (int)e3;
            global_array[5] = result;
            break;
        case 6:
            result = e1 + e3 + e5;
            global_array[6] = result;
            break;
        case 7:
            result = e2 * e4 * 42;  /* Rematerialize constant 42 */
            global_array[7] = result;
            break;
    }
    
    /* More arithmetic to increase register pressure */
    int f1 = result + a1;
    int f2 = f1 * a2;
    int f3 = f2 - a3;
    int f4 = f3 ^ a4;
    int f5 = f4 | a5;
    
    int g1 = f5 + b1;
    int g2 = g1 * b2;
    int g3 = g2 - b3;
    int g4 = g3 ^ b4;
    int g5 = g4 | b5;
    
    int h1 = g5 + c1;
    int h2 = h1 * c2;
    int h3 = h2 - c3;
    int h4 = h3 ^ c4;
    int h5 = h4 | c5;
    
    /* Final computation using all values */
    int final = h5 + d1 + d2 + e1 + e2 + e3 + e4 + e5 + f1 + f2 + f3 + f4 + f5;
    
    return final;
}

/* Another function with inline asm to create complex dataflow */
int __attribute__((noinline))
asm_dataflow(int x, int y)
{
    int result1, result2;
    
    /* Inline asm with multiple outputs creates complex DF */
    asm volatile (
        "movl %2, %0\n\t"
        "addl %3, %0\n\t"
        "movl %0, %1\n\t"
        "imull %2, %1"
        : "=r"(result1), "=r"(result2)
        : "r"(x), "r"(y)
        : "cc"
    );
    
    return result1 + result2;
}

/* Main function with loop to maximize pressure */
int main(void)
{
    int total = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    /* Loop to create sustained register pressure */
    for (int iter = 0; iter < 1000; iter++) {
        /* Vary inputs to prevent constant propagation */
        int base = iter ^ v1;
        
        /* Call compute-heavy function with many arguments */
        int res = compute_heavy_function(
            base + 1, base + 2, base + 3, base + 4, base + 5,
            base + 6, base + 7, base + 8, base + 9, base + 10
        );
        
        /* Mix in asm_dataflow for complex DF patterns */
        int asm_res = asm_dataflow(res, iter);
        
        /* Use bit-fields for sub-register accesses */
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } bf;
        
        bf.low = res & 0xFFFF;
        bf.high = (res >> 16) & 0xFFFF;
        
        /* More mixed-mode operations */
        int mixed = (short)bf.low * (int)bf.high;
        
        /* Address calculations that may be rematerialized */
        int idx = (mixed + iter) & 0xFF;
        int addr_calc = global_array[idx] + global_array[idx ^ 0xFF];
        
        /* Complex expression with many live values */
        total += res + asm_res + mixed + addr_calc;
        
        /* Volatile access acts as dataflow barrier */
        if (__builtin_expect(v3 != 0, 1)) {
            total ^= v2;
        }
    }
    
    /* Use packed structure for more mode complexity */
    struct __attribute__((packed)) {
        char a;
        short b;
        int c;
    } packed = {1, 2, total};
    
    int final_result = packed.a + packed.b + packed.c;
    
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
