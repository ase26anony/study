/* caller-save-test.c
 * Test program to trigger caller-save register spills at basic block boundaries
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 42;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int a, int b) {
    return a * b + 1;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    return a - b * c;
}

__attribute__((noinline, noclone))
void helper3(int *ptr) {
    *ptr += global_source;
}

/* Test function with register pressure and calls at block boundaries */
__attribute__((noinline, noclone))
int test_caller_save(int param) {
    /* Create register pressure with many variables */
    int v1 = param;
    int v2 = param * 2;
    int v3 = param * 3;
    int v4 = param * 4;
    int v5 = param * 5;
    int v6 = param * 6;
    int v7 = param * 7;
    int v8 = param * 8;
    int v9 = param * 9;
    int v10 = param * 10;
    
    /* Use explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = global_source;
    
    /* Force reg_var to be live across multiple calls */
    reg_var = v1 + v2 + v3;
    
    /* First call - placed at end of basic block (before return in if branch) */
    if (param > 100) {
        /* This call is at the end of a basic block */
        int result = helper1(reg_var, v4);
        /* Use reg_var immediately after call */
        reg_var += result;
        global_sink = reg_var;
        return reg_var;  /* Basic block ends after call */
    }
    
    /* More computations to keep variables live */
    v5 = helper2(v4, v5, reg_var);
    reg_var ^= v5;
    
    /* Second call - at end of loop body */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        v6 += i;
        v7 -= i;
        /* Call at end of loop body - creates block boundary */
        int temp = helper1(reg_var, v6);
        reg_var = temp + v7;  /* reg_var live across call */
        
        /* Loop latch is separate basic block */
    }
    
    /* Third call - before return in else branch */
    if (param < 50) {
        v8 = helper2(v8, v9, reg_var);
        reg_var |= v8;
    } else {
        /* Call at end of basic block before return */
        helper3(&reg_var);
        return reg_var;  /* Basic block ends after call */
    }
    
    /* More register pressure */
    v9 = v1 * v2 * v3 * v4 * v5;
    v10 = v6 * v7 * v8 * v9;
    
    /* Fourth call - in switch case at block end */
    switch (param % 4) {
        case 0:
            reg_var = helper1(reg_var, v9);
            break;  /* Basic block ends after call */
        case 1:
            reg_var = helper2(reg_var, v10, param);
            global_sink = reg_var;
            return reg_var;  /* Basic block ends after call */
        case 2:
            helper3(&reg_var);
            /* Fall through */
        default:
            reg_var += helper1(v10, param);
    }
    
    /* Final use to ensure reg_var must be preserved */
    global_sink = reg_var + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    return global_sink;
}

/* Another test with different pattern */
__attribute__((noinline, noclone))
int test_caller_save2(int param) {
    /* Use different call-clobbered register */
    register int reg_var2 asm ("r13") = param;
    
    /* Create complex control flow */
    int a = param;
    int b = param * 2;
    int c = param * 3;
    
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0) {
            /* Call at end of if block */
            reg_var2 = helper1(reg_var2, a);
            a++;  /* Separate basic block */
        } else {
            /* Call at end of else block */
            reg_var2 = helper2(reg_var2, b, c);
            b--;  /* Separate basic block */
        }
        
        /* Another call in loop body */
        c = helper1(b, c);
        
        /* Force spill by using many temporaries */
        int t1 = a * b;
        int t2 = b * c;
        int t3 = c * a;
        int t4 = t1 * t2;
        int t5 = t2 * t3;
        int t6 = t3 * t4;
        int t7 = t4 * t5;
        int t8 = t5 * t6;
        int t9 = t6 * t7;
        int t10 = t7 * t8;
        
        reg_var2 += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    }
    
    /* Call right before return */
    reg_var2 = helper1(reg_var2, param);
    return reg_var2;
}

int main() {
    int total = 0;
    
    /* Test various paths to exercise different basic block boundaries */
    for (int i = 0; i < 200; i++) {
        total += test_caller_save(i);
        total += test_caller_save2(i);
        
        /* Modify global to prevent optimization */
        global_source = (global_source * 13 + 17) % 256;
    }
    
    printf("Result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
