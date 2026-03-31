/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    /* Use inline asm to ensure register clobbering */
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a - b + 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b + 3;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + b * 2;
}

/* Test function 1: High register pressure with int variables */
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
        /* Use variables in computations */
        v1 = v2 + v3;
        v4 = v5 * v6;
        
        /* First call - clobbers call-used registers */
        int r1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last in basic block
           and may need to be moved by caller-save */
        v7 = v8 + v9;  /* This instruction might become BB_END */
        
        /* Second call - forces save/restore around it */
        int r2 = helper2(v3, v4);
        
        /* Use results to prevent elimination */
        v10 = r1 + r2 + v7;
        global_acc += v10;
        
        /* Modify variables for next iteration */
        v2++;
        v3--;
        v8 = v9 ^ v10;
    }
}

/* Test function 2: Explicit register clobbering with long variables */
__attribute__((noinline)) void test2(int seed) {
    long l1 = seed * 2L;
    long l2 = seed * 3L;
    long l3 = seed * 4L;
    long l4 = seed * 5L;
    long l5 = seed * 6L;
    long l6 = seed * 7L;
    long l7 = seed * 8L;
    long l8 = seed * 9L;
    
    /* Inline asm to explicitly clobber call-used registers */
    asm volatile ("" : : : "r11", "r12", "r13", "r14", "r15");
    
    for (int i = 0; i < 4; i++) {
        /* Multiple computations */
        l1 = l2 + l3;
        l4 = l5 - l6;
        
        /* Call that uses long arguments */
        long r1 = helper4(l1, l2);
        
        /* Potential BB_END instruction */
        l7 = l8 * 2;  /* This could be last in block */
        
        /* Another call */
        int r2 = helper3((int)l3, (int)l4);
        
        /* Use results */
        l8 = r1 + r2 + l7;
        global_acc += (int)l8;
        
        /* Update for next iteration */
        l2 += i;
        l3 -= i;
        l5 = l6 ^ l7;
    }
}

/* Test function 3: Mixed scalar and pointer operations */
__attribute__((noinline)) void test3(int seed) {
    int arr[4] = {seed, seed + 1, seed + 2, seed + 3};
    int *ptr = arr;
    int x1 = seed * 2;
    int x2 = seed * 3;
    int x3 = seed * 4;
    int x4 = seed * 5;
    int x5 = seed * 6;
    
    for (int i = 0; i < 5; i++) {
        /* Pointer arithmetic and dereference */
        int val = *ptr;
        ptr++;
        if (ptr >= &arr[4]) ptr = arr;
        
        /* Computation with pointer value */
        x1 = x2 + val;
        x3 = x4 * x5;
        
        /* Call that might need register saves */
        int r1 = helper1(x1, x2);
        
        /* Instruction that could be BB_END */
        x5 = x3 + x4;  /* Last instruction before next call */
        
        /* Another call */
        int r2 = helper2(x3, val);
        
        /* Use results */
        x4 = r1 + r2 + x5;
        global_acc += x4;
        
        /* Complex update to create live ranges */
        x2 = x3 ^ x4;
        x3 = x5 * i;
    }
}

/* Test function 4: Nested loops for more complex CFG */
__attribute__((noinline)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            /* Multiple computations */
            a = b + c;
            d = e * f;
            
            /* Call in inner loop */
            int r1 = helper1(a, b);
            
            /* Potential BB_END instruction in inner loop block */
            g = h + j;  /* Could be last instruction before loop backedge */
            
            /* Another call */
            int r2 = helper3(c, d);
            
            /* Use results */
            h = r1 + r2 + g;
            global_acc += h;
            
            /* Updates */
            b += j;
            c -= j;
            e = f ^ g;
        }
        
        /* Additional computation between outer loop iterations */
        f = helper2(g, h);
        a = b + f;
    }
}

int main() {
    volatile int seed = 12345;  /* Prevent constant propagation */
    
    /* Call test functions multiple times with varying inputs */
    for (int i = 0; i < 10; i++) {
        int s = seed + i * 100;
        test1(s);
        test2(s);
        test3(s);
        test4(s);
    }
    
    /* Print result to ensure all code is live */
    printf("Result: %d\n", global_acc);
    
    return 0;
}
