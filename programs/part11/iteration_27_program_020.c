/* caller-save-test.c
 * Designed to trigger caller-save register spills at basic block boundaries
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 42;

/* Noinline helper functions to ensure real calls */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    return a + b + global_source;
}

__attribute__((noinline, noclone)) 
int helper2(int a, int b, int c) {
    return a * b - c;
}

__attribute__((noinline, noclone))
void helper3(int *ptr) {
    *ptr += global_source;
}

/* Main test function - also noinline to preserve structure */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int v1 = param1 * 2;
    int v2 = param2 + 7;
    int v3 = param3 - 3;
    int v4 = v1 * v2;
    int v5 = v2 / (param3 ? param3 : 1);
    int v6 = v3 + v4;
    int v7 = v5 * 3;
    int v8 = v6 - v7;
    int v9 = v8 + param1;
    int v10 = v9 * 2;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2 + v3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* Scenario 1: Call at end of if-branch basic block */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call at the end of basic block (before return) */
        result = helper1(temp, v4);
        
        /* This return creates a basic block boundary right after the call */
        return result + reg_var;  /* reg_var must be live across helper1 call */
    }
    
    /* More register pressure */
    int v11 = v10 + param2;
    int v12 = v11 * 3;
    int v13 = v12 - param3;
    
    /* Update reg_var to ensure it's used */
    reg_var += v13;
    
    /* Scenario 2: Call at end of loop body basic block */
    for (int i = 0; i < 3; i++) {
        int loop_temp = reg_var + i;
        
        /* Function call as last statement in loop body */
        result += helper2(loop_temp, v5, v6);
        
        /* Loop increment/jump creates new basic block after call */
        /* reg_var must be preserved across helper2 call */
    }
    
    /* Scenario 3: Call before return in else branch */
    if (param2 < 0) {
        v13 = helper1(reg_var, v7);
        global_sink = v13;
    } else {
        /* Call as last action before return */
        helper3(&reg_var);
        /* Return creates basic block boundary */
        return reg_var + result;
    }
    
    /* Final use of reg_var */
    global_sink = reg_var;
    return result + v13;
}

/* Another test with different control flow */
__attribute__((noinline, noclone))
int test_function2(int x) {
    /* Bind to another call-clobbered register */
    register int reg_var2 asm ("r13") = x * 2;
    
    /* Multiple calls in different basic blocks */
    for (int i = 0; i < 4; i++) {
        if (i % 2 == 0) {
            /* Call in if-branch, basic block ends with call */
            reg_var2 += helper1(reg_var2, i);
            /* Jump to loop end creates boundary */
        } else {
            /* Different call in else branch */
            reg_var2 -= helper2(reg_var2, i, x);
            /* Basic block ends with call */
        }
        
        /* Force spill by using many temporaries */
        int t1 = reg_var2 * 3;
        int t2 = t1 + i;
        int t3 = t2 - x;
        int t4 = t3 * 2;
        int t5 = t4 / (x ? x : 1);
        reg_var2 = t5;
    }
    
    /* Call at end before return */
    return helper1(reg_var2, x);
}

/* Complex test with nested control flow */
__attribute__((noinline, noclone))
int test_function3(int a, int b) {
    register int r1 asm ("r12") = a;
    register int r2 asm ("r13") = b;
    register int r3 asm ("r14") = a + b;
    
    int sum = 0;
    
    /* Switch with calls at case ends */
    switch (a % 4) {
        case 0:
            sum = helper1(r1, r2);
            r3 += sum;
            /* Fall through creates interesting boundary */
        case 1:
            sum += helper2(r2, r3, a);
            /* Break creates boundary after call */
            break;
        case 2:
            sum = helper1(r3, a);
            /* Return creates boundary */
            return sum + r1;
        default:
            sum = helper2(r1, r2, r3);
            r1 = sum;
            /* Basic block ends with call to external function */
            printf("Default case: %d\n", sum);
    }
    
    /* Loop with call at end of body */
    while (r1 > 0) {
        int temp = helper1(r2, r3);
        r1 -= temp;
        /* Loop condition check creates new basic block */
    }
    
    return r1 + r2 + r3;
}

int main() {
    int total = 0;
    
    /* Call test functions multiple times with different parameters
     * to exercise different paths and increase register pressure */
    for (int i = 0; i < 100; i++) {
        total += test_function(i, i * 2, i * 3);
        total += test_function2(i);
        total += test_function3(i, i + 1);
        
        /* Modify global to prevent optimization */
        global_source = (global_source * 13 + 17) % 100;
    }
    
    /* Use result to prevent dead code elimination */
    global_sink = total;
    printf("Result: %d\n", total % 1000);
    
    return 0;
}
