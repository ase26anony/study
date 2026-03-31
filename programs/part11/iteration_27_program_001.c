/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g1 = 1;
volatile int g2 = 2;
volatile int g3 = 3;
volatile int g4 = 4;
volatile int sink = 0;

/* Noinline helper functions to ensure real calls */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    return a + b + g1;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    return a * b - c + g2;
}

__attribute__((noinline, noclone))
int helper3(int a, int b, int c, int d) {
    return (a + b) * (c - d) + g3;
}

/* Main test function - also noinline to preserve structure */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int v1 = param1 * 2;
    int v2 = param2 + 5;
    int v3 = param3 - 3;
    int v4 = v1 + v2;
    int v5 = v2 * v3;
    int v6 = v4 - v5;
    int v7 = v6 + g4;
    
    /* Explicit register variable bound to a call-clobbered register */
    /* Using r12 on x86_64 which is call-clobbered */
    register int reg_var asm ("r12") = v7 * 2;
    
    int result = 0;
    
    /* First call site - reg_var is live across this call */
    /* This call is at the end of a basic block (before return in if branch) */
    if (param1 > 0) {
        /* Multiple uses of reg_var to keep it live */
        int temp = reg_var + v1;
        temp = helper1(temp, v2);
        
        /* Call at what could be end of basic block */
        /* reg_var must be preserved across this call */
        int call_result = helper2(reg_var, v3, temp);
        
        /* Use reg_var immediately after call */
        result = call_result + reg_var + v4;
        
        /* This return creates a basic block boundary right after the call */
        return result;
    } else {
        /* Alternative path with different call pattern */
        int temp2 = reg_var - v5;
        
        /* Another call with reg_var live across it */
        int call_result2 = helper3(reg_var, v6, temp2, v7);
        
        /* Force reg_var to be used after call */
        result = call_result2 * reg_var;
    }
    
    /* Second scenario: call at end of loop body */
    for (int i = 0; i < 3; i++) {
        /* Compute something using reg_var */
        int loop_temp = reg_var + i + v1;
        
        /* Call at end of loop body - basic block ends after call */
        int loop_call = helper1(loop_temp, reg_var);
        
        /* Use result and reg_var */
        result += loop_call * reg_var;
        
        /* Loop increment/jump creates new basic block */
    }
    
    /* Third scenario: call before return (tail of block) */
    if (param2 < 0) {
        int final_temp = reg_var * 2;
        int final_call = helper2(final_temp, reg_var, v2);
        
        /* Call is last instruction before return */
        result = final_call + reg_var;
        return result;
    }
    
    /* Final use of reg_var */
    sink = reg_var;
    return result + reg_var;
}

/* Another test with switch statement for more block boundaries */
__attribute__((noinline, noclone))
int test_function2(int mode) {
    /* Use different call-clobbered register */
    register int reg_var2 asm ("r13") = g1 * 10;
    
    int result = 0;
    
    /* Switch creates multiple basic blocks */
    switch (mode % 4) {
        case 0: {
            int temp = reg_var2 + 5;
            /* Call at end of case block */
            result = helper1(temp, reg_var2);
            /* Fall through creates block boundary */
            /* break; intentionally omitted */
        }
        case 1: {
            int temp2 = reg_var2 * 2;
            /* Another call */
            result += helper2(temp2, reg_var2, g2);
            break;
        }
        case 2: {
            /* Call as last thing before break */
            result = helper3(reg_var2, g3, g1, g2);
            break;
        }
        default: {
            /* Multiple calls in sequence */
            int t1 = helper1(reg_var2, 1);
            int t2 = helper2(reg_var2, 2, t1);
            result = helper3(reg_var2, t1, t2, 3);
        }
    }
    
    sink = reg_var2;
    return result;
}

int main() {
    int total = 0;
    
    /* Exercise different paths */
    for (int i = -5; i <= 5; i++) {
        total += test_function(i, i * 2, i * 3);
        total += test_function2(i);
    }
    
    /* Also test edge cases */
    total += test_function(100, -100, 0);
    total += test_function2(99);
    
    printf("Result: %d (sink=%d)\n", total, sink);
    return 0;
}
