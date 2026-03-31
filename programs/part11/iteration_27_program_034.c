/* caller-save-test.c
 * Test program to trigger caller-save register spills at basic block boundaries
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;
volatile int global_data[256];

/* Force calls to be actual function calls */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    global_counter++;
    return a + b + global_counter;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    global_data[global_counter % 256] = a + b + c;
    return global_data[(global_counter - 1) % 256];
}

__attribute__((noinline, noclone))
int helper3(int a) {
    return a * 2 + global_counter;
}

/* Main test function - designed to force caller-save spills at BB boundaries */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int var1 = param1 * 2;
    int var2 = param2 + 5;
    int var3 = param3 - 3;
    int var4 = var1 * var2;
    int var5 = var2 + var3;
    int var6 = var3 * var4;
    int var7 = var4 - var5;
    int var8 = var5 * var6;
    int var9 = var6 + var7;
    int var10 = var7 * var8;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* Scenario 1: Call at end of if-branch basic block */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call at the end of basic block */
        result = helper1(temp, var4);
        
        /* This creates a BB boundary - the if block ends here */
        /* reg_var needs to be saved/restored across helper1 call */
    } else {
        /* Different path to create control flow */
        result = var5;
    }
    
    /* Use reg_var after call */
    reg_var += result;
    
    /* Scenario 2: Call at end of loop body basic block */
    for (int i = 0; i < 3; i++) {
        /* More register pressure inside loop */
        int loop_var1 = reg_var + i;
        int loop_var2 = var8 * i;
        int loop_var3 = var9 - i;
        
        /* Function call as last statement in loop body */
        int call_result = helper2(loop_var1, loop_var2, loop_var3);
        
        /* Loop body ends here, creating BB boundary before i++ */
        /* reg_var needs preservation across helper2 call */
        
        reg_var += call_result;
    }
    
    /* Scenario 3: Call just before return (end of function BB) */
    if (param2 < 100) {
        int final_compute = reg_var * 3 + var10;
        
        /* Call at the end of basic block before return */
        int final_result = helper3(final_compute);
        
        /* Store to volatile to prevent optimization */
        global_sink = final_result;
        
        return final_result;
    }
    
    /* Alternative return path */
    global_sink = reg_var;
    return reg_var + var10;
}

/* Additional test to increase register pressure */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Even more variables to increase register pressure */
    int a = iterations;
    int b = a * 2;
    int c = b + 5;
    int d = c - 3;
    int e = d * 2;
    int f = e + 7;
    int g = f - 1;
    int h = g * 3;
    int i = h + 2;
    int j = i - 4;
    
    /* Another register variable */
    register int reg_var2 asm ("r13") = a + b + c + d;
    
    /* Complex control flow with calls at BB ends */
    for (int k = 0; k < iterations; k++) {
        if (k % 2 == 0) {
            int temp = reg_var2 + k;
            
            /* Call at end of if-block */
            int res = helper1(temp, j);
            
            /* BB boundary here */
            reg_var2 = res - h;
        } else {
            int temp = reg_var2 - k;
            
            /* Different call at end of else-block */
            int res = helper2(temp, g, f);
            
            /* BB boundary here */
            reg_var2 = res + e;
        }
        
        /* Another call in loop body */
        if (k % 3 == 0) {
            int res = helper3(reg_var2);
            
            /* BB boundary */
            j += res;
        }
    }
    
    return reg_var2 + j;
}

int main() {
    int total = 0;
    
    /* Call test functions with different parameters
     * to exercise different control flow paths */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i * 10, i * 20);
        total += test_function2(i + 1);
        
        /* Mix in some direct helper calls to vary call patterns */
        if (i % 3 == 0) {
            total += helper1(i, total);
        }
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d (global_sink: %d)\n", total, global_sink);
    
    return 0;
}
