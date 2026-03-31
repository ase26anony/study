/* Test program to trigger caller-save register spills at basic block boundaries */
#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g1 = 1;
volatile int g2 = 2;
volatile int g3 = 3;
volatile int sink = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) int helper1(int a, int b) {
    return a + b + g1;
}

__attribute__((noinline, noclone)) int helper2(int a, int b, int c) {
    return a * b - c + g2;
}

__attribute__((noinline, noclone)) int helper3(int a) {
    return a ^ g3;
}

/* Main test function with register pressure */
__attribute__((noinline, noclone, optimize("O2"))) 
int test_function(int param1, int param2) {
    /* Create register pressure with many local variables */
    int v1 = param1 * 2;
    int v2 = param2 + 5;
    int v3 = v1 - v2;
    int v4 = v2 * 3;
    int v5 = v3 + v4;
    int v6 = v4 - v1;
    int v7 = v5 * v6;
    int v8 = v7 / (param1 + 1);
    int v9 = v8 << 2;
    int v10 = v9 | 0xFF;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm("r12") = v1 + v2 + v3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* Case 1: Call at end of if-block before return */
    if (param1 > 10) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call at what will be end of basic block */
        result = helper1(temp, v4);
        
        /* This return creates a basic block boundary right after the call */
        return result + reg_var;  /* reg_var must be preserved across helper1 call */
    }
    
    /* Case 2: Call as last statement in loop body */
    for (int i = 0; i < 3; i++) {
        /* Modify reg_var in loop */
        reg_var += i;
        
        /* Multiple variables to increase register pressure */
        int a = v4 + i;
        int b = v5 * i;
        int c = v6 - i;
        int d = v7 ^ i;
        int e = v8 | i;
        int f = v9 & i;
        
        /* Call at end of loop body - will be end of basic block */
        result += helper2(a, b, c);
        
        /* Loop increment/jump creates new basic block */
    }
    
    /* Case 3: Call before conditional return in switch */
    switch (param2 % 3) {
        case 0:
            reg_var ^= 0xABCD;
            /* Call at end of case block */
            result = helper3(reg_var);
            /* Fall through creates basic block boundary */
        case 1:
            result += helper1(reg_var, v10);
            break;
        default:
            result = helper2(reg_var, v10, param1);
            /* Return creates basic block boundary */
            return result;
    }
    
    /* Final use of reg_var ensures it must be preserved across all calls */
    sink = reg_var;  /* Volatile write prevents optimization */
    return result + sink;
}

/* Another test with different pattern */
__attribute__((noinline, noclone, optimize("O3")))
int test_function2(int iterations) {
    /* Bind to another call-clobbered register */
    register int counter asm("r13") = 0;
    register int accumulator asm("r14") = 1;
    
    /* Complex loop with calls at block boundaries */
    for (int i = 0; i < iterations; i++) {
        /* Multiple computations to use many registers */
        int a = i * 2;
        int b = i + 5;
        int c = a ^ b;
        int d = b << 3;
        int e = c | d;
        int f = d - a;
        int g = e * f;
        int h = g / (i + 1);
        
        /* Alternate between two different call patterns */
        if (i % 2 == 0) {
            /* Call at end of if-block */
            accumulator += helper1(counter, h);
            /* counter must be preserved across helper1 */
        } else {
            /* Different call at end of else-block */
            accumulator ^= helper2(counter, g, f);
        }
        
        /* Update counter - live across the calls above */
        counter += accumulator;
        
        /* Another call at loop end */
        if (i < iterations - 1) {
            accumulator = helper3(accumulator);
            /* Loop back edge creates basic block boundary */
        }
    }
    
    sink = counter;  /* Force counter to be live at end */
    return accumulator;
}

int main() {
    int total = 0;
    
    /* Test multiple paths to exercise different basic block structures */
    for (int i = 0; i < 20; i++) {
        total += test_function(i, i * 2);
        total += test_function2(i % 5 + 1);
    }
    
    printf("Result: %d\n", total);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
