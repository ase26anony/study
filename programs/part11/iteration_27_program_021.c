/* caller-save-test.c
 * Test program to trigger caller-save register spills at basic block boundaries
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
 * For coverage instrumentation: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 42;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int x, int y) {
    return x * y + global_source;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    return (a - b) * c;
}

__attribute__((noinline, noclone))
void helper3(void) {
    /* Empty function just to create a call site */
    global_sink++;
}

/* Test function that forces caller-save spills at block boundaries */
__attribute__((noinline, noclone))
int test_caller_save(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int var1 = param1 * 2;
    int var2 = param2 + global_source;
    int var3 = var1 - var2;
    int var4 = param3 * 3;
    int var5 = var2 / (param1 ? param1 : 1);
    int var6 = var3 + var4;
    int var7 = var5 * 2;
    int var8 = var6 - var7;
    int var9 = var8 + global_sink;
    int var10 = var9 * param2;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* Case 1: Call at end of if-block before return */
    if (param1 > 10) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call at the end of basic block */
        result = helper1(temp, var4);
        
        /* This return creates a basic block boundary right after the call */
        return result + reg_var;  /* reg_var must be preserved across call */
    }
    
    /* Case 2: Call at end of loop body */
    for (int i = 0; i < param2; i++) {
        /* Update reg_var in loop */
        reg_var += i * var5;
        
        /* Another variable to increase register pressure */
        int loop_var = var6 * i;
        
        /* Function call as last statement in loop body */
        result += helper2(reg_var, loop_var, var7);
        
        /* Loop increment/jump creates basic block boundary */
        /* reg_var must be preserved across helper2 call */
    }
    
    /* Case 3: Call before conditional return */
    if (param3 < 5) {
        /* Multiple uses of reg_var to keep it live */
        int a = reg_var * 3;
        int b = a + var8;
        
        /* Call at end of basic block */
        helper3();
        
        /* Return creates block boundary */
        return b + reg_var;
    }
    
    /* Case 4: Call in switch case at block end */
    switch (param1 % 3) {
        case 0:
            reg_var += var9;
            /* Call at end of case block */
            result = helper1(reg_var, var10);
            /* Fall through creates block boundary */
            break;
        case 1:
            reg_var -= var10;
            /* Another call */
            result = helper2(reg_var, var9, var8);
            /* Break creates block boundary */
            break;
        default:
            reg_var *= 2;
            result = reg_var;
            break;
    }
    
    /* Final use to ensure reg_var is live */
    global_sink = reg_var + result;
    
    return result;
}

/* Another test with different register pressure */
__attribute__((noinline, noclone))
int test_boundary_calls(int iterations) {
    /* Use multiple explicit register variables */
    register int r1 asm ("r10") = global_source;
    register int r2 asm ("r11") = global_source * 2;
    register int r3 asm ("r12") = global_source * 3;
    register int r4 asm ("r13") = global_source * 4;
    
    int sum = 0;
    
    /* Loop with calls at different positions */
    for (int i = 0; i < iterations; i++) {
        /* Rotate register values to force spills */
        int temp = r1;
        r1 = r2 + i;
        r2 = r3 * temp;
        r3 = r4 - i;
        r4 = temp + r1;
        
        /* Call at end of loop body - basic block boundary */
        if (i % 2 == 0) {
            sum += helper1(r1, r2);
            /* r1, r2 must be preserved */
        } else {
            sum += helper2(r3, r4, i);
            /* r3, r4 must be preserved */
        }
        
        /* Additional computation after call */
        r1 += sum;
        r2 -= sum;
    }
    
    /* Final call at end of function */
    helper3();
    
    return sum + r1 + r2 + r3 + r4;
}

/* Test with nested control flow */
__attribute__((noinline, noclone))
int test_nested_blocks(int x, int y) {
    register int reg1 asm ("r12") = x;
    register int reg2 asm ("r13") = y;
    int result = 0;
    
    if (x > 0) {
        if (y > 0) {
            reg1 *= 2;
            /* Call at end of inner if block */
            result = helper1(reg1, reg2);
            /* Block boundary: jump to outer block */
        } else {
            reg2 *= 3;
            /* Another call */
            result = helper2(reg1, reg2, x);
            /* Block boundary */
        }
        
        /* Use both registers after calls */
        result += reg1 - reg2;
    } else {
        /* Different path with call at end */
        reg1 = helper1(y, x);
        /* Return creates block boundary */
        return reg1;
    }
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Test various scenarios to exercise different paths */
    printf("Testing caller-save optimization...\n");
    
    /* Test 1: Call at end of if-block */
    total += test_caller_save(15, 5, 2);   /* Takes if (param1 > 10) path */
    
    /* Test 2: Calls in loop */
    total += test_caller_save(5, 8, 10);   /* Takes loop path */
    
    /* Test 3: Call before conditional return */
    total += test_caller_save(7, 3, 2);    /* Takes param3 < 5 path */
    
    /* Test 4: Switch cases */
    total += test_caller_save(6, 4, 8);    /* Takes switch path */
    total += test_caller_save(7, 4, 8);    /* Different switch case */
    
    /* Test 5: Multiple register variables */
    total += test_boundary_calls(10);
    
    /* Test 6: Nested control flow */
    total += test_nested_blocks(5, 3);
    total += test_nested_blocks(-2, 4);
    
    /* Use volatile sink to prevent optimization */
    global_sink = total;
    
    printf("Result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
