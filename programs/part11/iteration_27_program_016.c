/* caller-save-test.c
 * Test program to trigger caller-save register insertion at basic block ends
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing caller-save-test.c -o caller-save-test
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
int helper3(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    global_counter += sum;
    return sum;
}

/* Main test function that forces caller-save at block boundaries */
__attribute__((noinline, noclone))
int test_caller_save(int mode, int iterations) {
    /* Create register pressure with many local variables */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = mode * 100;
    
    /* More variables to increase register pressure */
    int arr[8];
    int sum = 0;
    
    /* Initialize array with volatile data to prevent constant propagation */
    for (int i = 0; i < 8; i++) {
        arr[i] = global_data[i] + i;
    }
    
    /* Force reg_var to be live across multiple calls at block boundaries */
    for (int i = 0; i < iterations; i++) {
        /* Use reg_var before call */
        v1 = reg_var + i;
        v2 = reg_var * 2;
        
        /* Call at the end of a basic block (before loop increment) */
        if (mode & 1) {
            /* This call is at the end of the if-block basic block */
            v3 = helper1(v1, v2);
            /* Basic block boundary after the call (implicit) */
        } else {
            v3 = helper2(v1, v2, reg_var);
        }
        
        /* Use reg_var after call - forces save/restore */
        reg_var = v3 + reg_var + global_counter;
        
        /* Another call at block boundary before return in conditional */
        if (i == iterations - 1) {
            /* Call as last statement before return in if-block */
            sum = helper3(arr, 8);
            /* This creates a basic block ending with the call */
            global_sink += sum;
            return reg_var + sum;
        }
        
        /* Complex computation to increase register pressure */
        v4 = helper1(reg_var, i);
        v5 = helper2(v4, reg_var, arr[i & 7]);
        
        /* Force multiple register uses */
        reg_var = (reg_var * 1103515245 + 12345) & 0x7fffffff;
        arr[i & 7] = reg_var;
        
        /* Call at end of loop body (before loop increment) */
        if ((i & 3) == 0) {
            v6 = helper1(arr[0], arr[4]);
            /* Basic block ends with call, next is loop increment */
        }
    }
    
    /* Final call at block boundary before function return */
    sum = helper2(reg_var, global_sink, mode);
    /* This call is at the end of the basic block before return */
    
    global_sink += sum;
    return reg_var + sum;
}

/* Another test with different control flow */
__attribute__((noinline, noclone))
int test_boundary_calls(int x) {
    /* Use explicit register variable */
    register int r_var asm ("r13") = x;
    int temp;
    
    /* Multiple basic blocks with calls at the end */
    if (x > 100) {
        /* Call at end of if-block */
        temp = helper1(r_var, x);
        r_var = temp * 2;
        /* Basic block ends, jumps to merge point */
    } else if (x > 50) {
        /* Another call at end of else-if block */
        temp = helper2(r_var, x, x * 2);
        r_var = temp / 2;
        /* Basic block ends */
    } else {
        /* Call at end of else block */
        temp = helper3(&x, 1);
        r_var = temp + 5;
        /* Basic block ends */
    }
    
    /* Use r_var after conditional - forces save/restore across calls */
    global_sink += r_var;
    
    /* Switch statement creates multiple basic blocks */
    switch (x % 4) {
        case 0:
            /* Call at end of case block */
            temp = helper1(r_var, 1);
            r_var = temp;
            break;  /* Basic block boundary */
            
        case 1:
            /* Call at end of case block */
            temp = helper2(r_var, 2, 3);
            r_var = temp;
            break;
            
        case 2:
            /* Call as last statement before break */
            r_var = helper3(&global_sink, 1);
            break;
            
        default:
            /* Call at end of default block */
            temp = helper1(r_var, r_var);
            r_var = temp * 3;
            break;
    }
    
    /* Final computation */
    return r_var + global_counter;
}

/* Test with nested loops */
__attribute__((noinline, noclone))
int test_loop_boundaries(int n) {
    register int loop_var asm ("r14") = n;
    int i, j;
    int total = 0;
    
    for (i = 0; i < n; i++) {
        /* Inner loop creates more basic blocks */
        for (j = 0; j < 3; j++) {
            /* Call at end of inner loop body */
            int result = helper1(loop_var, j);
            total += result;
            
            /* Update register variable - live across call */
            loop_var = (loop_var * 1664525 + 1013904223) & 0x7fffffff;
            
            /* Call at end of basic block (before inner loop increment) */
            if (j == 1) {
                total += helper2(loop_var, i, total);
                /* Basic block ends with call */
            }
        }
        
        /* Call at end of outer loop body (before outer loop increment) */
        if (i % 2 == 0) {
            total += helper3(&total, 1);
            /* Basic block ends with call */
        }
        
        /* Force spill by using many variables */
        int a = loop_var + i;
        int b = a * 2;
        int c = b + total;
        int d = helper1(a, b);
        int e = helper2(c, d, loop_var);
        
        loop_var = e + (loop_var >> 1);
    }
    
    /* Final call at block boundary before return */
    total += helper1(loop_var, total);
    return total;
}

int main() {
    int result = 0;
    
    /* Initialize volatile data */
    for (int i = 0; i < 256; i++) {
        global_data[i] = i * 3 + 1;
    }
    
    /* Run tests multiple times to exercise different paths */
    for (int i = 0; i < 10; i++) {
        result ^= test_caller_save(i, 5);
        result ^= test_boundary_calls(i * 20);
        result ^= test_loop_boundaries(3 + (i % 5));
        
        /* Add some global side effects */
        global_counter += i;
        global_sink += result;
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    printf("Global sink: %d\n", global_sink);
    
    return 0;
}
