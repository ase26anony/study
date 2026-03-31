/* caller-save-test.c
 * Designed to trigger caller-save register spills at basic block boundaries
 * Specifically targets lines 905-913 in caller-save.cc
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

/* Complex function with register pressure and calls at block boundaries */
__attribute__((noinline, noclone, optimize("O2")))
int test_function(int x, int y, int mode) {
    /* Create register pressure with many variables */
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
    
    /* Explicit register variable - force use of call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2 + v3;
    
    /* Use reg_var in computation before call */
    int pre_call = reg_var * 2 + g4;
    
    /* Result accumulator */
    int result = 0;
    
    /* Different basic blocks with calls at the end */
    if (mode == 0) {
        /* Call at end of if-block before else */
        result = helper1(v1, v2) + pre_call;
        reg_var = result + v3;  /* Keep reg_var live */
        
        /* This call is at the end of basic block (before else) */
        int temp = helper2(reg_var, v4, v5);
        sink = temp;  /* Force side effect */
        
        /* Basic block boundary here */
    } else if (mode == 1) {
        /* Call as last statement before return in this branch */
        result = helper2(v3, v4, v5) + reg_var;
        reg_var = result * 2;
        
        /* Another call at block end */
        int temp = helper3(reg_var, v6, v7, v8);
        result += temp;
        
        /* Return immediately after call - creates block boundary */
        return result + sink;
    } else {
        /* Loop with call at end of body */
        for (int i = 0; i < 3; i++) {
            v1 += i;
            v2 *= (i + 1);
            
            /* Use reg_var across call */
            reg_var = v1 + v2 + reg_var;
            
            /* Call at end of loop body (before increment) */
            int temp = helper1(reg_var, v3);
            result += temp;
            
            /* Loop increment creates new basic block */
        }
        
        /* Another call before final return */
        result += helper2(reg_var, v9, v10);
    }
    
    /* Use reg_var after all calls - forces save/restore */
    result += reg_var * 3;
    
    /* Final call at end of function */
    result = helper3(result, v9, v10, sink);
    
    return result;
}

/* Another test with more aggressive register pressure */
__attribute__((noinline, noclone, optimize("O3")))
int test_function2(int iterations) {
    /* Even more variables for register pressure */
    int a = g1, b = g2, c = g3, d = g4, e = g5;
    int f = a + b, g = c - d, h = e * f, i = g / 2;
    int j = h ^ i, k = j | a, l = k & b, m = l << 1;
    int n = m >> 2, o = n + c, p = o - d, q = p * e;
    
    /* Multiple register variables targeting call-clobbered regs */
    register int r12_var asm ("r12") = a + b + c;
    register int r13_var asm ("r13") = d + e + f;
    register int r14_var asm ("r14") = g + h + i;
    
    int total = 0;
    
    /* Loop with multiple calls at different block boundaries */
    for (int idx = 0; idx < iterations; idx++) {
        /* Use register variables */
        r12_var += idx;
        r13_var *= (idx + 1);
        r14_var ^= idx;
        
        /* Conditional with calls at block ends */
        if (idx % 2 == 0) {
            /* Call at end of if-block */
            int t1 = helper1(r12_var, r13_var);
            total += t1;
            
            /* Another call - last in block */
            int t2 = helper2(r14_var, t1, idx);
            r12_var = t2;
            
            /* Block boundary to else */
        } else {
            /* Different call pattern */
            int t3 = helper3(r12_var, r13_var, r14_var, idx);
            total += t3;
            
            /* Call right before loop increment */
            r13_var = helper1(t3, total);
            
            /* Loop increment creates new block */
        }
        
        /* Use all register variables to keep them live */
        total += r12_var + r13_var + r14_var;
    }
    
    /* Final computation with register variables */
    total = helper2(total, r12_var, r13_var);
    total = helper3(total, r14_var, sink, g5);
    
    return total;
}

/* Test with inline assembly to force specific register usage */
__attribute__((noinline, noclone))
int test_with_asm(int x, int y) {
    int a = x, b = y;
    
    /* Force rbx usage (call-clobbered on x86-64 System V) */
    register int rbx_var asm ("rbx");
    
    /* Initialize with inline asm to prevent optimization */
    asm volatile ("mov %1, %0" : "=r" (rbx_var) : "r" (a + b));
    
    /* Multiple computations to create pressure */
    for (int i = 0; i < 4; i++) {
        a += i;
        b *= (i + 1);
        
        /* Use rbx_var across call */
        rbx_var = rbx_var * 2 + a;
        
        /* Call at end of loop body */
        int temp = helper1(rbx_var, b);
        
        /* Force rbx_var to be live after call */
        asm volatile ("# keep rbx live" : "+r" (rbx_var));
        
        a += temp;
        
        /* Loop backedge creates block boundary */
    }
    
    /* Final use of register variable */
    int result = helper2(rbx_var, a, b);
    
    return result;
}

int main() {
    int checksum = 0;
    
    printf("Testing caller-save at block boundaries...\n");
    
    /* Test different paths to exercise various block boundaries */
    for (int i = 0; i < 10; i++) {
        checksum ^= test_function(g1 + i, g2 - i, i % 3);
        checksum += test_function2(i + 1);
        
        if (i % 2 == 0) {
            checksum += test_with_asm(i, i * 2);
        }
    }
    
    /* Use volatile sink to prevent optimization */
    sink = checksum;
    
    printf("Final checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    return checksum != 0 ? 0 : 1;
}
