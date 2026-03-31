/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to ensure computations aren't eliminated */
volatile int global_acc = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    /* Use inline asm to ensure register clobbering */
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b - 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b, int c) {
    asm volatile ("" : : "r"(a), "r"(b), "r"(c) : "memory");
    return (a ^ b) | c;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + (b << 2);
}

/* Test function 1: High register pressure with ints */
__attribute__((noinline)) int test1(int seed) {
    /* Declare many local variables to pressure registers */
    register int v1 asm("r10") = seed;
    register int v2 asm("r11") = seed + 1;
    int v3 = seed + 2;
    int v4 = seed + 3;
    int v5 = seed + 4;
    int v6 = seed + 5;
    int v7 = seed + 6;
    int v8 = seed + 7;
    int v9 = seed + 8;
    int v10 = seed + 9;
    
    int result = 0;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        int t1 = helper1(v1, v2);
        
        /* Critical instruction that should be at block end */
        /* This increment uses v3 which must be preserved across calls */
        v3 = v4 + v5;  /* This could be moved by caller-save */
        
        /* Second call - more register clobbering */
        int t2 = helper2(v3, t1);
        
        /* Use all variables to keep them live */
        v4 = v6 ^ v7;
        v5 = v8 | v9;
        v6 = helper3(v10, t2, v1);
        
        /* Accumulate results */
        result += t1 + t2 + v3 + v4 + v5 + v6;
        
        /* Modify variables for next iteration */
        v1++;
        v2--;
        v7 = v7 * 2;
        v8 = v8 / 2;
        v9 = v9 ^ v10;
        v10 = v10 + i;
    }
    
    return result;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) long test2(long seed) {
    /* Use explicit register variables */
    register long r1 asm("rax") = seed;
    register long r2 asm("rbx") = seed * 2;
    register long r3 asm("r12") = seed * 3;
    register long r4 asm("r13") = seed * 4;
    long r5 = seed * 5;
    long r6 = seed * 6;
    long r7 = seed * 7;
    long r8 = seed * 8;
    
    long result = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Inline asm that clobbers specific registers */
        asm volatile (
            "movq %1, %%r10\n\t"
            "addq %2, %%r10\n\t"
            "movq %%r10, %0"
            : "=r" (r5)
            : "r" (r1), "r" (r2)
            : "r10", "memory"
        );
        
        /* Call that clobbers more registers */
        long t1 = helper4(r3, r4);
        
        /* Critical instruction at potential block end */
        r6 = r7 + r8;  /* This could be moved */
        
        /* Another asm clobber */
        asm volatile (
            "imulq %1, %0"
            : "+r" (r6)
            : "r" (t1)
            : "cc", "memory"
        );
        
        /* Use results */
        result += r1 + r2 + r3 + r4 + r5 + r6 + t1;
        
        /* Modify register variables */
        r1 = r1 ^ result;
        r2 = r2 + i;
        r3 = r3 * 3;
        r4 = r4 - i;
        r7 = r7 << 1;
        r8 = r8 >> 1;
    }
    
    return result;
}

/* Test function 3: Mixed pointers and scalars */
__attribute__((noinline)) int test3(int seed) {
    int data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[4];
    
    int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    
    int result = 0;
    
    /* Create a basic block with calls and pointer updates */
    for (int i = 0; i < 5; i++) {
        /* First call using pointer values */
        int t1 = helper1(*ptr1, *ptr2);
        
        /* Pointer arithmetic that could be at block end */
        ptr1++;  /* This increment might be moved */
        
        /* Second call */
        int t2 = helper2(v1, v2);
        
        /* More operations */
        v3 = helper3(v3, t1, t2);
        
        /* Update pointer based on computation */
        ptr2 = &data[v3 & 7];
        
        /* Use all values */
        result += t1 + t2 + v1 + v2 + v3 + v4 + *ptr1 + *ptr2;
        
        /* Modify variables */
        v1 = v1 + v4;
        v2 = v2 ^ v3;
        v4 = v4 * 2;
    }
    
    return result;
}

/* Main driver */
int main() {
    volatile int seed = 12345;
    int total = 0;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        int s = seed + i * 100;
        
        total += test1(s);
        total += test2(s);
        total += test3(s);
        
        /* Modify seed to prevent constant propagation */
        seed = seed ^ total;
    }
    
    global_acc = total;
    printf("Result: %d\n", total);
    
    return 0;
}
