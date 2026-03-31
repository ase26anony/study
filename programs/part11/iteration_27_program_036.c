/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fno-pic caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g1 = 1, g2 = 2, g3 = 3, g4 = 4, g5 = 5;
volatile int sink = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) int helper1(int a, int b) {
    return a + b + g1;
}

__attribute__((noinline, noclone)) int helper2(int a, int b, int c) {
    return a * b - c + g2;
}

__attribute__((noinline, noclone)) int helper3(int a, int b, int c, int d) {
    return (a + b) * (c - d) + g3;
}

/* Force register pressure and specific register usage */
__attribute__((noinline, noclone)) int test_function(int param1, int param2, int param3) {
    /* Use explicit register variables for call-clobbered registers on x86_64 */
    register int r12_var asm("r12") = param1 * 2;  /* r12 is call-clobbered */
    register int r13_var asm("r13") = param2 + 7;  /* r13 is call-clobbered */
    register int r14_var asm("r14") = param3 - 3;  /* r14 is call-clobbered */
    
    /* Many local variables to increase register pressure */
    int v1 = g1, v2 = g2, v3 = g3, v4 = g4, v5 = g5;
    int v6 = v1 + v2, v7 = v3 * v4, v8 = v5 - v1;
    int v9 = v6 * v7, v10 = v8 + v9, v11 = v10 / 2;
    
    /* Force r12_var to be live across a function call */
    /* This creates a basic block boundary after the call */
    if (param1 > 0) {
        /* Call at the end of basic block - will be followed by jump to merge point */
        int result = helper1(r12_var, v11);
        
        /* Use r12_var after call - forces caller-save */
        r12_var = result + r12_var + v1;
        
        /* Basic block ends here (implicit jump to merge point) */
    } else {
        /* Alternative path without call */
        r12_var = helper2(r12_var, v2, v3);
    }
    
    /* Merge point - use all register variables */
    int sum = r12_var + r13_var + r14_var;
    
    /* More register pressure */
    for (int i = 0; i < 3; i++) {
        /* Loop creates basic blocks */
        if (i == 1) {
            /* Call at end of loop body basic block */
            r13_var = helper3(r13_var, v4, v5, i);
            /* Loop increment/jump in separate basic block */
        }
        v1 += i;
        v2 *= (i + 1);
    }
    
    /* Another call right before return - creates basic block boundary */
    r14_var = helper2(r14_var, sum, v1);
    
    /* Force use of all variables to prevent optimization */
    sink = r12_var + r13_var + r14_var + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11;
    
    return sink;
}

/* Second test with different control flow */
__attribute__((noinline, noclone)) int test_function2(int x) {
    register int rax_var asm("rax") = x * 3;  /* rax is call-clobbered (return reg) */
    register int rbx_var asm("rbx") = x + 10; /* rbx is callee-saved, for contrast */
    
    volatile int trigger = g1;
    
    /* Switch with calls at case ends */
    switch (x % 4) {
        case 0:
            rax_var = helper1(rax_var, trigger);
            /* Fall through to next case */
        case 1:
            rbx_var = helper2(rbx_var, rax_var, g2);
            /* Basic block ends with break */
            break;
        case 2:
            /* Call as last statement before break */
            rax_var = helper3(rax_var, rbx_var, g3, g4);
            break;
        default:
            rax_var = helper1(rax_var, rax_var);
            /* No break - falls through to end */
    }
    
    /* Call right before return in default case fallthrough */
    if (x % 4 == 3) {
        rax_var = helper2(rax_var, g5, trigger);
    }
    
    sink += rax_var + rbx_var;
    return rax_var;
}

/* Third test: loop with call at end of body */
__attribute__((noinline, noclone)) int test_function3(int n) {
    register int r15_var asm("r15") = n;  /* r15 is call-clobbered on x86_64 SysV */
    int acc = 0;
    
    for (int i = 0; i < n; i++) {
        int temp = g1 + i;
        
        /* Call at the end of loop body basic block */
        /* r15_var must be saved/restored around call */
        r15_var = helper1(r15_var, temp);
        
        /* Use result immediately */
        acc += r15_var;
        
        /* Loop increment in separate basic block */
    }
    
    /* Final call before return */
    acc = helper2(acc, r15_var, g2);
    
    sink += acc;
    return acc;
}

int main() {
    int total = 0;
    
    printf("Testing caller-save at basic block boundaries...\n");
    
    /* Run multiple test cases to exercise different paths */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i*2, i*3);
        total += test_function2(i);
        total += test_function3(i % 5);
    }
    
    /* Use volatile sink to prevent optimization */
    printf("Result: %d (sink=%d)\n", total, sink);
    
    /* Also test with different optimization flags via compile-time switch */
#ifdef TEST_CALL_END
    /* Additional test with call as last instruction before return */
    register int rcx_var asm("rcx") = total;  /* rcx is call-clobbered */
    rcx_var = helper1(rcx_var, g1);
    /* Function ends here - call was at end of basic block */
    return rcx_var;
#else
    return total % 256;
#endif
}
