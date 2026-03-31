/* caller-save-test.c
 * Test program to trigger caller-save register spills at basic block boundaries
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_volatile1 = 12345;
volatile int g_volatile2 = 67890;
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
    return (a + b) * (c - d);
}

/* Main test function with register pressure */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int var1 = param1 * 2;
    int var2 = param2 + 100;
    int var3 = param3 - 50;
    int var4 = var1 + var2;
    int var5 = var2 * var3;
    int var6 = var3 / 2;
    int var7 = var4 + var5;
    int var8 = var5 - var6;
    int var9 = var6 * var7;
    int var10 = var7 + var8;
    
    /* Explicit register variable bound to a call-clobbered register */
    /* Using r12 on x86_64 which is call-clobbered */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result1 = 0;
    int result2 = 0;
    
    /* Create basic block boundary scenarios */
    
    /* Scenario 1: Call at end of if-branch before return */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Call at what could be end of basic block */
        result1 = helper1(temp, var4);
        
        /* This return creates a basic block boundary */
        /* BB_END would point to the call instruction */
        return result1 + reg_var;  /* reg_var must be live here */
    }
    
    /* Scenario 2: Call in loop with boundary at loop end */
    for (int i = 0; i < 3; i++) {
        /* Modify reg_var in loop */
        reg_var += i;
        
        /* Multiple variables to increase register pressure */
        int a = var4 + i;
        int b = var5 * i;
        int c = var6 - i;
        int d = var7 + i * 2;
        
        /* Call that could be at end of loop body basic block */
        result2 = helper2(a, b, c);
        
        /* Use reg_var after call - forces save/restore */
        reg_var += result2;
        
        /* Loop increment/jump creates basic block boundary */
        /* BB_END of loop body would point to the call if it's last */
    }
    
    /* Scenario 3: Nested condition with calls at block ends */
    int final_result = 0;
    if (param2 < 0) {
        /* More register pressure */
        int x1 = var8 * 3;
        int x2 = var9 / 2;
        int x3 = var10 + 100;
        
        /* Call at potential block end */
        int tmp = helper3(x1, x2, x3, reg_var);
        
        /* Use reg_var after call */
        final_result = tmp + reg_var;
        
        /* Jump to return creates boundary */
    } else {
        /* Alternative path with different call */
        int y1 = var9 * 2;
        int y2 = var10 - 50;
        
        /* Another call at block end */
        int tmp = helper1(y1, y2);
        
        /* reg_var must be preserved across this call too */
        final_result = tmp - reg_var;
        
        /* Fall through to return */
    }
    
    /* Force use of reg_var at the end */
    g_sink = reg_var;
    
    return final_result + reg_var;
}

/* Another test with different pattern */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Bind to another call-clobbered register (r13 on x86_64) */
    register int counter asm ("r13") = iterations * 2;
    
    int sum = 0;
    
    /* Loop where call is at end of body */
    for (int i = 0; i < iterations; i++) {
        /* Lots of variables for register pressure */
        int a = i * 3;
        int b = i + 100;
        int c = counter - i;
        int d = a * b;
        int e = b + c;
        int f = c * d;
        int g = d + e;
        int h = e * f;
        
        /* Call at what could be end of basic block */
        int partial = helper2(a, b, c);
        
        /* counter must be live across the call */
        counter += partial;
        
        sum += counter;
        
        /* Loop back edge creates basic block boundary */
    }
    
    /* Final call at block end before return */
    int final = helper1(sum, counter);
    
    /* Return creates another boundary */
    return final + counter;
}

int main() {
    int total = 0;
    
    /* Run test with different parameters to exercise different paths */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i * 2, i * 3);
        total += test_function2(i + 1);
        
        /* Modify volatiles to affect helper functions */
        g_volatile1 += i;
        g_volatile2 -= i;
    }
    
    printf("Result: %d\n", total);
    printf("Sink value: %d\n", g_sink);
    
    return 0;
}
