/* Test case for GCC early rematerialization pass
 * Targeting lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to prevent optimization and create dataflow barriers */
volatile int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
volatile short vs1 = 10, vs2 = 20, vs3 = 30;

/* Global array to create address calculations */
int global_arr[100] = {0};

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
high_pressure_function(int x1, int x2, int x3, int x4, int x5,
                       int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2 + v1;
    int b = a * x3 - v2;
    int c = b ^ x4 + v3;
    int d = c - x5 * v4;
    int e = d | x6 & v5;
    int f = e + x7 - v1;
    int g = f * x8 / (v2 + 1);
    int h = g ^ x9 + v3;
    int i = h - x10 * v4;
    int j = i | x1 & v5;
    int k = j + x2 - v1;
    int l = k * x3 / (v2 + 1);
    int m = l ^ x4 + v3;
    int n = m - x5 * v4;
    int o = n | x6 & v5;
    int p = o + x7 - v1;
    int q = p * x8 / (v2 + 1);
    int r = q ^ x9 + v3;
    int s = r - x10 * v4;
    int t = s | x1 & v5;
    
    /* Mixed-type operations to trigger mode changes */
    short sa = (short)(a & 0xFFFF);
    short sb = (short)(b & 0xFFFF);
    short sc = (short)(c & 0xFFFF);
    short sd = (short)(d & 0xFFFF);
    
    /* More intermediate values with different types */
    int u = (int)sa * vs1;
    int v = (int)sb * vs2;
    int w = (int)sc * vs3;
    int y = (int)sd * vs1;
    
    /* Complex expression with many live values */
    int z = (a + b + c + d + e + f + g + h + i + j + 
             k + l + m + n + o + p + q + r + s + t +
             u + v + w + y) & 0xFFF;
    
    /* Switch statement to create complex control flow */
    int result = 0;
    switch (z & 7) {
        case 0:
            result = a + b + (int)sa;
            break;
        case 1:
            result = c + d + (int)sb;
            break;
        case 2:
            result = e + f + (int)sc;
            break;
        case 3:
            result = g + h + (int)sd;
            break;
        case 4:
            result = i + j + u;
            break;
        case 5:
            result = k + l + v;
            break;
        case 6:
            result = m + n + w;
            break;
        case 7:
            result = o + p + y;
            break;
    }
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_arr[z % 100];
    int *ptr2 = &global_arr[(z + 1) % 100];
    int *ptr3 = &global_arr[(z + 2) % 100];
    
    /* Use the pointers in computations */
    *ptr1 = result + a;
    *ptr2 = result + b;
    *ptr3 = result + c;
    
    /* Inline assembly to create complex dataflow */
    int asm_out1, asm_out2;
    asm volatile (
        "movl %1, %0\n\t"
        "addl %2, %0\n\t"
        : "=r" (asm_out1)
        : "r" (result), "r" (a)
        : "cc"
    );
    
    asm volatile (
        "movl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "=r" (asm_out2)
        : "r" (result), "r" (b)
        : "cc"
    );
    
    /* More arithmetic to keep values live */
    int final1 = asm_out1 * d + e;
    int final2 = asm_out2 * f + g;
    
    /* Use volatile in condition to create barrier */
    if (__builtin_expect(v1 > 0, 1)) {
        final1 += h + i + j;
    } else {
        final2 += k + l + m;
    }
    
    /* Another complex expression */
    return final1 * final2 + n + o + p + q + r + s + t;
}

/* Second function with different pattern */
int __attribute__((noinline))
another_high_pressure(int base) {
    /* Create many similar computations */
    int sum = 0;
    for (int i = 0; i < 50; i++) {
        /* Many intermediate values in loop */
        int t1 = base + i + v1;
        int t2 = t1 * i - v2;
        int t3 = t2 ^ base + v3;
        int t4 = t3 - i * v4;
        int t5 = t4 | base & v5;
        
        /* Mixed types */
        short st1 = (short)(t1 & 0xFF);
        short st2 = (short)(t2 & 0xFF);
        
        /* Mode conversions */
        int it1 = (int)st1 * vs1;
        int it2 = (int)st2 * vs2;
        
        /* Address calculation */
        int idx = (t1 + t2 + t3 + t4 + t5) % 100;
        int *ptr = &global_arr[idx];
        
        /* Complex update */
        sum += t1 + t2 + t3 + t4 + t5 + it1 + it2 + *ptr;
        
        /* Volatile access */
        if (__builtin_expect(v1 > 0, 1)) {
            sum += v2;
        }
    }
    return sum;
}

/* Main function with maximum register pressure */
int main() {
    int total = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        global_arr[i] = i;
    }
    
    /* Call high-pressure functions multiple times */
    for (int iter = 0; iter < 1000; iter++) {
        /* Pass many arguments to increase register pressure */
        int result1 = high_pressure_function(
            iter, iter+1, iter+2, iter+3, iter+4,
            iter+5, iter+6, iter+7, iter+8, iter+9
        );
        
        int result2 = another_high_pressure(iter);
        
        /* Complex computation with results */
        total += result1 * 31 + result2 * 17;
        
        /* Use volatile to prevent dead code elimination */
        if (__builtin_expect(v1 > 0, 1)) {
            total ^= v2;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
