/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -S caller-save-test.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    return a * b - 1;
}

__attribute__((noinline, noipa)) int helper3(int a, int b, int c) {
    return (a + b) * c;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a ^ b;
}

/* Test 1: Many integer variables causing register pressure */
void test1(int seed) {
    /* Declare many local variables to increase register pressure */
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
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Use variables in computations to keep them live */
        v1 = v2 + v3;
        v4 = v5 - v6;
        v7 = v8 * v9;
        
        /* First call - clobbers call-used registers */
        v2 = helper1(v1, v4);
        
        /* Critical instruction: This should be at the end of a basic block
         * and may need to be moved by caller-save */
        v3 = v7 + v10;  /* This instruction may become BB_END */
        
        /* Second call - more register clobbering */
        v5 = helper2(v2, v3);
        
        /* Use results to prevent elimination */
        global_acc += v1 + v2 + v3 + v4 + v5;
    }
}

/* Test 2: Use long variables and asm to suggest specific registers */
void test2(long seed) {
    long l1 = seed + 1000;
    long l2 = seed + 2000;
    long l3 = seed + 3000;
    long l4 = seed + 4000;
    long l5 = seed + 5000;
    long l6 = seed + 6000;
    
    /* Loop with multiple basic blocks */
    for (int i = 0; i < 4; i++) {
        /* Complex computation creating register pressure */
        l1 = l2 ^ l3;
        l4 = l5 | l6;
        
        /* Call that clobbers registers */
        l2 = helper4(l1, l4);
        
        /* ASM to suggest specific register usage */
        register long r11_val asm("r11") = l2;
        register long r12_val asm("r12") = l3;
        
        /* Critical instruction that may be at block end */
        l3 = r11_val + r12_val + i;  /* Potential BB_END candidate */
        
        /* Another call */
        l5 = helper4(l2, l3);
        
        /* Use asm to explicitly clobber call-used registers */
        asm volatile("" : : "r"(r11_val), "r"(r12_val) : "r11", "r12");
        
        global_acc += (int)(l1 + l2 + l3 + l4 + l5);
    }
}

/* Test 3: Mix pointers and scalars for complex live ranges */
void test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr = data;
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int tmp1, tmp2, tmp3;
    
    /* Create a basic block ending with pointer arithmetic */
    for (int i = 0; i < 5; i++) {
        /* Load through pointer */
        tmp1 = *ptr;
        ptr++;
        
        /* First call */
        tmp2 = helper1(tmp1, i);
        
        /* Critical: pointer update that may be at block end */
        tmp3 = tmp1 + tmp2;  /* Potential BB_END */
        
        /* Second call with three arguments */
        sum1 = helper3(tmp1, tmp2, tmp3);
        
        /* Use results */
        sum2 += sum1;
        global_acc += tmp3;
    }
    
    /* Another loop with different pattern */
    ptr = data;
    for (int i = 0; i < 3; i++) {
        int val = *ptr;
        
        /* Back-to-back calls for maximum pressure */
        int res1 = helper1(val, i);
        int res2 = helper2(res1, val);
        
        /* Instruction that uses both results - may need moving */
        int final = res1 + res2 + *ptr;  /* Another BB_END candidate */
        
        ptr += 2;
        global_acc += final;
    }
}

/* Test 4: Nested loops with calls at different levels */
void test4(int seed) {
    int a = seed, b = seed * 2, c = seed * 3;
    int d, e, f;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Multiple computations */
            d = a + b + i;
            e = b + c + j;
            
            /* Call in inner loop */
            f = helper1(d, e);
            
            /* Critical instruction in inner loop block */
            a = f + j;  /* May be BB_END of inner block */
            
            /* Another call */
            b = helper2(a, f);
            
            global_acc += d + e + f;
        }
        
        /* Outer loop has different pattern */
        c = helper3(a, b, i);
        global_acc += c;
    }
}

/* Main driver */
int main() {
    volatile int seed = 12345;  /* Prevent constant propagation */
    
    /* Call test functions multiple times */
    for (int iter = 0; iter < 10; iter++) {
        int current_seed = seed + iter * 100;
        
        test1(current_seed);
        test2((long)current_seed);
        test3(current_seed);
        test4(current_seed);
        
        /* Modify seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %d\n", global_acc);
    return 0;
}
