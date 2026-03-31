/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -mtune=generic -fomit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    return a * b - 1;
}

__attribute__((noinline, noipa)) int helper3(int a, int b, int c) {
    return (a ^ b) | c;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a + b * 2;
}

__attribute__((noinline, noipa)) long helper5(long a, long b) {
    return a - b / 3;
}

/* Test 1: High register pressure with int variables */
void test1(int iterations) {
    /* Declare many local variables to create register pressure */
    register int v1 asm("r10") = global_seed + 1;
    register int v2 asm("r11") = global_seed + 2;
    int v3 = global_seed + 3;
    int v4 = global_seed + 4;
    int v5 = global_seed + 5;
    int v6 = global_seed + 6;
    int v7 = global_seed + 7;
    int v8 = global_seed + 8;
    int v9 = global_seed + 9;
    int v10 = global_seed + 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple calls that clobber registers */
        v1 = helper1(v1, v2);
        v2 = helper2(v2, v3);
        
        /* This instruction should be at the end of a basic block
         * and may need to be moved by caller-save */
        v3 = v4 + v5;  /* Potential last instruction before loop back */
        
        v4 = helper3(v4, v5, v6);
        v5 = helper1(v5, v6);
        
        /* Another instruction that could be at block end */
        v6 = v7 * v8;  /* Another candidate for movement */
        
        v7 = helper2(v7, v8);
        v8 = helper3(v8, v9, v10);
        
        /* Mix computations to keep variables live */
        v9 = v10 + i;
        v10 = v1 - i;
    }
    
    /* Use results to prevent elimination */
    global_accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test 2: Explicit register clobbering with asm and long variables */
void test2(int iterations) {
    long l1 = global_seed * 2;
    long l2 = global_seed * 3;
    long l3 = global_seed * 4;
    long l4 = global_seed * 5;
    long l5 = global_seed * 6;
    long l6 = global_seed * 7;
    long l7 = global_seed * 8;
    long l8 = global_seed * 9;
    
    /* Use inline asm to explicitly clobber call-used registers */
    for (int i = 0; i < iterations; i++) {
        /* Call that clobbers registers */
        l1 = helper4(l1, l2);
        
        /* Inline asm that uses and clobbers specific registers */
        asm volatile (
            "addq %%r11, %%r12\n\t"
            "subq %%r13, %%r14"
            : 
            : "r" (l3), "r" (l4), "r" (l5), "r" (l6)
            : "r11", "r12", "r13", "r14", "cc", "memory"
        );
        
        /* Instruction that could be at block end and need moving */
        l2 = l3 + l4;  /* Candidate for BB_END update */
        
        l3 = helper5(l3, l4);
        
        /* Another potential block-end instruction */
        l4 = l5 * l6;  /* Another movement candidate */
        
        l5 = helper4(l5, l6);
        
        /* Complex expression to increase register pressure */
        l6 = (l7 << 2) | (l8 >> 1);
        l7 = helper5(l7, l8);
        l8 = l1 ^ l2;
    }
    
    global_accumulator += (int)(l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8);
}

/* Test 3: Mixed pointer and scalar variables */
void test3(int iterations) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = global_seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int scalar1 = global_seed;
    int scalar2 = global_seed * 2;
    int scalar3 = global_seed * 3;
    int scalar4 = global_seed * 4;
    int scalar5 = global_seed * 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Dereference pointer before call */
        scalar1 = *ptr1 + scalar2;
        
        /* Call that may clobber pointer registers */
        scalar2 = helper1(scalar2, scalar3);
        
        /* Pointer update that could be at block end */
        ptr1++;  /* This could be the last instruction in BB */
        
        scalar3 = helper2(scalar3, scalar4);
        
        /* Another pointer operation */
        *ptr2 = scalar4 + scalar5;
        
        scalar4 = helper3(scalar4, scalar5, scalar1);
        
        /* Instruction that might need moving */
        ptr2--;  /* Another candidate for movement */
        
        scalar5 = helper1(scalar5, scalar1);
        
        /* Mix pointer and scalar operations */
        scalar1 = *(ptr1 - 1) + *(ptr2 + 1);
    }
    
    global_accumulator += scalar1 + scalar2 + scalar3 + scalar4 + scalar5 + *ptr1 + *ptr2;
}

/* Test 4: Nested loops with calls at different levels */
void test4(int outer_iter, int inner_iter) {
    int a1 = global_seed;
    int a2 = global_seed + 1;
    int a3 = global_seed + 2;
    int a4 = global_seed + 3;
    int a5 = global_seed + 4;
    
    for (int i = 0; i < outer_iter; i++) {
        a1 = helper1(a1, a2);
        
        for (int j = 0; j < inner_iter; j++) {
            a2 = helper2(a2, a3);
            
            /* Instruction that could be at the end of inner loop block */
            a3 = a4 + a5;  /* Potential BB_END candidate */
            
            a4 = helper3(a4, a5, a1);
            
            /* Another potential block-end instruction */
            a5 = a1 * a2;  /* Another movement candidate */
        }
        
        /* Instruction at outer loop level that could be at block end */
        a1 = a2 - a3;  /* Could trigger BB_END update */
    }
    
    global_accumulator += a1 + a2 + a3 + a4 + a5;
}

/* Test 5: Switch statement with calls in different cases */
void test5(int iterations) {
    int b1 = global_seed;
    int b2 = global_seed + 10;
    int b3 = global_seed + 20;
    int b4 = global_seed + 30;
    
    for (int i = 0; i < iterations; i++) {
        switch (i % 4) {
            case 0:
                b1 = helper1(b1, b2);
                /* Instruction that could be at case block end */
                b2 = b3 + b4;  /* Potential for movement */
                break;
            case 1:
                b2 = helper2(b2, b3);
                b3 = b4 * b1;  /* Another candidate */
                break;
            case 2:
                b3 = helper3(b3, b4, b1);
                b4 = b1 - b2;  /* Another candidate */
                break;
            case 3:
                b4 = helper1(b4, b1);
                b1 = b2 ^ b3;  /* Another candidate */
                break;
        }
    }
    
    global_accumulator += b1 + b2 + b3 + b4;
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int seed = global_seed;
    
    /* Call test functions with different parameters
     * to explore various caller-save scenarios */
    test1(seed % 10 + 5);
    test2(seed % 8 + 4);
    test3(seed % 12 + 3);
    test4(seed % 5 + 2, seed % 6 + 2);
    test5(seed % 15 + 5);
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
