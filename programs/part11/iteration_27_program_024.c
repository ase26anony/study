/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g1 = 1;
volatile int g2 = 2;
volatile int g3 = 3;
volatile int g4 = 4;
volatile int sink = 0;

/* Noinline helper functions to force actual calls */
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

/* Complex function with register pressure */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create many local variables to increase register pressure */
    int v1 = param1 * 2;
    int v2 = param2 + 7;
    int v3 = param3 - 3;
    int v4 = v1 + v2;
    int v5 = v2 * v3;
    int v6 = v3 / (param1 ? param1 : 1);
    int v7 = v4 ^ v5;
    int v8 = v5 | v6;
    int v9 = v6 & v7;
    int v10 = v7 + v8;
    int v11 = v8 - v9;
    int v12 = v9 * v10;
    int v13 = v10 / (v11 ? v11 : 1);
    int v14 = v11 ^ v12;
    int v15 = v12 | v13;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2 + v3 + g4;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* Scenario 1: Call at end of if-branch basic block */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Call at what will be the end of this basic block */
        result = helper1(temp, v4);
        
        /* This return creates a basic block boundary right after the call */
        return result + reg_var;  /* reg_var must be live here */
    }
    
    /* Scenario 2: Call at end of loop body basic block */
    for (int i = 0; i < param2; i++) {
        /* Compute with reg_var */
        reg_var += v5 + i;
        
        /* Another computation forcing reg_var to stay in register */
        v6 = reg_var & 0xFF;
        
        /* Call at end of loop body - creates basic block boundary */
        v7 = helper2(reg_var, v6, v8);
        
        /* Loop increment/jump creates new basic block */
    }
    
    /* Scenario 3: Call before return in else branch */
    if (param3 < 0) {
        v8 = helper3(reg_var, v9, v10, v11);
        result = v8 * 2;
    } else {
        /* Call as last instruction before return */
        result = helper1(reg_var, v12);
        
        /* Basic block ends with this call, return is separate block */
    }
    
    /* Force reg_var to be used after all calls */
    sink = reg_var;  /* Prevent dead store elimination */
    
    return result + reg_var;
}

/* Another test with different pattern */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Bind to another call-clobbered register */
    register int counter asm ("r13") = g1;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple live values across calls */
        int a = counter + i;
        int b = a * 2;
        int c = b - i;
        
        /* Call with many arguments to increase register pressure */
        int res = helper3(a, b, c, counter);
        
        /* Update counter - must be preserved across call */
        counter += res & 0xF;
        
        /* Another call at loop end */
        if (i % 2 == 0) {
            sum += helper2(counter, res, a);
            /* Basic block ends with this call when i%2==0 */
        } else {
            sum += helper1(counter, b);
            /* Different basic block ends with this call */
        }
        
        /* Loop latch is separate basic block */
    }
    
    sink = counter;
    return sum + counter;
}

/* Test with switch statement creating multiple basic blocks */
__attribute__((noinline, noclone))
int test_function3(int mode) {
    register int key asm ("r14") = g2;
    int result = 0;
    
    switch (mode % 4) {
        case 0:
            key += helper1(key, g1);
            /* Fall through - creates complex CFG */
        case 1:
            result = helper2(key, g3, mode);
            /* Call at end of case basic block */
            break;
        case 2:
            result = helper3(key, mode, g1, g2);
            /* Another call at end of basic block */
            break;
        default:
            key *= 2;
            result = helper1(key, result);
            /* Call at end of default basic block */
            break;
    }
    
    sink = key;
    return result + key;
}

int main() {
    int total = 0;
    
    /* Run multiple test cases to exercise different paths */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i * 2, i - 5);
        total += test_function2(i + 1);
        total += test_function3(i);
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d (sink=%d)\n", total, sink);
    
    return 0;
}
