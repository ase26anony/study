/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For coverage analysis: gcc -O1 -fprofile-arcs -ftest-coverage -fno-optimize-sibling-calls -fno-inline caller-save-test.c -o caller-save-test
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

/* Complex function with register pressure and calls at block boundaries */
__attribute__((noinline, noclone))
int test_function(int x, int y, int mode) {
    /* Create register pressure with many local variables */
    int v1 = x + y;
    int v2 = x * y;
    int v3 = x - y;
    int v4 = x ^ y;
    int v5 = x | y;
    int v6 = x & y;
    int v7 = x << 2;
    int v8 = y >> 1;
    int v9 = v1 + v2;
    int v10 = v3 * v4;
    
    /* Use explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2 + v3;
    
    /* Force reg_var to be live across calls */
    int result = 0;
    
    if (mode == 0) {
        /* Call at end of basic block before return */
        int tmp = helper1(v1, v2);
        result = reg_var + tmp;  /* reg_var must be preserved across helper1 call */
        sink = result;  /* Use result to prevent optimization */
        return result;  /* Basic block ends after call */
    } 
    else if (mode == 1) {
        /* Call as last statement in if branch */
        int tmp = helper2(v3, v4, v5);
        result = reg_var * tmp;
        sink = result;
        
        if (x > y) {
            /* Another call at end of nested block */
            int tmp2 = helper3(v6, v7, v8, v9);
            result += tmp2 + reg_var;  /* reg_var live across helper3 */
            sink = result;
            return result;  /* Block ends after call */
        } else {
            return result + 1;
        }
    }
    else if (mode == 2) {
        /* Loop with call at end of loop body */
        for (int i = 0; i < 3; i++) {
            v1 += i;
            v2 *= (i + 1);
            
            /* Call at end of loop body - creates block boundary */
            int tmp = helper1(v1, v2);
            result = reg_var + tmp;  /* reg_var must be saved/restored each iteration */
            sink = result;
            
            /* Loop increment/jump is in separate basic block */
        }
        return result + reg_var;
    }
    else {
        /* Multiple calls in sequence with computations in between */
        int tmp1 = helper1(v1, v2);
        reg_var += tmp1;  /* Modify reg_var */
        
        int tmp2 = helper2(v3, v4, v5);
        result = reg_var * tmp2;  /* reg_var live across helper2 */
        
        int tmp3 = helper3(v6, v7, v8, v9);
        result += tmp3 + reg_var;  /* reg_var live across helper3 */
        
        sink = result;
        
        /* Call as last statement before return */
        int final = helper1(result, reg_var);
        return final;  /* Block ends after call */
    }
}

/* Another test with switch statement creating multiple block boundaries */
__attribute__((noinline, noclone))
int test_switch(int x, int y) {
    register int reg_var asm ("r12") = x * y + g1;
    int result = 0;
    
    switch (x % 4) {
        case 0:
            result = helper1(x, y) + reg_var;
            sink = result;
            break;  /* Basic block ends with call */
            
        case 1:
            result = helper2(x, y, reg_var);
            sink = result;
            /* Fall through to next case */
            
        case 2:
            result += helper3(reg_var, x, y, g2);
            sink = result;
            return result;  /* Block ends after call */
            
        case 3:
            if (y > 0) {
                result = helper1(reg_var, y);
                sink = result;
                return result;  /* Block ends after call in if branch */
            }
            break;
    }
    
    return result + reg_var;
}

/* Main function to exercise all paths */
int main() {
    int total = 0;
    
    /* Test different modes to exercise different block structures */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i + 1, i % 4);
        total += test_switch(i, i + 2);
        
        /* Additional pressure with volatile accesses */
        g1 = (g1 + 1) % 10;
        g2 = (g2 * 2) % 20;
        g3 = (g3 - 1) & 0xFF;
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d (sink: %d)\n", total, sink);
    
    return 0;
}
