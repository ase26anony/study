/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all caller-save-test.c 2>&1 | grep -i caller_save
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_vol1 = 1;
volatile int g_vol2 = 2;
volatile int g_vol3 = 3;
volatile int sink = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone)) 
int helper1(int a, int b) {
    return a + b + g_vol1;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    return a * b - c + g_vol2;
}

__attribute__((noinline, noclone)) 
int helper3(int a) {
    return a ^ g_vol3;
}

/* Complex function with register pressure and calls at block boundaries */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create many local variables to increase register pressure */
    int v1 = param1 * 2;
    int v2 = param2 + 7;
    int v3 = param3 - 5;
    int v4 = v1 ^ v2;
    int v5 = v2 & v3;
    int v6 = v3 | v1;
    int v7 = v4 + v5;
    int v8 = v5 - v6;
    int v9 = v6 * v7;
    int v10 = v7 / (v8 ? v8 : 1);
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2 + v3;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* First call site: call at end of if-block before return */
    if (param1 > 0) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Call at what will become end of basic block */
        result = helper1(temp, v4);
        
        /* This return creates block boundary right after call */
        return result + reg_var;  /* reg_var must be preserved across call */
    }
    
    /* Second call site: call at end of loop body */
    for (int i = 0; i < param2; i++) {
        /* Complex computation using reg_var */
        reg_var = reg_var + v5 + i;
        v6 = helper2(reg_var, v6, v7);
        
        /* Call as last statement in loop body - creates block boundary */
        v8 = helper3(v6);
        
        /* Loop increment/jump creates new basic block */
    }
    
    /* Third call site: call before conditional return in switch */
    switch (param3 & 3) {
        case 0:
            v9 = helper1(reg_var, v9);
            /* Fall through creates block boundary */
        case 1:
            v10 = helper2(v9, v10, reg_var);
            break;
        case 2:
            reg_var = helper3(reg_var);
            /* Return creates block boundary */
            return reg_var + v10;
        default:
            v7 = helper1(v10, reg_var);
            /* Break creates block boundary */
            break;
    }
    
    /* Use reg_var after all calls */
    sink = reg_var;  /* Force reg_var to be live */
    
    /* More register pressure */
    int v11 = v7 + v8;
    int v12 = v9 - v10;
    int v13 = v11 * v12;
    int v14 = v12 / (v13 ? v13 : 1);
    
    /* Final call that clobbers registers */
    result = helper2(v13, v14, reg_var);
    
    return result + reg_var;
}

/* Another test with different pattern */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Bind to another call-clobbered register */
    register int counter asm ("r13") = iterations;
    register int accumulator asm ("r14") = 0;
    
    volatile int* volatile_ptr = &sink;
    
    /* Loop with call at end of body */
    for (int i = 0; i < iterations; i++) {
        /* Use both register variables */
        accumulator += counter * i;
        
        /* Call that clobbers registers, placed at block end */
        int temp = helper1(accumulator, counter);
        
        /* Update counter after call - must be preserved */
        counter = temp + i;
        
        /* Loop backedge creates new basic block */
    }
    
    /* Another call at block boundary */
    if (accumulator > 1000) {
        *volatile_ptr = helper2(accumulator, counter, iterations);
        /* Return creates block boundary */
        return accumulator;
    }
    
    /* Final computation using both register variables */
    accumulator = helper3(counter);
    *volatile_ptr = accumulator;
    
    return accumulator + counter;
}

int main() {
    int checksum = 0;
    
    /* Test multiple paths to exercise different block boundaries */
    for (int i = 0; i < 10; i++) {
        checksum += test_function(i, i * 2, i * 3);
        checksum += test_function2(i + 5);
        
        /* Modify volatiles to change behavior */
        g_vol1 ^= i;
        g_vol2 += i;
        g_vol3 -= i;
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
