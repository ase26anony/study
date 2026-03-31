/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For coverage analysis: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g1 = 1, g2 = 2, g3 = 3, g4 = 4, g5 = 5;
volatile int sink = 0;

/* Helper functions that won't be inlined */
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

/* Force register pressure and specific register usage */
__attribute__((noinline, noclone))
int test_function(int x, int y, int z) {
    /* Create many local variables to increase register pressure */
    int v1 = x + y;
    int v2 = y * z;
    int v3 = z - x;
    int v4 = x * x;
    int v5 = y * y;
    int v6 = z * z;
    int v7 = v1 + v2;
    int v8 = v3 + v4;
    int v9 = v5 + v6;
    
    /* Use explicit register variable for a call-clobbered register */
    /* r12 is call-clobbered on x86_64 System V ABI */
    register int reg_var asm ("r12") = v7 + v8 + v9 + g4;
    
    /* Force reg_var to be live across multiple function calls */
    int result = 0;
    
    /* Scenario 1: Call at end of basic block before return */
    if (x > 0) {
        /* reg_var is used before call */
        int tmp1 = reg_var * 2;
        
        /* Function call at the end of basic block (before implicit return) */
        result = helper1(tmp1, v1);
        
        /* This return creates a basic block boundary right after the call */
        return result + reg_var;  /* reg_var must be preserved across call */
    }
    
    /* Scenario 2: Call at end of loop body */
    for (int i = 0; i < 3; i++) {
        /* reg_var is modified and used in loop */
        reg_var += i * g5;
        
        /* Multiple computations to increase register pressure */
        int a = v1 * i + v2;
        int b = v3 * i - v4;
        int c = v5 * i + v6;
        
        /* Function call as last statement in loop body */
        int call_result = helper2(a, b, c);
        
        /* Loop increment/jump creates basic block boundary after call */
        /* reg_var must be preserved across helper2 call */
    }
    
    /* Scenario 3: Call in one branch of if-else */
    if (y > 0) {
        /* Complex computation using reg_var */
        int tmp2 = reg_var * 3 + v7;
        
        /* Call at end of if branch */
        result = helper3(tmp2, v8, v9, g1);
        
        /* Basic block ends here (jump to merge point) */
    } else {
        /* Different computation in else branch */
        result = reg_var / 2;
    }
    
    /* Use reg_var after conditional calls */
    sink = reg_var;  /* Force reg_var to be live */
    
    return result + reg_var;
}

/* Another test with different register pressure pattern */
__attribute__((noinline, noclone))
int test_function2(int a, int b) {
    /* Even more variables to force spills */
    int w1 = a + 1, w2 = a + 2, w3 = a + 3, w4 = a + 4;
    int w5 = b + 1, w6 = b + 2, w7 = b + 3, w8 = b + 4;
    int w9 = w1 * w2, w10 = w3 * w4, w11 = w5 * w6, w12 = w7 * w8;
    
    /* Use another call-clobbered register */
    register int reg_var2 asm ("r13") = w9 + w10 + w11 + w12;
    
    /* Nested loops with calls at block ends */
    int sum = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Modify register variable */
            reg_var2 += i * j * g2;
            
            /* Call at end of inner loop body */
            sum += helper1(reg_var2, w1 + i);
            
            /* Inner loop increment creates block boundary */
        }
        
        /* Call at end of outer loop body */
        sum += helper2(reg_var2, w2, i);
    }
    
    /* Final call right before return */
    int final = helper3(sum, reg_var2, a, b);
    
    /* Force reg_var2 to be live across all calls */
    sink = reg_var2;
    
    return final;
}

/* Test with volatile to prevent reordering */
__attribute__((noinline, noclone))
int test_volatile_barrier(int x) {
    volatile int barrier = g1;
    
    /* Use explicit register variable */
    register int reg asm ("r12") = x * barrier;
    
    /* Multiple calls separated by volatile accesses */
    int r1 = helper1(reg, barrier);
    barrier = g2;
    
    /* Call at end of basic block before conditional jump */
    if (r1 > 0) {
        int r2 = helper2(reg, r1, barrier);
        barrier = g3;
        return r2 + reg;  /* reg must survive helper2 call */
    }
    
    barrier = g4;
    return reg;
}

int main() {
    int total = 0;
    
    /* Call test functions multiple times with different arguments
     * to exercise different paths and register allocation decisions */
    total += test_function(1, 2, 3);
    total += test_function(-1, 5, 10);
    total += test_function(10, -5, 0);
    
    total += test_function2(7, 8);
    total += test_function2(3, 4);
    
    total += test_volatile_barrier(6);
    total += test_volatile_barrier(-2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (sink=%d)\n", total, sink);
    
    return 0;
}
