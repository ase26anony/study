/* Test case for early rematerialization register creation */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variables to create dataflow barriers */
volatile int v1 = 12345;
volatile int v2 = 67890;
volatile int v3 = 0xABCDEF;

/* Global array to create address calculations */
int global_array[256];

/* Function with high register pressure and complex dataflow */
int __attribute__((noinline)) 
compute_heavy(int x1, int x2, int x3, int x4, int x5,
              int x6, int x7, int x8, int x9, int x10) {
    /* Many distinct intermediate values to create register pressure */
    int a = x1 + x2;
    int b = a * x3;
    int c = b - x4;
    int d = c ^ x5;
    int e = d | x6;
    int f = e & x7;
    int g = f + x8;
    int h = g - x9;
    int i = h * x10;
    int j = i ^ x1;
    int k = j | x2;
    int l = k & x3;
    int m = l + x4;
    int n = m - x5;
    int o = n * x6;
    int p = o ^ x7;
    int q = p | x8;
    int r = q & x9;
    int s = r + x10;
    int t = s - x1;
    
    /* Mixed-type operations to create mode changes */
    short s1 = (short)(t & 0xFFFF);
    short s2 = (short)((t >> 16) & 0xFFFF);
    int u = (int)s1 * (int)s2;  /* Mode conversion here */
    
    /* Use volatile to prevent optimization and create barriers */
    if (v1 > 10000) {
        u += v2;
    }
    
    /* Complex switch to create control flow complexity */
    switch (u & 0x7) {  /* 8 cases */
        case 0:
            u = u * a + b;
            break;
        case 1:
            u = u * c - d;
            break;
        case 2:
            u = (u & e) | f;
            break;
        case 3:
            u = (u ^ g) + h;
            break;
        case 4:
            u = u * i - j;
            /* Mixed types again */
            u = (short)u * (int)(u >> 16);
            break;
        case 5:
            u = u | k & l;
            break;
        case 6:
            u = u + m - n;
            break;
        case 7:
            u = u * o ^ p;
            break;
    }
    
    /* More computations to extend live ranges */
    int v = u + q;
    int w = v - r;
    int x = w * s;
    int y = x ^ t;
    int z = y | u;
    
    /* Address calculations that might be rematerialized */
    int *ptr1 = &global_array[a & 0xFF];
    int *ptr2 = &global_array[b & 0xFF];
    int *ptr3 = &global_array[c & 0xFF];
    
    /* Use the pointers in computations */
    z += *ptr1;
    z += *ptr2;
    z += *ptr3;
    
    /* Inline assembly to create complex dataflow */
    asm volatile (
        "addl %1, %0\n\t"
        "subl %2, %0\n\t"
        : "+r" (z)
        : "r" (v), "r" (w)
        : "cc"
    );
    
    /* Another volatile barrier */
    if (v3 != 0) {
        z ^= v3;
    }
    
    return z;
}

/* Second function with different pattern */
int __attribute__((noinline))
compute_heavy2(int base) {
    int sum = base;
    
    /* Loop with many live values */
    for (int i = 0; i < 100; i++) {
        /* Many intermediate values in loop */
        int t1 = sum + i;
        int t2 = t1 * 3;
        int t3 = t2 - 17;
        int t4 = t3 ^ 0x55AA;
        int t5 = t4 | 0xFF00;
        int t6 = t5 & 0x0F0F;
        int t7 = t6 + 42;
        int t8 = t7 - i;
        int t9 = t8 * 7;
        int t10 = t9 ^ t1;
        
        /* Mixed width operations */
        short st1 = (short)(t10 & 0xFFFF);
        char c1 = (char)(t10 & 0xFF);
        int t11 = (int)st1 * (int)c1;  /* Mode mixing */
        
        /* Conditional that uses many values */
        if (__builtin_expect((t11 & 1) == 0, 0)) {
            sum += t1 + t3 + t5 + t7 + t9;
        } else {
            sum += t2 + t4 + t6 + t8 + t10;
        }
        
        /* Switch inside loop for more complexity */
        switch (sum & 3) {
            case 0: sum = sum * 2 + t11; break;
            case 1: sum = (sum << 1) | (t11 & 1); break;
            case 2: sum = sum ^ t11; break;
            case 3: sum = sum - t11; break;
        }
    }
    
    return sum;
}

/* Main function that creates maximum pressure */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i * 3 + 1;
    }
    
    /* Call compute_heavy with many different arguments
       to create diverse dataflow patterns */
    for (int iter = 0; iter < 50; iter++) {
        /* Vary arguments to prevent constant propagation */
        int arg1 = v1 + iter;
        int arg2 = v2 - iter;
        int arg3 = iter * 3;
        int arg4 = iter * 5 + 1;
        int arg5 = iter * 7 - 2;
        int arg6 = iter * 11 + 3;
        int arg7 = iter * 13 - 4;
        int arg8 = iter * 17 + 5;
        int arg9 = iter * 19 - 6;
        int arg10 = iter * 23 + 7;
        
        result ^= compute_heavy(arg1, arg2, arg3, arg4, arg5,
                               arg6, arg7, arg8, arg9, arg10);
        
        /* Also call the second function */
        result += compute_heavy2(iter);
        
        /* Use volatile to prevent dead code elimination */
        if (v3 > 0) {
            result &= 0x7FFFFFFF;
        }
    }
    
    printf("Result: %d\n", result);
    return 0;
}
