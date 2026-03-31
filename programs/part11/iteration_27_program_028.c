/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -fdump-rtl-all caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int g1 = 1;
volatile int g2 = 2;
volatile int g3 = 3;
volatile int g4 = 4;
volatile int g5 = 5;
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

__attribute__((noinline, noclone)) int helper4(int a, int b, int c, int d, int e) {
    return a * b + c * d - e + g4;
}

/* Test function that forces caller-save at basic block boundaries */
__attribute__((noinline, noclone)) 
int test_caller_save(int x, int y, int z) {
    /* Create register pressure with many variables */
    int v1 = x + y;
    int v2 = y * z;
    int v3 = z - x;
    int v4 = v1 * v2;
    int v5 = v2 + v3;
    int v6 = v3 * v4;
    int v7 = v4 - v5;
    int v8 = v5 * v6;
    int v9 = v6 + v7;
    int v10 = v7 * v8;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2 + v3 + g5;
    
    /* Force reg_var to be live across multiple calls */
    int result = 0;
    
    /* Scenario 1: Call at end of if-branch basic block */
    if (x > 0) {
        /* Use reg_var before call */
        int tmp1 = reg_var * 2;
        
        /* Call at what could be end of basic block */
        result += helper1(tmp1, v4);
        
        /* Basic block boundary: implicit jump to merge point */
    } else {
        result += helper2(v5, v6, v7);
    }
    
    /* reg_var is still live here */
    result += reg_var;
    
    /* Scenario 2: Call at end of loop body basic block */
    for (int i = 0; i < 3; i++) {
        /* Use reg_var in computation */
        int tmp2 = reg_var + i * 10;
        
        /* Multiple calls to increase pressure */
        if (i == 0) {
            result += helper3(tmp2, v8, v9, v10);
        } else if (i == 1) {
            /* Call as last statement before loop increment */
            result += helper4(tmp2, v9, v10, v1, v2);
            
            /* This could be end of basic block before loop latch */
        } else {
            result += helper1(tmp2, v3);
        }
        
        /* Loop increment/latch is separate basic block */
    }
    
    /* Scenario 3: Call just before return (end of function BB) */
    if (y > 0) {
        int tmp3 = reg_var * 3 + result;
        result = helper2(tmp3, v4, v5);
        /* Return would be separate BB or same BB depending on optimization */
    }
    
    /* Force use of reg_var after all calls */
    sink = reg_var;  /* Prevent dead store elimination */
    
    return result + reg_var;
}

/* Another test with different control flow */
__attribute__((noinline, noclone))
int test_boundary_calls(int a, int b) {
    /* More register pressure */
    int r1 = a * 2;
    int r2 = b * 3;
    int r3 = r1 + r2;
    int r4 = r1 * r2;
    int r5 = r2 - r1;
    int r6 = r3 * r4;
    int r7 = r4 + r5;
    int r8 = r5 * r6;
    int r9 = r6 - r7;
    int r10 = r7 * r8;
    
    /* Another register variable */
    register int live_reg asm ("r13") = r1 + r3 + r5 + g1;
    
    int sum = 0;
    
    /* Switch with calls at case ends */
    switch (a % 4) {
        case 0:
            sum += helper1(live_reg, r2);
            live_reg += 10;
            /* Fall through - creates BB boundary */
        case 1:
            sum += helper2(live_reg, r3, r4);
            /* BB ends with break */
            break;
        case 2:
            sum += helper3(live_reg, r5, r6, r7);
            /* Call as last thing before break */
            sum += helper4(live_reg, r8, r9, r10, r1);
            break;
        default:
            sum += helper1(live_reg, r2);
            /* Return creates BB end */
            sink = live_reg;
            return sum + live_reg;
    }
    
    /* Use after switch */
    sum += live_reg * 2;
    
    /* Call at end of another BB before return */
    if (b > 0) {
        sum = helper3(sum, live_reg, r1, r2);
        /* BB could end here before function epilogue */
    }
    
    sink = live_reg;
    return sum + live_reg;
}

int main(void) {
    int total = 0;
    
    /* Exercise different paths */
    for (int i = 0; i < 10; i++) {
        total += test_caller_save(i, i*2, i*3);
        total += test_boundary_calls(i, i+1);
    }
    
    /* Use volatile sink to prevent optimization */
    printf("Result: %d (sink: %d)\n", total, sink);
    
    return 0;
}
