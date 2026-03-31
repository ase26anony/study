/* Test program to trigger caller-save register insertion at basic block boundaries */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller_save_test.c */

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
    int v5 = v2 * v3;
    int v6 = v3 + v4;
    int v7 = v4 - v5;
    int v8 = v5 ^ v6;
    int v9 = v6 * v7;
    int v10 = v7 + v8;
    
    /* Explicit register variable bound to a call-clobbered register */
    /* Using r12 on x86_64 which is call-clobbered */
    register int reg_var asm ("r12");
    reg_var = v1 + v2 + v3;
    
    /* Force reg_var to be live across multiple calls at block boundaries */
    
    /* CASE 1: Call at end of if-block before return */
    if (param1 > 100) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call at what should be end of basic block */
        int result = helper1(temp, v4);
        
        /* This assignment creates a basic block boundary */
        global_sink = result;
        
        /* Use reg_var after call - forces save/restore */
        reg_var += global_sink;
        
        /* Return creates block boundary */
        return reg_var + v10;
    }
    
    /* CASE 2: Call at end of loop body */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Compute with reg_var */
        reg_var = reg_var * (i + 1) + v5;
        
        /* Function call at end of loop body (before i++ in separate block) */
        int call_result = helper2(reg_var, v6, i);
        
        /* Use result immediately */
        sum += call_result;
        
        /* reg_var is live across the call */
        reg_var ^= call_result;
        
        /* Loop increment is in separate basic block */
    }
    
    /* CASE 3: Call before conditional return in switch */
    switch (param2 % 3) {
        case 0:
            reg_var = helper3(reg_var);
            /* Fall through to return - call at block end */
            global_sink = reg_var;
            return sum + reg_var;
            
        case 1:
            /* Another call at block end */
            reg_var = helper1(reg_var, sum);
            /* Different return path */
            return reg_var - v9;
            
        default:
            /* Complex computation forcing spills */
            int t1 = helper2(reg_var, v7, v8);
            int t2 = helper1(t1, v9);
            reg_var = helper3(t2);
            /* Call at end before return */
            global_sink = helper2(reg_var, v10, sum);
            return reg_var + global_sink;
    }
}

/* Another test with different register pressure */
__attribute__((noinline, noclone))
int test_caller_save2(int iterations) {
    /* Bind to another call-clobbered register */
    register int r13_var asm ("r13");
    r13_var = global_source;
    
    volatile int sink = 0;
    int acc = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Force r13_var to be live across call */
        int pre_value = r13_var;
        
        /* Call at what should be end of basic block (before loop latch) */
        int result = helper1(pre_value, i);
        
        /* Use r13_var after call */
        r13_var = result ^ pre_value;
        
        acc += r13_var;
        
        /* Create more register pressure in loop */
        int a = acc * 2;
        int b = a + i;
        int c = b ^ r13_var;
        int d = helper2(a, b, c);
        r13_var += d;
        
        /* Another call at potential block end */
        if (i % 2 == 0) {
            sink = helper3(r13_var);
            /* if-block ends here */
        } else {
            r13_var = helper2(r13_var, acc, sink);
            /* else-block ends here */
        }
        
        /* Loop latch is separate basic block */
    }
    
    return acc + r13_var;
}

int main() {
    int total = 0;
    
    /* Test multiple paths to exercise different basic block structures */
    for (int i = 0; i < 10; i++) {
        total += test_caller_save(i * 50, i * 20, i * 30);
        total += test_caller_save2(i + 1);
    }
    
    /* Also test edge cases */
    total += test_caller_save(150, 0, 0);  /* Takes if-branch */
    total += test_caller_save(50, 1, 0);   /* Takes switch case 1 */
    total += test_caller_save(50, 2, 0);   /* Takes switch default */
    
    printf("Result: %d\n", total);
    return 0;
}
