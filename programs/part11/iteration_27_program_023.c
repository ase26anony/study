/* caller-save-test.c
 * Test program to trigger caller-save register spilling at basic block boundaries
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_volatile1 = 123;
volatile int g_volatile2 = 456;
volatile int g_volatile3 = 789;
volatile int g_sink = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int a, int b) {
    return a + b + g_volatile1;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    return a * b - c + g_volatile2;
}

__attribute__((noinline, noclone))
int helper3(int a, int b, int c, int d) {
    return (a + b) * (c - d) + g_volatile3;
}

/* Complex function with register pressure */
__attribute__((noinline, noclone, optimize("O2")))
int test_function(int param1, int param2, int param3) {
    /* Create many local variables to increase register pressure */
    int var1 = param1 * 2;
    int var2 = param2 + 7;
    int var3 = param3 - 3;
    int var4 = var1 + var2;
    int var5 = var2 * var3;
    int var6 = var3 / (param1 ? param1 : 1);
    int var7 = var4 ^ var5;
    int var8 = var5 | var6;
    int var9 = var6 & var7;
    int var10 = var7 + var8;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* First call site - placed at end of basic block (before return in if branch) */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp1 = reg_var * 2;
        
        /* Call at end of basic block - will be followed by return */
        int call_result = helper1(temp1, var4);
        
        /* Use reg_var after call - forces caller-save */
        reg_var = call_result + reg_var + var5;
        
        /* This return creates basic block boundary right after call */
        return reg_var + g_sink;
    }
    
    /* Second call site - in loop with basic block boundary */
    for (int i = 0; i < param2; i++) {
        /* Use reg_var before call */
        int temp2 = reg_var + i;
        
        /* Multiple variables live across call */
        int a = var6 + i;
        int b = var7 * i;
        int c = var8 - i;
        
        /* Call at end of loop body - basic block ends after call */
        int call_result = helper2(temp2, a, b);
        
        /* Use reg_var after call */
        reg_var = call_result + reg_var + c + var9;
        
        /* Loop increment creates new basic block */
        var10 += i;
    }
    
    /* Third call site - before conditional jump */
    if (param3 < 100) {
        /* Use reg_var before call */
        int temp3 = reg_var / 2;
        
        /* Call with many arguments */
        int call_result = helper3(temp3, var4, var5, var6);
        
        /* Use reg_var after call */
        reg_var = call_result - reg_var + var7;
        
        /* Jump to label creates basic block boundary */
        goto compute_result;
    } else {
        /* Alternative path */
        reg_var = helper1(reg_var, var8);
        goto compute_result;
    }
    
compute_result:
    /* Final computation using reg_var */
    result = reg_var + var10;
    
    /* One more call at end of basic block (before return) */
    int final_call = helper2(result, var9, reg_var);
    
    /* Store to volatile sink to prevent optimization */
    g_sink = final_call;
    
    return final_call;
}

/* Another test with different pattern */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Multiple register variables to increase pressure */
    register int r1 asm ("r10") = g_volatile1;
    register int r2 asm ("r11") = g_volatile2;
    register int r3 asm ("r12") = g_volatile3;
    
    int sum = 0;
    
    /* Loop with call at end of basic block */
    for (int i = 0; i < iterations; i++) {
        /* Use register variables before call */
        int a = r1 + i;
        int b = r2 * i;
        
        /* Call at end of loop body */
        int res = helper1(a, b);
        
        /* Use register variables after call */
        r1 = res + r3;
        r2 = r1 - i;
        r3 = r2 * 2;
        
        /* Accumulate result */
        sum += r1 + r2 + r3;
        
        /* Basic block boundary: loop back edge */
    }
    
    /* Final call at end of basic block (before return) */
    int final = helper3(r1, r2, r3, sum);
    
    g_sink += final;
    return final;
}

/* Test with switch statement creating multiple basic blocks */
__attribute__((noinline, noclone))
int test_function3(int mode) {
    register int reg_var asm ("r12") = g_volatile1;
    int result = 0;
    
    switch (mode % 4) {
        case 0:
            /* Call at end of basic block */
            result = helper1(reg_var, 10);
            reg_var = result * 2;
            /* Fall through to next case */
            
        case 1:
            result += helper2(reg_var, 20, 30);
            reg_var = result - 5;
            /* Break creates basic block boundary */
            break;
            
        case 2:
            result = helper3(reg_var, 40, 50, 60);
            reg_var = result / 2;
            /* Return creates basic block boundary */
            return reg_var + g_sink;
            
        case 3:
            result = helper1(reg_var, 70);
            reg_var = result + 100;
            /* Break creates basic block boundary */
            break;
    }
    
    /* Final call at end of basic block */
    int final = helper2(reg_var, result, 80);
    g_sink += final;
    return final;
}

int main() {
    int total = 0;
    
    /* Run test functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i * 2, i * 3);
        total += test_function2(i + 1);
        total += test_function3(i);
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d (sink: %d)\n", total, g_sink);
    
    return 0;
}
