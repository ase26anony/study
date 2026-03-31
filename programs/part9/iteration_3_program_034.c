/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -S caller-save-test.c
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
    return a - b + 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    return a * b + 3;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a + b * 2;
}

__attribute__((noinline, noipa)) long helper5(long a, long b) {
    return a - b / 2;
}

/* Test function 1: High register pressure with ints */
void test1(int iterations) {
    /* Declare many local variables to pressure registers */
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
        
        /* Critical instruction: This should be at the end of a basic block
         * and may need to be moved by caller-save */
        v3 = v1 + v2 + i;  /* This instruction might become BB_END */
        
        /* More register pressure */
        v4 = helper3(v3, v4);
        v5 = helper1(v4, v5);
        
        /* Another potential end-of-block instruction */
        v6 = v4 - v5;  /* Could be BB_END */
        
        v7 = helper2(v6, v7);
        v8 = helper3(v7, v8);
        
        /* Use results to prevent elimination */
        global_accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    }
}

/* Test function 2: Explicit register clobbering with asm */
void test2(int iterations) {
    long l1 = global_seed * 2;
    long l2 = global_seed * 3;
    long l3 = global_seed * 4;
    long l4 = global_seed * 5;
    long l5 = global_seed * 6;
    long l6 = global_seed * 7;
    
    for (int i = 0; i < iterations; i++) {
        /* Use inline asm to explicitly clobber call-used registers */
        asm volatile ("" : : : "r11", "r12", "r13", "r14", "r15");
        
        /* Calls that use long arguments */
        l1 = helper4(l1, l2);
        l2 = helper5(l2, l3);
        
        /* Instruction that might need moving */
        l3 = l1 + l2 + i;  /* Potential BB_END candidate */
        
        /* More asm clobbering */
        asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
        
        l4 = helper4(l3, l4);
        l5 = helper5(l4, l5);
        
        /* Another end-of-block candidate */
        l6 = l4 * l5;  /* Could be BB_END */
        
        global_accumulator += l1 + l2 + l3 + l4 + l5 + l6;
    }
}

/* Test function 3: Mixed pointers and scalars */
void test3(int iterations) {
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = global_seed + i;
    }
    
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[5];
    int val1 = global_seed;
    int val2 = global_seed * 2;
    int val3 = global_seed * 3;
    int val4 = global_seed * 4;
    
    for (int i = 0; i < iterations; i++) {
        /* Dereference and update before calls */
        val1 = *ptr1 + i;
        val2 = *ptr2 - i;
        
        /* Calls that might require saving pointer registers */
        val1 = helper1(val1, val2);
        val2 = helper2(val2, val3);
        
        /* Critical store instruction - could be BB_END */
        *ptr1 = val1 + val2;  /* Store that might need moving */
        
        /* Update pointers (clobbers address registers) */
        ptr1++;
        if (ptr1 >= &arr[10]) ptr1 = &arr[0];
        
        val3 = helper3(val3, val4);
        val4 = helper1(val4, val1);
        
        /* Another potential BB_END instruction */
        *ptr2 = val3 * val4;  /* Store at end of block */
        
        ptr2--;
        if (ptr2 < &arr[0]) ptr2 = &arr[9];
        
        global_accumulator += val1 + val2 + val3 + val4 + *ptr1 + *ptr2;
    }
}

/* Test function 4: Complex control flow within basic blocks */
void test4(int iterations) {
    int a = global_seed;
    int b = global_seed + 1;
    int c = global_seed + 2;
    int d = global_seed + 3;
    int e = global_seed + 4;
    int f = global_seed + 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Series of calls with computations in between */
        a = helper1(a, b);
        b = helper2(b, c);
        
        /* This addition should be at block end before spill insertion */
        c = a + b + i;  /* Target for BB_END update */
        
        /* Conditional to create basic block boundaries */
        if (c > 100) {
            d = helper3(d, e);
            /* Another end-of-block candidate */
            e = d * 2;  /* Could be BB_END */
        } else {
            d = helper1(e, f);
            /* And another */
            f = d / 2;  /* Could be BB_END */
        }
        
        /* More pressure */
        a = helper2(c, d);
        b = helper3(d, e);
        
        global_accumulator += a + b + c + d + e + f;
    }
}

int main() {
    int iterations = 5;  /* Small loop to keep code manageable */
    
    /* Call all test functions to increase coverage chances */
    test1(iterations);
    test2(iterations);
    test3(iterations);
    test4(iterations);
    
    /* Use the result */
    printf("Result: %d\n", global_accumulator);
    
    return 0;
}
