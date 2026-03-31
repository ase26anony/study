/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For coverage analysis: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;
volatile int global_data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

/* Force real function calls (no inlining) */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    return a + b + global_counter;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    global_counter++;
    return a * b + c;
}

__attribute__((noinline, noclone))
void helper3(void) {
    /* Clobber many registers */
    asm volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                  "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
}

/* Main test function - also noinline to preserve structure */
__attribute__((noinline, noclone))
int test_caller_save(int param1, int param2) {
    /* Create register pressure with many local variables */
    int var1 = param1 + 1;
    int var2 = param2 * 2;
    int var3 = global_data[0];
    int var4 = global_data[1];
    int var5 = global_data[2];
    int var6 = global_data[3];
    int var7 = global_data[4];
    int var8 = global_data[5];
    int var9 = global_data[6];
    int var10 = global_data[7];
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12");
    reg_var = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result1 = 0;
    int result2 = 0;
    
    /* Scenario 1: Call at end of basic block before return */
    if (param1 > 100) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call as last statement in if block */
        result1 = helper1(temp, var4);
        
        /* This creates a basic block ending with the call */
        return result1 + reg_var;  /* reg_var must be live here */
    }
    
    /* Scenario 2: Call at end of loop body */
    for (int i = 0; i < 3; i++) {
        /* Update reg_var inside loop */
        reg_var += var5 + i;
        
        /* Function call as last statement in loop body */
        result2 = helper2(reg_var, var6, var7);
        
        /* Loop increment/jump creates new basic block after call */
        /* reg_var must be preserved across the call */
    }
    
    /* Scenario 3: Call before conditional return */
    if (param2 < 50) {
        int temp2 = reg_var - var8;
        
        /* Call followed by immediate return */
        int result3 = helper1(temp2, var9);
        return result3 + reg_var;  /* reg_var live across call */
    }
    
    /* Mix in another call that clobbers registers */
    helper3();
    
    /* Force use of reg_var after all calls */
    global_sink = reg_var + var10;
    
    return result2 + global_sink;
}

/* Another test with different control flow */
__attribute__((noinline, noclone))
int test_boundary_calls(int seed) {
    /* More register variables to increase pressure */
    register int r13_var asm ("r13");
    register int r14_var asm ("r14");
    
    r13_var = seed * 3;
    r14_var = seed + 100;
    
    volatile int* volatile_ptr = &global_counter;
    
    /* Complex control flow with calls at block boundaries */
    for (int j = 0; j < 5; j++) {
        int condition = (*volatile_ptr) & (1 << j);
        
        if (condition) {
            /* Call at end of if block */
            int temp = helper1(r13_var, r14_var);
            r13_var = temp + j;
            
            /* Basic block ends with assignment, next is loop increment */
        } else {
            /* Different call pattern */
            r14_var = helper2(r13_var, j, *volatile_ptr);
            
            /* Insert a label to force block boundary */
            if (j == 3) {
                goto special_case;
            }
        }
        
        /* Use both register variables */
        *volatile_ptr += r13_var - r14_var;
    }
    
special_case:
    /* Final call right before return */
    int final_result = helper1(r13_var, r14_var);
    return final_result;
}

int main(void) {
    int total = 0;
    
    /* Test multiple paths to exercise different basic block structures */
    for (int i = 0; i < 100; i++) {
        global_counter = i;
        
        /* Vary parameters to take different code paths */
        total += test_caller_save(i, i * 2);
        total += test_boundary_calls(i);
        
        /* Alternate between high and low values */
        if (i % 2 == 0) {
            total += test_caller_save(i * 10, i / 2);
        } else {
            total += test_caller_save(i / 2, i * 10);
        }
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
