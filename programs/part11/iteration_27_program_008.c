/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
 * For coverage instrumentation: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g_volatile1 = 12345;
volatile int g_volatile2 = 67890;
volatile int g_sink = 0;

/* Helper functions that won't be inlined */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    return a + b + g_volatile1;
}

__attribute__((noinline, noclone)) 
int helper2(int a, int b, int c) {
    return a * b - c + g_volatile2;
}

__attribute__((noinline, noclone))
void helper3(int *ptr) {
    *ptr += g_volatile1;
}

/* Force register pressure and specific register usage */
#ifdef __x86_64__
#define CALL_CLOBBERED_REG "r12"
#elif defined(__i386__)
#define CALL_CLOBBERED_REG "ebx"
#else
/* For other architectures, adjust as needed */
#define CALL_CLOBBERED_REG ""
#endif

/* Main test function - noinline to ensure it's analyzed separately */
__attribute__((noinline, noipa))
int test_caller_save(int param1, int param2, int param3) {
    /* Create register pressure with many local variables */
    int var1 = param1 * 2;
    int var2 = param2 + 100;
    int var3 = param3 - 50;
    int var4 = var1 + var2;
    int var5 = var2 * var3;
    int var6 = var4 - var5;
    int var7 = var6 / 3;
    int var8 = var7 + 999;
    int var9 = var8 * 2;
    int var10 = var9 - 777;
    
    /* Explicit register variable bound to call-clobbered register */
    register int reg_var asm(CALL_CLOBBERED_REG) = var1 + var2 + var3;
    
    /* Force reg_var to be live across multiple calls */
    int result1 = 0;
    int result2 = 0;
    
    /* Scenario 1: Call at end of basic block before return from if branch */
    if (param1 > 100) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        /* Function call as last statement in if block - creates basic block end */
        result1 = helper1(temp, var4);
        
        /* This assignment would be in a different basic block */
        reg_var = result1 + 10;
        
        /* Force use after call */
        g_sink = reg_var;
        
        return reg_var + result1;
    }
    
    /* Scenario 2: Call at end of loop body */
    for (int i = 0; i < 3; i++) {
        /* Compute with reg_var */
        reg_var = reg_var + var5 + i;
        
        /* Another local to increase pressure */
        int local_pressure = var6 * i + var7;
        
        /* Call at end of loop body - basic block ends after call */
        result2 = helper2(reg_var, local_pressure, var8);
        
        /* Loop increment/jump is in different basic block */
    }
    
    /* Scenario 3: Call before return in else branch */
    if (param2 < 50) {
        reg_var = reg_var * 3;
        g_sink = reg_var;
    } else {
        /* Call as last statement before return */
        helper3(&reg_var);
        /* Basic block ends here, return is separate */
    }
    
    /* Final use ensures reg_var must be preserved */
    g_sink = reg_var + result2;
    
    return reg_var + result1 + result2;
}

/* Second test with more complex control flow */
__attribute__((noinline, noipa))
int test_boundary_calls(int iterations) {
    int sum = 0;
    
    /* Use explicit register variable */
    register int counter asm(CALL_CLOBBERED_REG) = iterations;
    
    for (int i = 0; i < iterations; i++) {
        int a = i * 2;
        int b = i + 5;
        
        /* Multiple variables to increase register pressure */
        int c = a * b;
        int d = c - i;
        int e = d / 2;
        int f = e + 100;
        int g = f * 3;
        int h = g - 50;
        
        /* Live range across call */
        counter = counter + a + b + c;
        
        if (i % 2 == 0) {
            /* Call at end of if block */
            int res = helper1(counter, h);
            sum += res;
            /* counter remains live */
        } else {
            /* Different call, still at block end */
            int res = helper2(counter, g, f);
            sum -= res;
        }
        
        /* Counter used after the if-else, must be preserved */
        counter = counter - d + e;
        
        /* Another call at loop end for some iterations */
        if (i % 3 == 0) {
            helper3(&counter);
        }
    }
    
    g_sink = sum + counter;
    return sum;
}

int main() {
    int total = 0;
    
    /* Test various paths to exercise different basic block structures */
    total += test_caller_save(150, 30, 200);  /* Takes if branch */
    total += test_caller_save(50, 70, 100);   /* Takes else branch */
    total += test_caller_save(0, 10, 20);     /* Takes different path */
    
    total += test_boundary_calls(5);
    total += test_boundary_calls(10);
    
    /* Use result to prevent optimization */
    printf("Result: %d (sink: %d)\n", total, g_sink);
    
    return 0;
}
