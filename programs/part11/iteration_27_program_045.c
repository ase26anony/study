/* caller-save-test.c
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-inline -fno-strict-aliasing -mtune=generic caller-save-test.c -o caller-save-test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all -fdump-rtl-caller_save caller-save-test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;
volatile int global_data[256];

/* Noinline helper functions to ensure real calls */
__attribute__((noinline, noclone))
int helper1(int a, int b) {
    global_counter++;
    return a + b + global_counter;
}

__attribute__((noinline, noclone))
int helper2(int a, int b, int c) {
    global_counter += 2;
    return a * b + c - global_counter;
}

__attribute__((noinline, noclone))
void helper3(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = helper1(arr[i], i);
    }
}

/* Test function that forces caller-save at basic block boundaries */
__attribute__((noinline, noclone))
int test_caller_save(int mode, int iterations) {
    /* Create register pressure with many local variables */
    int v1 = global_data[0];
    int v2 = global_data[1];
    int v3 = global_data[2];
    int v4 = global_data[3];
    int v5 = global_data[4];
    int v6 = global_data[5];
    int v7 = global_data[6];
    int v8 = global_data[7];
    int v9 = global_data[8];
    int v10 = global_data[9];
    
    /* Explicit register variable bound to a call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2;
    
    int result = 0;
    
    /* Force reg_var to be live across multiple calls at basic block ends */
    for (int i = 0; i < iterations; i++) {
        /* Use reg_var before call */
        int temp = reg_var * 2;
        
        if (mode == 0) {
            /* Call at end of if-block before return */
            result = helper1(temp, v3);
            /* Basic block ends here - reg_var must be saved/restored */
            return result + reg_var;  /* reg_var live across return */
        } else if (mode == 1) {
            /* Call at end of if-block with jump to else */
            result = helper2(temp, v4, v5);
            /* This creates a basic block boundary */
            if (result > 100) {
                global_sink = reg_var + result;  /* Use reg_var after call */
                reg_var = helper1(reg_var, i);   /* Another call */
                /* Basic block ends with call at the end */
                break;  /* Jump to loop exit */
            } else {
                v6 = result + reg_var;  /* Use in else block */
            }
        } else if (mode == 2) {
            /* Call as last statement in loop body */
            for (int j = 0; j < 3; j++) {
                v7 = helper1(v7, j);
                v8 = helper2(v8, j, reg_var);  /* reg_var used in call */
                /* Loop body ends with call - basic block boundary */
                if (j == 2) {
                    result = helper1(reg_var, v9);  /* Call at block end */
                    /* reg_var must be preserved for next iteration */
                }
                reg_var += v7;  /* Update reg_var after call */
            }
        } else {
            /* Complex pattern with multiple calls */
            int arr[10];
            for (int k = 0; k < 10; k++) {
                arr[k] = reg_var + k;
            }
            
            /* Call at end of basic block before function return */
            helper3(arr, 10);
            
            /* Use reg_var after call */
            for (int k = 0; k < 10; k++) {
                result += arr[k] * reg_var;
            }
            
            /* Another call at block end */
            result = helper1(result, reg_var);
            return result;  /* Basic block ends with return */
        }
        
        /* Update reg_var to keep it live */
        reg_var = helper1(reg_var, v10);
        v10 = helper2(v10, reg_var, result);
    }
    
    /* Final use of reg_var */
    global_sink = reg_var;
    return result;
}

/* Another test with different register pressure */
__attribute__((noinline, noclone))
int test_boundary_calls(int seed) {
    /* Even more variables to increase pressure */
    int a1 = seed + 1, a2 = seed + 2, a3 = seed + 3, a4 = seed + 4;
    int a5 = seed + 5, a6 = seed + 6, a7 = seed + 7, a8 = seed + 8;
    int a9 = seed + 9, a10 = seed + 10, a11 = seed + 11, a12 = seed + 12;
    int a13 = seed + 13, a14 = seed + 14, a15 = seed + 15, a16 = seed + 16;
    
    /* Use explicit register for a call-clobbered register */
    register int critical asm ("r13") = a1 * a2;
    
    /* Force spills around calls at block boundaries */
    if (seed % 2 == 0) {
        int r1 = helper1(critical, a3);
        int r2 = helper2(r1, a4, a5);
        /* Call at end of if block */
        critical = helper1(r2, critical);  /* critical used before and after */
        return critical + a6;  /* Block ends with return */
    } else {
        for (int i = 0; i < 5; i++) {
            a7 = helper1(a7, i);
            a8 = helper2(a8, i, critical);
            /* Call at end of loop body - basic block boundary */
            if (i == 4) {
                a9 = helper1(critical, a9);  /* Last call in block */
                /* critical must be saved/restored here */
            }
            critical += a7;
        }
        
        /* Switch with calls at case ends */
        switch (seed % 3) {
            case 0:
                a10 = helper1(critical, a10);
                break;  /* Basic block ends with break */
            case 1:
                a11 = helper2(critical, a11, a12);
                critical = helper1(a11, a13);
                break;
            default:
                a14 = helper1(a14, critical);
                return a14;  /* Block ends with return */
        }
    }
    
    /* Final computation forcing one more caller-save */
    return helper2(critical, a15, a16);
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < 256; i++) {
        global_data[i] = i * 3 + 7;
    }
    
    int total = 0;
    
    /* Exercise different paths to trigger various caller-save scenarios */
    for (int i = 0; i < 100; i++) {
        total += test_caller_save(i % 4, 3 + (i % 3));
        total += test_boundary_calls(i);
        
        /* Mix in some direct calls to increase register pressure globally */
        if (i % 7 == 0) {
            global_sink += helper1(total, i);
        }
        if (i % 5 == 0) {
            global_sink += helper2(total, i, global_sink);
        }
    }
    
    printf("Result: %d (global_sink: %d)\n", total, global_sink);
    
    /* Verify we actually did some work */
    if (total != 0 || global_sink != 0) {
        printf("Test executed successfully.\n");
    }
    
    return 0;
}
