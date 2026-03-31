/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller-save-test.c -o caller-save-test
 */

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
void helper3(void) {
    /* Empty function just to create a call site */
    global_sink++;
}

/* Test function designed to force caller-save at basic block boundaries */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int var1 = param1 * 2;
    int var2 = param2 + 7;
    int var3 = param3 - 3;
    int var4 = var1 + var2;
    int var5 = var2 * var3;
    int var6 = var4 - var5;
    int var7 = var6 + 11;
    int var8 = var7 * 3;
    int var9 = var8 / 2;
    int var10 = var9 + 17;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* Use reg_var in computation before call */
    int pre_call = reg_var * 2 + global_source;
    
    /* SCENARIO 1: Call at end of if-branch basic block */
    if (param1 > 0) {
        /* Force reg_var to be live across this call */
        int result1 = helper1(reg_var, pre_call);
        
        /* This call is at the end of the basic block (before else) */
        helper3();
        
        /* BB_END update should happen here when inserting caller-save */
        /* After call, use reg_var - forcing save/restore */
        global_sink = reg_var + result1;
        
        return global_sink;  /* Return creates block boundary */
    } else {
        /* Different path to create control flow */
        int result2 = helper2(reg_var, var4, var5);
        global_sink = result2 - reg_var;
    }
    
    /* SCENARIO 2: Call at end of loop body basic block */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Modify reg_var inside loop */
        reg_var += i;
        
        /* Call with reg_var live */
        int loop_result = helper1(reg_var, var6 + i);
        
        /* This call is at the end of loop body basic block */
        helper3();
        
        /* BB_END update should happen here */
        /* Use reg_var after call */
        sum += reg_var + loop_result;
        
        /* Loop increment/jump creates new basic block */
    }
    
    /* SCENARIO 3: Call before return (end of function block) */
    if (param2 < 0) {
        int final_result = helper2(reg_var, sum, var10);
        
        /* Call as last instruction before return */
        helper3();
        
        /* BB_END update should happen here */
        /* Final use of reg_var */
        global_sink = reg_var + final_result;
        
        return global_sink;  /* Return creates block boundary */
    }
    
    /* Final computation using reg_var */
    reg_var = helper1(reg_var, sum);
    
    return reg_var + global_sink;
}

/* Another test with more complex control flow */
__attribute__((noinline, noclone))
int test_function2(int x) {
    /* Use different call-clobbered register */
    register int reg_var2 asm ("r13") = x * 3;
    
    /* Create switch statement for multiple basic blocks */
    switch (x % 4) {
        case 0: {
            int temp = helper1(reg_var2, 10);
            helper3();  /* Call at end of case block */
            /* BB_END update here */
            reg_var2 += temp;
            break;
        }
        case 1: {
            int temp = helper2(reg_var2, 5, 2);
            helper3();  /* Call at end of case block */
            /* BB_END update here */
            reg_var2 -= temp;
            /* Fall through */
        }
        case 2: {
            helper3();  /* Another call */
            /* BB_END update here */
            reg_var2 *= 2;
            break;
        }
        default: {
            int temp = helper1(reg_var2, reg_var2);
            helper3();  /* Call at end of default block */
            /* BB_END update here */
            reg_var2 = temp;
            break;
        }
    }
    
    return reg_var2;
}

int main(void) {
    int checksum = 0;
    
    /* Call test functions with different parameters
     * to exercise different paths and register pressure */
    checksum += test_function(1, 2, 3);
    checksum += test_function(-1, -2, -3);
    checksum += test_function(0, 100, -100);
    checksum += test_function(5, -5, 10);
    
    checksum += test_function2(0);
    checksum += test_function2(1);
    checksum += test_function2(2);
    checksum += test_function2(3);
    checksum += test_function2(7);
    
    /* Add some volatile operations to prevent optimization */
    global_sink = checksum;
    
    printf("Result: %d\n", checksum);
    
    /* Verify with a simple check */
    if (checksum != 0) {
        return 0;  /* Success */
    }
    return 1;  /* Shouldn't happen */
}
