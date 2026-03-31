/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller-save-test.c -o caller-save-test */

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
int test_caller_save(int iterations, int mode) {
    /* Create register pressure with many variables */
    int v1 = global_source;
    int v2 = v1 * 2;
    int v3 = v2 + 17;
    int v4 = v3 ^ 0xFF;
    int v5 = v4 - 100;
    int v6 = v5 / 3;
    int v7 = v6 << 2;
    int v8 = v7 | 0xAA;
    int v9 = v8 & 0x55;
    int v10 = v9 + 999;
    
    /* Explicit register variable bound to a call-clobbered register */
    /* Using r12 on x86_64 which is call-clobbered */
    register int reg_var asm ("r12") = v1 + v2 + v3;
    
    int result = 0;
    
    /* Loop to create multiple basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Force reg_var to be live across function calls */
        reg_var = reg_var + i + v4;
        
        /* Create a basic block ending with a function call */
        if (mode == 0) {
            /* This call is at the end of the if-block basic block */
            int tmp = helper1(reg_var, v5);
            /* Use reg_var immediately after call - forces save/restore */
            result += tmp + reg_var;
            /* Basic block ends here (before the else) */
        } else if (mode == 1) {
            /* Alternative path with different call pattern */
            int tmp = helper2(reg_var, v6, v7);
            result += tmp - reg_var;
            /* Another basic block boundary */
        } else {
            /* Third path - call as last statement before loop end */
            int tmp = helper3(reg_var);
            result += tmp ^ reg_var;
            /* Loop increment creates new basic block */
        }
        
        /* Modify reg_var so it stays live across iterations */
        reg_var = reg_var ^ result;
        
        /* Create more register pressure between calls */
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        v5 = v6 + v7;
        v6 = v7 + v8;
        v7 = v8 + v9;
        v8 = v9 + v10;
        v9 = v10 + i;
        v10 = v1 * i;
    }
    
    /* Final use of reg_var forces one more save/restore opportunity */
    if (result > 1000) {
        /* Call at the end of if-block before return */
        int tmp = helper1(reg_var, result);
        result = tmp;
        /* Basic block ends here (before function return) */
    } else {
        result = helper2(reg_var, result, 42);
    }
    
    /* Store to volatile to prevent dead code elimination */
    global_sink = result + reg_var;
    
    return result;
}

/* Second test function with different control flow */
__attribute__((noinline, noclone))
int test_boundary_calls(int x) {
    /* Use different call-clobbered register */
    register int counter asm ("r13") = x;
    
    /* Create a switch with calls at the end of each case */
    switch (x % 4) {
        case 0: {
            counter = helper1(counter, 1);
            /* Case ends with call - basic block boundary follows */
            break;
        }
        case 1: {
            int tmp = counter * 2;
            counter = helper2(counter, tmp, 3);
            break;
        }
        case 2: {
            counter = helper3(counter);
            /* Fall through to create more complex CFG */
        }
        default: {
            counter = helper1(counter, counter);
            /* Last statement before return */
        }
    }
    
    /* One more call right before return */
    if (counter > 0) {
        counter = helper2(counter, global_source, 99);
        /* Call at end of basic block before return */
    }
    
    global_sink += counter;
    return counter;
}

/* Third test: nested loops with calls at block ends */
__attribute__((noinline, noclone))
int test_nested_loops(int n) {
    register int acc asm ("r14") = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 3; j++) {
            /* Call at end of inner loop body */
            acc += helper1(i, j);
            /* Inner loop increment creates new basic block */
        }
        
        /* Call at end of outer loop body, before condition check */
        if (i % 2 == 0) {
            acc -= helper2(acc, i, 5);
            /* Basic block ends here (before loop increment) */
        } else {
            acc ^= helper3(acc);
        }
    }
    
    /* Call as last statement before return */
    acc = helper1(acc, global_source);
    
    global_sink ^= acc;
    return acc;
}

int main() {
    int total = 0;
    
    /* Exercise different paths to trigger various caller-save scenarios */
    for (int i = 0; i < 10; i++) {
        total += test_caller_save(3, i % 3);
        total += test_boundary_calls(i);
        total += test_nested_loops(2);
    }
    
    printf("Result: %d (sink: %d)\n", total, global_sink);
    return 0;
}
