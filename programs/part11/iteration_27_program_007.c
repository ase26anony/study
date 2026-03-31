/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller-save-test.c -o caller-save-test
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
int helper3(int a) {
    return a ^ 0xABCD;
}

/* Main test function - also noinline to preserve structure */
__attribute__((noinline, noclone))
int test_caller_save(int param1, int param2, int param3) {
    /* Create register pressure with many variables */
    int v1 = param1 * 2;
    int v2 = param2 + 7;
    int v3 = param3 - 3;
    int v4 = v1 ^ v2;
    int v5 = v2 | v3;
    int v6 = v3 & v1;
    int v7 = v4 + v5;
    int v8 = v5 - v6;
    int v9 = v6 * v7;
    int v10 = v7 / (v8 ? v8 : 1);
    
    /* Explicit register variable bound to a call-clobbered register (r12 on x86_64) */
    register int reg_var asm ("r12") = v1 + v2 + v3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* First basic block: call at end before conditional branch */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call at the end of basic block (before implicit jump to block merge) */
        result = helper1(temp, v4);
        
        /* This creates a basic block boundary after the call */
        if (param2 > 0) {
            /* Another use of reg_var - forces it to be live across the call */
            global_sink = reg_var + result;
            result += helper2(reg_var, v5, v6);
        }
    } else {
        /* Different path to create control flow complexity */
        result = helper3(v7);
    }
    
    /* Loop to create another basic block boundary after a call */
    for (int i = 0; i < 3; i++) {
        /* Use reg_var in computation */
        int loop_val = reg_var + i;
        
        /* Call at the end of loop body (before loop increment/jump) */
        int call_result = helper2(loop_val, v8, v9);
        
        /* Store result - ensures reg_var must be preserved across call */
        global_sink = reg_var + call_result;
        
        /* Loop increment creates new basic block */
    }
    
    /* Call right before return - creates basic block end */
    result += helper1(reg_var, v10);
    
    /* Use reg_var one more time */
    global_sink = reg_var;
    
    return result;
}

/* Another test with different control flow */
__attribute__((noinline, noclone))
int test_caller_save2(int x) {
    /* Multiple register variables to increase pressure */
    register int r1 asm ("r12") = x * 3;
    register int r2 asm ("r13") = x + 11;
    
    int a = x * 2;
    int b = x + 5;
    int c = x - 3;
    int d = a ^ b;
    int e = b | c;
    int f = c & a;
    
    /* Switch statement creates multiple basic blocks */
    switch (x % 4) {
        case 0:
            /* Call at end of case before break */
            a = helper1(r1, a);
            /* Break creates basic block boundary */
            break;
        case 1:
            b = helper2(r2, b, c);
            break;
        case 2:
            /* Nested call sequence */
            c = helper3(r1);
            /* Another call - r1 must be preserved */
            d = helper1(r1, d);
            break;
        default:
            e = helper2(r1, e, f);
            /* Call as last statement before break */
            f = helper3(r2);
            break;
    }
    
    /* Use register variables after switch */
    global_sink = r1 + r2;
    
    /* Call at end of function before return */
    return helper1(r1, a + b + c + d + e + f);
}

int main() {
    int total = 0;
    
    /* Test multiple paths to exercise different basic block structures */
    for (int i = 0; i < 10; i++) {
        total += test_caller_save(i, i * 2, i * 3);
        total += test_caller_save2(i);
    }
    
    /* Also test edge cases */
    total += test_caller_save(-1, 0, 1);
    total += test_caller_save(100, -50, 25);
    
    printf("Result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
