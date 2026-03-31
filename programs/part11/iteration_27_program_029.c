/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all -fdump-rtl-caller_save caller-save-test.c 2>&1 | grep -A5 -B5 "caller-save"
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g1 = 1, g2 = 2, g3 = 3, g4 = 4;
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
int test_function(int param1, int param2, int param3) {
    /* Create many local variables to increase register pressure */
    int v1 = param1 * 2;
    int v2 = param2 + g1;
    int v3 = param3 - g2;
    int v4 = v1 * v2;
    int v5 = v2 + v3;
    int v6 = v3 * param1;
    int v7 = v4 - v5;
    int v8 = v5 * v6;
    int v9 = v6 + v7;
    int v10 = v7 * v8;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2 + v3;
    
    /* First call site - placed at end of basic block (before return in if branch) */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp1 = reg_var * 2;
        
        /* Function call at end of basic block - reg_var must be saved/restored */
        int result1 = helper1(temp1, v4);
        
        /* This return creates a basic block boundary right after the call */
        return result1 + reg_var;  /* reg_var used after call */
    }
    
    /* Second call site - in loop with call at end of loop body */
    int sum = 0;
    for (int i = 0; i < param2; i++) {
        /* Modify reg_var before call */
        reg_var += i;
        
        /* Multiple variables live across call */
        int temp2 = helper2(reg_var, v5, v6);
        
        /* Call at end of loop body - basic block ends after call */
        int temp3 = helper3(temp2, v7, v8, v9);
        
        /* Use result and reg_var after call */
        sum += temp3 + reg_var;
        
        /* Loop increment creates new basic block */
    }
    
    /* Third call site - call just before another basic block boundary */
    if (param3 < 0) {
        int temp4 = helper1(reg_var, v10);
        
        /* Jump to label creates basic block boundary */
        goto done;
    }
    
    /* Fourth call site - call as last statement before return */
    int temp5 = helper2(reg_var, sum, v9);
    
done:
    /* Force use of reg_var after potential call */
    sink = reg_var + v10;
    
    return sink + (param1 * param2 * param3);
}

/* Another test with different register pressure pattern */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Even more variables to force spills */
    int a = g1, b = g2, c = g3, d = g4;
    int e = a * b, f = c * d, g = e + f, h = f - e;
    int i = g * h, j = h * a, k = i + j, l = j - i;
    
    /* Use different call-clobbered register */
    register int reg_var2 asm ("r13") = a + b + c + d;
    
    /* Nested loops with calls at block ends */
    int total = 0;
    for (int x = 0; x < iterations; x++) {
        for (int y = 0; y < 3; y++) {
            /* Call at end of inner loop body */
            int tmp = helper1(reg_var2 + x, e + y);
            
            /* Basic block ends after call (before inner loop increment) */
            total += tmp;
            
            /* Update register variable - must survive across call */
            reg_var2 += tmp;
        }
        
        /* Call at end of outer loop body */
        if (x % 2 == 0) {
            int tmp2 = helper2(reg_var2, total, f);
            
            /* Another basic block boundary */
            g = tmp2;
        }
    }
    
    /* Final call right before return */
    int final = helper3(reg_var2, total, g, h);
    
    sink = reg_var2;
    return final;
}

/* Test with switch statement creating multiple basic blocks */
__attribute__((noinline, noclone))
int test_function3(int mode) {
    register int key_reg asm ("r14") = mode * 10 + g1;
    int result = 0;
    
    switch (mode % 4) {
        case 0: {
            /* Call at end of case block */
            result = helper1(key_reg, g2);
            /* Fall through creates block boundary */
            key_reg += result;
        }
        case 1: {
            /* Another call */
            int tmp = helper2(key_reg, result, g3);
            /* Break creates block boundary */
            result += tmp;
            break;
        }
        case 2: {
            /* Call then return */
            result = helper3(key_reg, g1, g2, g3);
            return result + key_reg;  /* Block ends with return after call */
        }
        default: {
            /* Call then break */
            result = helper1(key_reg, key_reg);
            key_reg *= 2;
            break;
        }
    }
    
    /* Use register variable after switch */
    sink = key_reg;
    return result;
}

int main() {
    int checksum = 0;
    
    /* Run test functions multiple times with different parameters */
    for (int i = 0; i < 10; i++) {
        checksum += test_function(i, i*2, i*3);
        checksum += test_function2(i);
        checksum += test_function3(i);
        
        /* Modify globals to change behavior */
        g1 = (g1 * 3) % 100;
        g2 = (g2 * 5) % 100;
        g3 = (g3 * 7) % 100;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
