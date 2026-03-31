/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For coverage analysis: add -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;
volatile int global_data[256];

/* Force real function calls (no inlining) */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    global_counter++;
    return a + b + global_counter;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    global_data[global_counter & 255] = a + b + c;
    return global_data[(global_counter - 1) & 255];
}

__attribute__((noinline, noclone))
void helper3(void) {
    /* Clobber many registers */
    asm volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
                   "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    global_counter++;
}

/* Main test function - designed to force caller-save at basic block boundaries */
__attribute__((noinline, noclone))
int test_caller_save(int iterations, int seed) {
    /* Create register pressure with many local variables */
    int v1 = seed + 1;
    int v2 = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = seed * 2;
    
    int result = 0;
    
    /* Loop creates basic blocks with calls at the end */
    for (int i = 0; i < iterations; i++) {
        /* Use reg_var in computation before call */
        int temp = reg_var + v1 + v2;
        
        /* Call at what could be end of basic block (before loop increment) */
        if (i & 1) {
            /* Call placed at end of if-block */
            reg_var = helper1(reg_var, temp);
            /* Basic block boundary likely after call */
            result += reg_var;
        } else {
            /* Different call, still at block end */
            reg_var = helper2(reg_var, v3, v4);
            result += reg_var;
        }
        
        /* Force reg_var to be live across helper3 call */
        v5 = reg_var + v6;
        helper3();  /* This call clobbers r12 */
        
        /* Use reg_var after call - forces save/restore */
        v7 = reg_var + v8;
        
        /* Another call at potential block end */
        if (v7 > 1000) {
            reg_var = helper1(reg_var, v9);
            /* This could be last insn in block before return */
            return reg_var + result;  /* Early return creates block boundary */
        }
        
        /* More computations to increase register pressure */
        v9 = v9 + v10;
        v10 = v10 * 2;
        reg_var = reg_var ^ v9;
    }
    
    /* Final call at end of function (before return) */
    reg_var = helper2(reg_var, result, v10);
    
    /* This is the key scenario: call at end of basic block */
    if (result > 1000000) {
        /* Dead code path to create conditional block */
        helper3();
        return 0;
    }
    
    /* Function call as last instruction before return */
    result = helper1(reg_var, result);
    return result;  /* Basic block ends with call then return */
}

/* Second test with different control flow */
__attribute__((noinline, noclone))
int test_boundary_calls(int mode) {
    register int critical asm ("r13") = mode * 100;
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6, g = 7, h = 8;
    
    /* Switch creates multiple basic blocks */
    switch (mode & 3) {
        case 0:
            critical = helper1(critical, a);
            /* Call at end of case block */
            global_sink = critical;
            break;  /* Block boundary after break */
            
        case 1:
            critical = helper2(critical, b, c);
            /* Multiple calls in sequence */
            d = helper1(d, e);
            critical += d;
            /* Last instruction in block is a call */
            helper3();
            break;
            
        case 2:
            /* Call at end of if-block inside case */
            if (critical > 50) {
                critical = helper1(critical, f);
                /* This call is at end of if-then block */
                g = helper2(g, h, critical);
                return g;  /* Return creates block boundary */
            }
            critical = helper2(critical, g, h);
            break;
            
        default:
            /* Loop with call at end of body */
            for (int i = 0; i < 5; i++) {
                critical += i;
                /* Call at end of loop body (before increment) */
                helper3();
            }
            break;
    }
    
    /* Final use of critical register */
    return helper1(critical, global_sink);
}

int main(void) {
    int total = 0;
    
    printf("Testing caller-save at basic block boundaries...\n");
    
    /* Multiple calls to exercise different paths */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(10, i * 100);
        total += test_boundary_calls(i % 4);
        
        /* Create varying register pressure */
        global_data[i & 255] = total;
    }
    
    /* Use results to prevent optimization */
    volatile int sink = total;
    printf("Result: %d (sink: %d)\n", total, sink);
    
    return 0;
}
