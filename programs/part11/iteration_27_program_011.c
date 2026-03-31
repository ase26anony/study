/* Test program to trigger caller-save register spills at basic block boundaries */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic test.c */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_counter = 0;
volatile int g_sink1 = 0;
volatile int g_sink2 = 0;
volatile int g_sink3 = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int a, int b) {
    g_counter++;
    return a + b + g_counter;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    g_counter += 2;
    return a * b + c - g_counter;
}

__attribute__((noinline, noclone))
int helper3(int a, int b, int c, int d) {
    g_counter += 3;
    return (a + b) * (c - d) + g_counter;
}

/* Force register pressure and specific register allocation */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create many local variables to increase register pressure */
    int var1 = param1 * 2;
    int var2 = param2 + 7;
    int var3 = param3 - 3;
    int var4 = var1 + var2;
    int var5 = var2 * var3;
    int var6 = var3 / (param1 ? param1 : 1);
    int var7 = var4 - var5;
    int var8 = var5 + var6;
    int var9 = var6 * var7;
    int var10 = var7 - var8;
    
    /* Explicit register variable bound to a call-clobbered register (r12 on x86_64) */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* First call site - at end of basic block (before return in if branch) */
    if (param1 > 0) {
        /* reg_var is used before call */
        int temp = reg_var * 2;
        
        /* Function call at end of basic block */
        result = helper1(temp, var4);
        
        /* This return creates basic block boundary right after call */
        g_sink1 = reg_var;  /* Force reg_var to be live after call */
        return result;
    }
    
    /* Second call site - at end of loop body */
    for (int i = 0; i < param2; i++) {
        /* Update reg_var in loop */
        reg_var += i * 3;
        
        /* Use reg_var before call */
        int temp2 = reg_var - var5;
        
        /* Function call at end of loop body */
        result += helper2(temp2, var6, i);
        
        /* Loop increment/jump creates basic block boundary */
        g_sink2 = reg_var;  /* Force reg_var to be live after call */
    }
    
    /* Third call site - complex control flow */
    switch (param3 % 3) {
        case 0: {
            int temp3 = reg_var + var7;
            /* Call at end of case block */
            result += helper3(temp3, var8, var9, var10);
            /* Break creates basic block boundary */
            g_sink3 = reg_var;
            break;
        }
        case 1: {
            int temp4 = reg_var - var8;
            /* Another call at end of case block */
            result += helper1(temp4, var9);
            /* Break creates basic block boundary */
            g_sink3 = reg_var * 2;
            break;
        }
        default: {
            int temp5 = reg_var * var9;
            /* Call before return */
            result += helper2(temp5, var10, param1);
            /* Return creates basic block boundary */
            g_sink3 = reg_var / 2;
            return result;
        }
    }
    
    /* Final use of reg_var */
    result += reg_var;
    return result;
}

/* Another test with different register pressure pattern */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Even more variables to increase pressure */
    int a = iterations;
    int b = a * 3;
    int c = b + 7;
    int d = c - 2;
    int e = d * 5;
    int f = e / 3;
    int g = f + 11;
    int h = g - 4;
    int i = h * 2;
    int j = i + 9;
    
    /* Multiple register variables */
    register int reg1 asm ("r12") = a + b + c;
    register int reg2 asm ("r13") = d + e + f;
    
    int sum = 0;
    
    /* Loop with calls at different positions */
    for (int k = 0; k < iterations; k++) {
        /* Update both register variables */
        reg1 += k * 2;
        reg2 -= k;
        
        /* Call at end of loop body when k is even */
        if (k % 2 == 0) {
            sum += helper1(reg1, reg2);
            /* Force both registers to be live across call */
            g_sink1 = reg1;
            g_sink2 = reg2;
        } else {
            /* Different call when k is odd */
            sum += helper2(reg1, g, h);
            g_sink1 = reg1 * 2;
        }
        
        /* More computations to keep registers busy */
        reg1 = (reg1 + reg2) * 3;
        reg2 = (reg2 - a) / 2;
    }
    
    /* Final call at end of function */
    sum += helper3(reg1, reg2, i, j);
    return sum;
}

int main() {
    int total = 0;
    
    /* Call test functions with different parameters to exercise different paths */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i * 2, i * 3);
        total += test_function2(i + 1);
        
        /* Mix in some direct calls to increase caller-save opportunities */
        total += helper1(i, total);
        total += helper2(i, total, g_counter);
    }
    
    printf("Result: %d\n", total);
    printf("Sink values: %d, %d, %d\n", g_sink1, g_sink2, g_sink3);
    
    return 0;
}
