/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
volatile int global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    /* Force register clobbering */
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    /* Clobber different registers */
    asm volatile("" : : : "rbx", "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2");
    return a - b + 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b, int c) {
    /* Clobber many registers */
    asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                 "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    return a * b + c;
}

/* Test function 1: Many local variables with consecutive calls */
__attribute__((noinline)) void test1(int seed) {
    /* Declare many local variables to create register pressure */
    register int v1 asm("r10") = seed + 1;
    register int v2 asm("r11") = seed + 2;
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
        /* First call - clobbers registers */
        int t1 = helper1(v1, v2);
        
        /* Critical instruction that should be at the end of basic block */
        /* This instruction uses variables that must be live across calls */
        v3 = v4 + v5;  /* This could be moved by caller-save */
        
        /* Second call - clobbers more registers */
        int t2 = helper2(v6, v7);
        
        /* More computations to use all variables */
        v8 = v9 + v10 + t1 + t2;
        
        /* Update loop variable in a way that creates a basic block end */
        /* This increment might be the last instruction before jump */
        i += 0;  /* Placeholder for actual increment in loop header */
        
        /* Use results to prevent elimination */
        global_acc += v3 + v8;
    }
}

/* Test function 2: Explicit register usage with asm statements */
__attribute__((noinline)) void test2(int seed) {
    long l1 = seed * 2;
    long l2 = seed * 3;
    long l3 = seed * 4;
    long l4 = seed * 5;
    long l5 = seed * 6;
    long l6 = seed * 7;
    long l7 = seed * 8;
    long l8 = seed * 9;
    
    /* Force specific register usage */
    register long r11_val asm("r11") = l1;
    register long r12_val asm("r12") = l2;
    
    for (int j = 0; j < 4; j++) {
        /* Call that clobbers r11, r12 */
        int t1 = helper1(l3, l4);
        
        /* Instruction that might need to be moved */
        /* Using the register-pinned variables */
        long temp = r11_val + r12_val;
        
        /* Another call */
        int t2 = helper2(l5, l6);
        
        /* More operations */
        l7 = l8 + temp + t1 + t2;
        
        /* Conditional to create basic block boundaries */
        if (j & 1) {
            l7 += 1;
        }
        
        /* This store could be at the end of a basic block */
        global_acc += l7;
        
        /* Update register-pinned variables */
        r11_val += 1;
        r12_val += 2;
    }
}

/* Test function 3: Mixed pointers and scalars */
__attribute__((noinline)) void test3(int seed) {
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = seed + i;
    }
    
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[5];
    int scalar1 = seed * 2;
    int scalar2 = seed * 3;
    int scalar3 = seed * 4;
    int scalar4 = seed * 5;
    int scalar5 = seed * 6;
    
    for (int k = 0; k < 2; k++) {
        /* First call - clobbers registers including pointer registers */
        int t1 = helper3(*ptr1, scalar1, scalar2);
        
        /* Critical instruction: pointer update that might be at block end */
        /* This could be moved by caller-save */
        int val = *ptr2 + scalar3;
        
        /* Second call */
        int t2 = helper1(scalar4, scalar5);
        
        /* Update pointer based on computation */
        ptr1 += (t1 + t2) & 1;
        
        /* Store result */
        *ptr1 = val;
        
        /* Update scalar that's live across calls */
        scalar1 = scalar2 + 1;
        
        /* Use result */
        global_acc += *ptr1;
    }
}

/* Test function 4: Complex live ranges with multiple basic blocks */
__attribute__((noinline)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    /* Create multiple basic blocks with conditional */
    for (int i = 0; i < 5; i++) {
        int result1 = helper1(a, b);
        
        /* This instruction should be at the end of its basic block */
        int temp = c + d;
        
        int result2 = helper2(e, f);
        
        /* Conditional creates new basic block */
        if (temp > result1) {
            g = helper3(h, result1, result2);
            /* Another potential block-end instruction */
            a = b + 1;
        } else {
            f = helper1(g, h);
            /* Another potential block-end instruction */
            b = a - 1;
        }
        
        /* Final computation in the block */
        int final = helper2(temp, g);
        
        global_acc += final;
        
        /* Update loop variables in a way that creates live ranges */
        c += 1;
        d += 2;
    }
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times with different seeds */
    for (int iter = 0; iter < 10; iter++) {
        int current_seed = seed + iter * 100;
        
        test1(current_seed);
        test2(current_seed + 1);
        test3(current_seed + 2);
        test4(current_seed + 3);
        
        /* Add some branching to vary control flow */
        if (iter & 1) {
            test1(current_seed + 100);
        }
    }
    
    printf("Global accumulator: %d\n", global_acc);
    return 0;
}
