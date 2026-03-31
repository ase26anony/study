/* caller-save-test.c
 * Test program to trigger caller-save register spills at basic block boundaries
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller-save-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 42;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int a, int b) {
    return a + b + global_source;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    return a * b - c;
}

__attribute__((noinline, noclone))
void helper3(void) {
    /* Empty function just to create a call site */
    global_sink++;
}

/* Main test function - also marked noinline to preserve structure */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create register pressure with many variables */
    int v1 = param1 * 2;
    int v2 = param2 + 7;
    int v3 = param3 - 3;
    int v4 = v1 * v2;
    int v5 = v2 / (param3 ? param3 : 1);
    int v6 = v3 + v4;
    int v7 = v4 - v5;
    int v8 = v5 * v6;
    int v9 = v6 + v7;
    int v10 = v7 * v8;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12");
    reg_var = v1 + v2 + v3 + global_source;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* Scenario 1: Call at end of if-block before return */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call at the end of basic block */
        result = helper1(temp, v4);
        
        /* This return creates a basic block boundary right after the call */
        return result + reg_var;  /* reg_var must be live here */
    }
    
    /* Scenario 2: Call at end of loop body */
    for (int i = 0; i < param2; i++) {
        /* Compute something with reg_var */
        int loop_temp = reg_var + i;
        
        /* Call at the end of loop body basic block */
        result += helper2(loop_temp, v5, i);
        
        /* Loop increment/jump creates new basic block */
        reg_var += result % 10;  /* Modify reg_var to force spill/restore */
    }
    
    /* Scenario 3: Call before conditional return */
    if (param3 < 0) {
        int temp2 = reg_var - v6;
        result = helper1(temp2, v7);
        
        /* Basic block ends with call, then jumps to return */
        global_sink = result;
        return reg_var;  /* reg_var live across call */
    }
    
    /* Scenario 4: Multiple calls in sequence with reg_var live */
    int sum = reg_var;
    for (int j = 0; j < 3; j++) {
        sum += helper1(reg_var + j, v8);
        helper3();  /* Another call that clobbers registers */
        
        /* Force spill by using many temporaries */
        int t1 = sum * 2;
        int t2 = t1 + v9;
        int t3 = t2 - v10;
        sum = t3;
    }
    
    /* Final use of reg_var */
    result = helper2(reg_var, sum, global_sink);
    
    return result;
}

/* Another test with different control flow */
__attribute__((noinline, noclone))
int test_function2(int x) {
    /* Bind to another call-clobbered register */
    register int reg_var2 asm ("r13") = x * 3;
    
    /* Complex expression to increase register pressure */
    int a = x + 1;
    int b = x * 2;
    int c = x - 5;
    int d = a * b;
    int e = b + c;
    int f = c * d;
    int g = d - e;
    int h = e * f;
    
    /* Switch statement creates multiple basic blocks */
    switch (x % 4) {
        case 0:
            reg_var2 += helper1(a, b);
            /* Fall through - call at end of case */
        case 1:
            reg_var2 *= helper2(c, d, e);
            break;
        case 2:
            /* Call as last statement before break */
            reg_var2 = helper1(reg_var2, f);
            break;
        default:
            reg_var2 -= helper2(g, h, x);
            /* Call then immediate return */
            return helper1(reg_var2, 100);
    }
    
    /* Use reg_var2 after switch */
    return reg_var2 + helper2(a, b, c);
}

int main(void) {
    int total = 0;
    
    /* Call test functions with different parameters to exercise different paths */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i * 2, i - 5);
        total += test_function2(i);
        
        /* Modify global to affect future calls */
        global_source = (global_source * 13 + 7) % 100;
    }
    
    /* Use volatile sink to prevent dead code elimination */
    global_sink = total;
    
    printf("Result: %d\n", total);
    return 0;
}
