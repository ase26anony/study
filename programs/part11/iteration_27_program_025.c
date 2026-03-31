/* caller_save_test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller_save_test.c -o caller_save_test
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

/* Complex function with register pressure and calls at block boundaries */
__attribute__((noinline, noclone)) int test_function(int iter, int mode) {
    /* Create many local variables to increase register pressure */
    int v1 = g1, v2 = g2, v3 = g3, v4 = g4, v5 = g5;
    int v6 = v1 + v2, v7 = v3 * v4, v8 = v5 - v1;
    int v9 = v6 * v7, v10 = v8 + v9;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v10;
    
    int result = 0;
    int i;
    
    /* Loop creates basic block boundaries */
    for (i = 0; i < iter; i++) {
        int temp;
        
        /* Different paths create different basic blocks */
        if (mode == 0) {
            /* Call at end of basic block before loop increment */
            temp = helper1(reg_var, v1);
            /* Use reg_var after call - forces save/restore */
            reg_var = temp + v2 + i;
            result += reg_var;
            
            /* Additional computation to keep reg_var live */
            v1 = helper2(v1, v2, v3);
        } 
        else if (mode == 1) {
            /* Another call at block end */
            temp = helper2(reg_var, v3, v4);
            reg_var = temp - v5;
            result += reg_var * 2;
            
            v2 = helper1(v2, v4);
        } 
        else {
            /* Call as last statement before return in some iterations */
            if (i == iter - 1) {
                temp = helper3(reg_var, v4, v5, v6);
                reg_var = temp / 2;
                result = reg_var;  /* Last use - return value */
                sink = result;     /* Volatile sink */
                return result;     /* Call is at end of block before return */
            } else {
                temp = helper1(reg_var, v6);
                reg_var = temp + v7;
                result += reg_var;
            }
        }
        
        /* More computations to increase register pressure */
        v3 = v4 + v5;
        v4 = v5 * v6;
        v5 = v6 - v7;
        v6 = v7 + v8;
        v7 = v8 * v9;
        v8 = v9 - v10;
        v9 = v10 + v1;
        v10 = v1 * v2;
        
        /* Force reg_var to be live across all this */
        reg_var += v3 + v4 + v5;
    }
    
    sink = result;
    return result;
}

/* Another test with if-else structure creating block boundaries */
__attribute__((noinline, noclone)) int test_function2(int x, int y) {
    /* Multiple register variables */
    register int r1 asm ("r12") = x + g1;
    register int r2 asm ("r13") = y + g2;
    register int r3 asm ("r14") = x * y + g3;
    
    int a = g4, b = g5, c, d;
    
    /* Complex condition creating basic blocks */
    if (x > 0) {
        /* Call at end of if block */
        c = helper1(r1, a);
        r1 = c + b;
        
        /* Another call */
        d = helper2(r2, r1, b);
        r2 = d - a;
        
        /* Use volatile to prevent optimization */
        sink = r1 + r2;
        
        /* Return immediately after call - creates block boundary */
        if (y < 0) {
            int temp = helper3(r3, r1, r2, a);
            r3 = temp * 2;
            sink = r3;
            return r3;  /* Call is at end of block before return */
        }
    } else {
        /* Different path */
        c = helper2(r1, r2, a);
        r1 = c * 3;
        
        d = helper1(r3, b);
        r3 = d - r1;
    }
    
    /* More calls in a switch */
    switch (x % 3) {
        case 0:
            a = helper1(r1, r2);
            r1 = a + r3;
            break;  /* Basic block ends after call */
        case 1:
            a = helper2(r2, r3, r1);
            r2 = a - g1;
            /* Fall through */
        case 2:
            a = helper3(r3, r1, r2, g2);
            r3 = a * 2;
            break;
    }
    
    /* Final computation using all register variables */
    int result = r1 + r2 * 2 + r3 * 3;
    sink = result;
    return result;
}

int main() {
    int total = 0;
    
    /* Test different paths to cover various basic block structures */
    total += test_function(3, 0);
    total += test_function(4, 1);
    total += test_function(2, 2);
    total += test_function(5, 0);
    
    total += test_function2(10, 20);
    total += test_function2(-5, 30);
    total += test_function2(7, -8);
    
    printf("Result: %d\n", total);
    
    /* Additional test with inline assembly to force specific register usage */
    {
        register int r12_val asm ("r12") = total + g1;
        register int r13_val asm ("r13") = total * 2 + g2;
        
        /* Use inline assembly to clobber registers */
        asm volatile ("" : : "r" (r12_val), "r" (r13_val) : "r12", "r13");
        
        /* Call with register variables live */
        int temp = helper1(r12_val, r13_val);
        
        /* Force save/restore around call */
        r12_val = temp + g3;
        r13_val = temp - g4;
        
        asm volatile ("" : : "r" (r12_val), "r" (r13_val));
        
        total += r12_val + r13_val;
    }
    
    printf("Final result: %d\n", total);
    return 0;
}
