/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For coverage analysis: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_vol1 = 12345;
volatile int g_vol2 = 67890;
volatile int g_sink = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int a, int b) {
    g_vol1 = a + b;
    return g_vol1 ^ 0x55AA;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    g_vol2 = a * b + c;
    return g_vol2 | 0xFF00;
}

__attribute__((noinline, noclone))
int helper3(int a) {
    return a * 3 + 7;
}

/* Main test function - designed to force caller-save at BB boundaries */
__attribute__((noinline, noclone))
int test_caller_save(int mode, int iter) {
    /* Use explicit register variables for call-clobbered registers (x86_64) */
    register int r12_var asm ("r12") = g_vol1 + mode * 100;
    register int r13_var asm ("r13") = g_vol2 + iter * 200;
    
    /* Many local variables to increase register pressure */
    int v1 = g_vol1;
    int v2 = g_vol2;
    int v3 = v1 * v2;
    int v4 = v1 + v2;
    int v5 = v3 - v4;
    int v6 = v5 * 2;
    int v7 = v6 / 3;
    int v8 = v7 | 0xFF;
    int v9 = v8 & 0x3F;
    int v10 = v9 << 2;
    
    /* Force r12_var to be live across multiple calls */
    r12_var = r12_var + v3 + v4 + v5;
    
    /* First call site - placed at end of basic block (before return in if branch) */
    if (mode & 1) {
        /* This call is at the end of the if-block basic block */
        int result = helper1(r12_var, v10);
        
        /* Use r12_var after call - forces save/restore */
        r12_var = r12_var ^ result;
        
        /* Return creates BB boundary right after call */
        return r12_var + g_sink;
    }
    
    /* Second call site - at end of loop body basic block */
    int sum = 0;
    for (int i = 0; i < iter; i++) {
        v1 = helper2(r13_var, i, v10);
        
        /* r13_var used before and after call - must be preserved */
        r13_var = r13_var + v1 + i;
        
        /* Loop increment/jump creates BB boundary after call */
        sum += r13_var;
    }
    
    /* Third call site - complex control flow to create multiple BB ends */
    int x = g_vol1;
    while (x > 0) {
        if (x & 1) {
            /* Call at end of if-block inside loop */
            int tmp = helper3(r12_var + x);
            r12_var = r12_var * tmp;
            
            /* Break creates BB boundary */
            if (tmp > 1000) break;
        } else {
            /* Alternative path with different call */
            int tmp = helper1(r13_var, x);
            r13_var = r13_var - tmp;
        }
        
        /* Call at end of loop body (before continue/loop test) */
        x = helper2(x, r12_var, r13_var);
        
        /* Loop test creates BB boundary */
    }
    
    /* Mix both register variables */
    return (r12_var * 31 + r13_var * 17) ^ sum;
}

/* Another test with switch statement creating multiple BB ends */
__attribute__((noinline, noclone))
int test_switch_calls(int val) {
    register int rbx_var asm ("rbx") = g_vol1 * 2;
    register int r14_var asm ("r14") = g_vol2 / 2;
    
    int result = 0;
    
    switch (val % 4) {
        case 0:
            /* Call at end of case block (before break) */
            result = helper1(rbx_var, val);
            rbx_var += result;
            /* Break creates BB boundary */
            break;
            
        case 1:
            result = helper2(rbx_var, r14_var, val);
            r14_var ^= result;
            /* Fall through to create different BB structure */
            
        case 2:
            result = helper3(r14_var + val);
            rbx_var *= result;
            /* Break creates BB boundary */
            break;
            
        case 3:
            /* Multiple calls in same block */
            result = helper1(rbx_var, r14_var);
            /* Call at end of block before return */
            g_sink = helper2(result, val, g_vol1);
            /* Return creates BB boundary */
            return rbx_var + r14_var;
    }
    
    /* Use both variables after calls */
    return (rbx_var - r14_var) * result;
}

int main() {
    int total = 0;
    
    /* Exercise different paths */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i % 3, (i % 5) + 1);
        total += test_switch_calls(i);
        
        /* Modify volatiles to change behavior */
        g_vol1 = (g_vol1 * 1103515245 + 12345) & 0x7FFFFFFF;
        g_vol2 = (g_vol2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent optimization */
    g_sink = total;
    printf("Result: %d\n", total);
    
    return 0;
}
