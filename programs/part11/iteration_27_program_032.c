/* caller-save-test.c - Test program to trigger caller-save register spills at basic block boundaries */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;
volatile int global_result = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int a, int b) {
    global_counter++;
    return a + b + global_counter;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    global_counter += 2;
    return a * b - c + global_counter;
}

__attribute__((noinline, noclone))
int helper3(int a) {
    global_counter += 3;
    return a * 7 + global_counter;
}

/* Test function with register pressure and calls at block boundaries */
__attribute__((noinline, noclone, optimize("O2")))
int test_caller_save(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int var1 = param1 * 2;
    int var2 = param2 + 5;
    int var3 = param3 - 3;
    int var4 = var1 + var2;
    int var5 = var2 * var3;
    int var6 = var3 / 2;
    int var7 = var4 ^ var5;
    int var8 = var5 | var6;
    int var9 = var6 & var7;
    int var10 = var7 + var8;
    
    /* Explicit register variable bound to a call-clobbered register */
    /* Using r12 on x86_64 which is call-clobbered */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* First call - placed at end of basic block (before return in if branch) */
    if (param1 > 0) {
        /* Multiple uses of reg_var to keep it live */
        reg_var = reg_var * 2 + 1;
        
        /* Call at end of basic block - will be followed by return */
        int tmp = helper1(reg_var, var4);
        
        /* Use reg_var after call - forces caller-save */
        result = tmp + reg_var;
        
        /* This return creates basic block boundary after call */
        return result;
    }
    
    /* Second scenario - call at end of loop body */
    reg_var = reg_var + var5;
    
    for (int i = 0; i < 3; i++) {
        /* Use reg_var in computation */
        int compute = reg_var * i + var6;
        
        /* Call at end of loop body - basic block ends after call */
        int tmp = helper2(reg_var, compute, var7);
        
        /* Use reg_var after call */
        result += tmp + reg_var;
        
        /* Loop increment/jump creates new basic block */
        reg_var = reg_var + tmp;
    }
    
    /* Third scenario - call before conditional jump */
    reg_var = reg_var ^ var8;
    
    if (param2 < 0) {
        /* Call before jump to else branch */
        int tmp = helper3(reg_var);
        result += tmp;
        
        /* Jump to else branch creates basic block boundary */
        if (tmp > 100) {
            result += reg_var * 2;
        } else {
            result += reg_var / 2;
        }
    } else {
        result -= reg_var;
    }
    
    /* Fourth scenario - call as last statement before switch */
    reg_var = reg_var | var9;
    int tmp = helper1(reg_var, var10);
    
    switch (param3 % 3) {
        case 0:
            result += tmp + reg_var;
            break;
        case 1:
            result += tmp - reg_var;
            break;
        case 2:
            result += tmp * reg_var;
            break;
    }
    
    /* Final use to ensure reg_var stays live */
    global_sink = reg_var;
    
    return result;
}

/* Another test with different register binding */
__attribute__((noinline, noclone, optimize("O3")))
int test_caller_save2(int iterations) {
    /* Use r13 on x86_64 (also call-clobbered) */
    register int counter asm ("r13") = 0;
    register int accumulator asm ("r12") = 1;
    
    /* Create artificial register pressure */
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    int i = 9, j = 10, k = 11, l = 12, m = 13, n = 14, o = 15, p = 16;
    
    for (int idx = 0; idx < iterations; idx++) {
        /* Use all variables to keep them live */
        a = b + c;
        b = c + d;
        c = d + e;
        d = e + f;
        e = f + g;
        f = g + h;
        g = h + i;
        h = i + j;
        i = j + k;
        j = k + l;
        k = l + m;
        l = m + n;
        m = n + o;
        n = o + p;
        o = p + a;
        p = a + b;
        
        /* Update register variables */
        counter++;
        accumulator = accumulator * 2 + counter;
        
        /* Call at end of loop body - basic block boundary */
        if (idx % 2 == 0) {
            int tmp = helper2(accumulator, counter, a);
            accumulator += tmp;
        } else {
            int tmp = helper3(accumulator);
            accumulator -= tmp;
        }
        
        /* Use accumulator after call */
        global_sink = accumulator;
    }
    
    return accumulator + counter;
}

/* Main function to drive tests */
int main() {
    int total = 0;
    
    printf("Testing caller-save register spills at basic block boundaries...\n");
    
    /* Run multiple test cases to exercise different paths */
    for (int i = 0; i < 10; i++) {
        int result1 = test_caller_save(i, i - 5, i * 2);
        int result2 = test_caller_save2(i + 1);
        
        total += result1 + result2;
        global_result = result1 ^ result2;
        
        printf("Iteration %d: result1=%d, result2=%d, total=%d\n", 
               i, result1, result2, total);
    }
    
    /* Use results to prevent optimization */
    volatile int final_check = total + global_result + global_sink;
    
    printf("Final checksum: %d\n", final_check);
    printf("Test completed.\n");
    
    return (final_check != 0) ? 0 : 1;
}
