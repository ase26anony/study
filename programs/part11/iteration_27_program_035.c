/* caller-save-test.c - Test program to trigger caller-save register spills at basic block boundaries */

#include <stdio.h>
#include <stdint.h>

/* Volatile globals to prevent optimization */
volatile int global_counter = 0;
volatile int global_sink = 0;
volatile int global_result = 0;

/* Helper functions that won't be inlined */
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
int helper3(int a) {
    global_counter += 3;
    return a * 2 + global_counter;
}

/* Test function with register pressure and calls at block boundaries */
__attribute__((noinline, noclone, optimize("O2")))
int test_caller_save(int mode) {
    /* Create register pressure with many local variables */
    int v1 = global_counter + 1;
    int v2 = global_counter * 2;
    int v3 = global_counter + 3;
    int v4 = global_counter * 4;
    int v5 = global_counter + 5;
    int v6 = global_counter * 6;
    int v7 = global_counter + 7;
    int v8 = global_counter * 8;
    
    /* Explicit register variable - force use of call-clobbered register */
    register int reg_var asm ("r12") = v1 + v2;
    
    /* More variables to increase pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8;
    
    /* Force reg_var to be live across calls by using it before and after */
    reg_var = reg_var * 2 + v3;
    
    if (mode == 0) {
        /* Call at end of basic block before return */
        t1 = helper1(v1, v2);
        t2 = helper2(v3, v4, reg_var);  /* reg_var used here */
        reg_var = t1 + t2;  /* reg_var modified after call */
        
        /* This call is at the end of the if block */
        t3 = helper3(reg_var);
        global_sink = t3;
        return t3;  /* Basic block ends with return */
    }
    else if (mode == 1) {
        /* Call at end of loop body */
        int sum = 0;
        for (int i = 0; i < 3; i++) {
            reg_var = reg_var + i + v5;
            /* Call at end of loop body - creates block boundary */
            t1 = helper1(reg_var, v6);
            sum += t1;
            /* Loop increment/jump is in separate basic block */
        }
        global_sink = sum;
        return sum;
    }
    else if (mode == 2) {
        /* Multiple calls in different branches */
        if (reg_var > 100) {
            t1 = helper2(v7, v8, reg_var);
            reg_var = t1 * 2;
            /* Call at end of if block */
            t2 = helper3(reg_var);
            global_sink = t2;
            return t2;  /* Return creates block boundary */
        } else {
            t1 = helper1(reg_var, v1);
            reg_var = t1 + v2;
            /* Another call at end of else block */
            t2 = helper2(reg_var, v3, v4);
            global_sink = t2;
            return t2;  /* Return creates block boundary */
        }
    }
    else {
        /* Nested calls with register variable live across them */
        t1 = helper1(reg_var, v5);
        reg_var = t1 + v6;
        
        if (reg_var > 50) {
            t2 = helper2(reg_var, v7, v8);
            /* Call at end of if block before else */
            t3 = helper3(t2);
            global_sink = t3;
        } else {
            t2 = helper1(reg_var, v1);
            /* Call at end of else block */
            t3 = helper2(t2, v2, v3);
            global_sink = t3;
        }
        
        /* Use reg_var again to keep it live */
        reg_var = reg_var + global_sink;
        return helper1(reg_var, v4);
    }
}

/* Another test with different register usage pattern */
__attribute__((noinline, noclone, optimize("O3")))
int test_caller_save2(int iterations) {
    /* Use multiple explicit register variables */
    register int r1 asm ("r10") = global_counter;
    register int r2 asm ("r11") = global_counter * 2;
    register int r3 asm ("r12") = global_counter * 3;
    register int r4 asm ("r13") = global_counter * 4;
    
    int result = 0;
    
    /* Loop with calls at block boundaries */
    for (int i = 0; i < iterations; i++) {
        /* All register variables live across calls */
        r1 = r1 + i;
        r2 = r2 * (i + 1);
        
        /* Call at end of loop body */
        int temp = helper1(r1, r2);
        
        r3 = r3 + temp;
        r4 = r4 - temp;
        
        /* Another call, still in same block */
        temp = helper2(r3, r4, i);
        
        result += temp;
        
        /* Loop back edge creates new basic block */
    }
    
    /* Final computation with calls */
    r1 = helper3(r1);
    r2 = helper1(r2, r3);
    
    /* Call before return */
    result += helper2(r1, r2, r4);
    
    global_sink = result;
    return result;
}

int main() {
    int total = 0;
    
    printf("Testing caller-save register spills at basic block boundaries...\n");
    
    /* Test different modes to exercise different paths */
    for (int i = 0; i < 5; i++) {
        total += test_caller_save(i % 4);
        total += test_caller_save2(i + 1);
        
        /* Mix in some volatile operations to prevent optimization */
        global_result ^= total;
        asm volatile("" : : "r"(total) : "memory");
    }
    
    printf("Result checksum: %d\n", total);
    printf("Global sink: %d\n", global_sink);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
