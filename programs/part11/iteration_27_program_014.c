/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
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
void helper3(int *ptr) {
    *ptr += global_source;
}

/* Main test function - also noinline to preserve structure */
__attribute__((noinline, noclone))
int test_caller_save(int param1, int param2, int param3) {
    /* Create register pressure with many variables */
    int v1 = param1 * 2;
    int v2 = param2 + 7;
    int v3 = param3 - 3;
    int v4 = v1 * v2;
    int v5 = v2 + v3;
    int v6 = v3 * param1;
    int v7 = v4 - v5;
    int v8 = v6 + v7;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12");
    reg_var = v1 + v2 + v3 + global_source;
    
    /* More variables to increase pressure */
    int v9 = v8 * 2;
    int v10 = v9 - param2;
    int v11 = v10 + reg_var;
    
    /* First call - reg_var is live across this call */
    int result1 = helper1(v11, v8);
    
    /* Use reg_var after call */
    int v12 = reg_var * result1;
    
    /* Create a basic block boundary after a call */
    if (v12 > 1000) {
        /* Call at the end of basic block, followed by return */
        int result2 = helper2(reg_var, v12, result1);
        global_sink += result2;
        return result2;  /* Basic block ends here */
    } else {
        /* Another path with call at end of block */
        int v13 = v12 * 2;
        helper3(&v13);
        global_sink += v13;
        return v13;  /* Basic block ends here */
    }
}

/* Another test with loop structure */
__attribute__((noinline, noclone))
int test_loop_caller_save(int iterations) {
    register int reg_var asm ("r12") = global_source;
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Many variables to create register pressure */
        int v1 = i * 2;
        int v2 = reg_var + i;
        int v3 = v1 * v2;
        int v4 = sum + v3;
        
        /* Call at the end of loop body - basic block ends after call */
        int result = helper1(v4, reg_var);
        
        /* Use reg_var after call */
        reg_var = result + v3;
        sum += reg_var;
        
        /* Loop increment creates new basic block */
    }
    
    return sum;
}

/* Test with switch statement creating multiple block ends */
__attribute__((noinline, noclone))
int test_switch_caller_save(int choice) {
    register int reg_var asm ("r12") = global_source;
    int result = 0;
    
    switch (choice % 4) {
        case 0: {
            int v1 = reg_var * 2;
            /* Call at end of case block */
            result = helper1(v1, reg_var);
            global_sink += result;
            break;  /* Basic block ends here */
        }
        case 1: {
            int v2 = reg_var + 100;
            /* Another call at end of case block */
            result = helper2(v2, reg_var, global_source);
            global_sink += result;
            break;  /* Basic block ends here */
        }
        case 2: {
            int v3 = reg_var - 50;
            /* Call with side effect */
            helper3(&v3);
            result = v3;
            global_sink += result;
            break;  /* Basic block ends here */
        }
        default: {
            int v4 = reg_var / 2;
            /* Final call at end of default block */
            result = helper1(v4, v4);
            global_sink += result;
            /* No break, falls through to return */
        }
    }
    
    return result + reg_var;
}

int main() {
    int total = 0;
    
    /* Test different paths to cover various basic block endings */
    total += test_caller_save(10, 20, 30);
    total += test_caller_save(100, 200, 300);
    total += test_loop_caller_save(5);
    total += test_switch_caller_save(0);
    total += test_switch_caller_save(1);
    total += test_switch_caller_save(2);
    total += test_switch_caller_save(3);
    
    printf("Result: %d (sink: %d)\n", total, global_sink);
    
    /* Verify with checksum */
    int checksum = total + global_sink;
    if (checksum != 0) {
        return 0;  /* Success */
    }
    
    return 1;
}
