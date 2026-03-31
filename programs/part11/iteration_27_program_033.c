/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all -fdump-rtl-caller_save caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>

/* Volatile globals to prevent optimization */
volatile int global_sink = 0;
volatile int global_source = 42;

/* Noinline helper functions to ensure real calls */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    return a * b + global_source;
}

__attribute__((noinline, noclone)) 
int helper2(int a, int b, int c) {
    return a - b * c;
}

__attribute__((noinline, noclone))
void helper3(void) {
    global_sink++;
}

/* Main test function - also noinline to preserve structure */
__attribute__((noinline, noclone))
int test_function(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int var1 = param1 * 2;
    int var2 = param2 + 7;
    int var3 = param3 - 5;
    int var4 = var1 * var2;
    int var5 = var2 / (param3 ? param3 : 1);
    int var6 = var3 + var4;
    int var7 = var5 * 3;
    int var8 = var6 - var7;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = var1 + var2 + var3;
    
    /* More variables to increase register pressure */
    int var9 = var4 * 2;
    int var10 = var5 + 11;
    int var11 = var6 - 13;
    int var12 = var7 * 17;
    
    /* First basic block: computation before conditional */
    int result = 0;
    
    /* Create a basic block boundary after the call */
    if (param1 > param2) {
        /* Use reg_var before call */
        int pre_call = reg_var * 2;
        
        /* Function call at the end of basic block - followed by return */
        int call_result = helper1(pre_call, var4);
        
        /* This should force caller-save spill at BB end */
        global_sink = call_result + reg_var;  /* reg_var live across call */
        
        /* Return immediately after call - creates BB boundary */
        return global_sink + var8;
    } else {
        /* Alternative path with loop to create different BB structure */
        int sum = 0;
        for (int i = 0; i < param3; i++) {
            /* More register pressure inside loop */
            int loop_var1 = var9 + i;
            int loop_var2 = var10 * i;
            
            /* Use reg_var in loop */
            int loop_compute = reg_var + loop_var1;
            
            /* Function call at end of loop body */
            int loop_result = helper2(loop_compute, loop_var2, var11);
            
            /* Force reg_var to be live across call */
            sum += loop_result + reg_var;  /* reg_var must be preserved */
            
            /* Loop increment creates BB boundary after call */
        }
        
        /* Another call before return in else branch */
        helper3();
        
        return sum + var12;
    }
}

/* Second test function with different BB structure */
__attribute__((noinline, noclone))
int test_function2(int iterations) {
    /* Bind to another call-clobbered register */
    register int reg_var2 asm ("r13") = global_source;
    
    int acc = 0;
    
    /* Loop where call is at the end of loop body */
    for (int i = 0; i < iterations; i++) {
        /* Complex computation to use many registers */
        int a = i * 3;
        int b = i + 5;
        int c = i - 2;
        int d = a * b;
        int e = b + c;
        
        /* Make reg_var2 live across the call */
        int pre_value = reg_var2 * d;
        
        /* Call at what could be BB end if loop is optimized a certain way */
        int call_val = helper1(pre_value, e);
        
        /* Use reg_var2 after call */
        acc += call_val + reg_var2;
        
        /* Loop tail could be separate BB */
        if (i % 2 == 0) {
            reg_var2 += call_val;  /* Modify register variable */
        }
    }
    
    /* Final call right before return */
    int final = helper2(acc, reg_var2, 100);
    return final;
}

/* Third test: switch statement creates multiple BBs ending with calls */
__attribute__((noinline, noclone))
int test_function3(int choice) {
    register int reg_var3 asm ("r14") = choice * 10;
    
    switch (choice % 4) {
        case 0: {
            /* BB ending with call */
            int temp = reg_var3 + 5;
            int result = helper1(temp, 2);
            global_sink = result + reg_var3;
            /* Fall through to return */
            break;
        }
        case 1: {
            /* Different BB also ending with call */
            int temp = reg_var3 * 3;
            helper3();
            /* Call is last in BB before break */
            return temp + reg_var3;
        }
        case 2: {
            /* BB with call then immediate return */
            int result = helper2(reg_var3, 7, 8);
            return result + reg_var3;
        }
        default: {
            /* Multiple calls in sequence */
            int a = helper1(reg_var3, 1);
            int b = helper2(a, reg_var3, 2);
            /* Last call before return */
            helper3();
            return b + reg_var3;
        }
    }
    
    return reg_var3;
}

int main(void) {
    int total = 0;
    
    /* Test different paths to exercise various BB structures */
    for (int i = 0; i < 10; i++) {
        total += test_function(i, i*2, i+3);
        total += test_function2(i+1);
        total += test_function3(i);
    }
    
    printf("Result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    /* Verify with a checksum */
    int checksum = total ^ global_sink;
    if (checksum != 0) {
        printf("Checksum: %d\n", checksum);
    }
    
    return 0;
}
