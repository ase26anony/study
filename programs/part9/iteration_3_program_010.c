/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra -mtune=generic -fomit-frame-pointer */
/* Additional flags for coverage: --coverage -fprofile-arcs -ftest-coverage */

#include <stdio.h>
#include <stdint.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    return a * b - 1;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    return (a << 3) | (b & 0xFF);
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a ^ b;
}

__attribute__((noinline, noipa)) long helper5(long a, long b) {
    return a + (b >> 2);
}

/* Test function 1: High integer register pressure */
void test1(int iterations) {
    /* Declare many local variables to pressure registers */
    int v1 = global_seed + 1;
    int v2 = global_seed + 2;
    int v3 = global_seed + 3;
    int v4 = global_seed + 4;
    int v5 = global_seed + 5;
    int v6 = global_seed + 6;
    int v7 = global_seed + 7;
    int v8 = global_seed + 8;
    int v9 = global_seed + 9;
    int v10 = global_seed + 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Use all variables in computations to keep them live */
        v1 = helper1(v1, v2);
        v2 = helper2(v2, v3);
        
        /* Critical instruction that should be at the end of a basic block */
        v3 = v4 + v5;  /* This instruction may need to be moved */
        
        v4 = helper3(v4, v5);
        v5 = helper1(v5, v6);
        
        /* Another critical instruction */
        v6 = v7 * v8;  /* Potential end-of-block instruction */
        
        v7 = helper2(v7, v8);
        v8 = helper3(v8, v9);
        
        /* Instruction that uses result to prevent elimination */
        v9 = v10 + v1;
        
        /* Force spill by using many values */
        global_accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
}

/* Test function 2: Long variables with explicit register usage */
void test2(int iterations) {
    long l1 = global_seed * 2L;
    long l2 = global_seed * 3L;
    long l3 = global_seed * 4L;
    long l4 = global_seed * 5L;
    long l5 = global_seed * 6L;
    long l6 = global_seed * 7L;
    long l7 = global_seed * 8L;
    long l8 = global_seed * 9L;
    
    for (int i = 0; i < iterations; i++) {
        /* Use inline asm to suggest specific registers */
        register long r11 asm("r11") = l1;
        register long r12 asm("r12") = l2;
        
        /* Call that clobbers registers */
        l1 = helper4(l1, l2);
        
        /* Instruction that may need moving - positioned before another call */
        l2 = l3 ^ l4;  /* Potential end-of-block candidate */
        
        l3 = helper5(l3, l4);
        
        /* More register pressure */
        asm volatile("" : "+r" (r11), "+r" (r12));
        l4 = r11 + r12;
        
        l5 = helper4(l5, l6);
        
        /* Another potential movable instruction */
        l6 = l7 - l8;
        
        l7 = helper5(l7, l8);
        
        global_accumulator += (int)(l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8);
    }
}

/* Test function 3: Mixed pointers and scalars */
void test3(int iterations) {
    int data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    int *ptr1 = &data[0];
    int *ptr2 = &data[4];
    int scalar1 = global_seed;
    int scalar2 = global_seed * 2;
    int scalar3 = global_seed * 3;
    int scalar4 = global_seed * 4;
    int scalar5 = global_seed * 5;
    
    for (int i = 0; i < iterations; i++) {
        /* Dereference pointer, update, then call */
        int val1 = *ptr1;
        scalar1 = helper1(scalar1, val1);
        
        /* Instruction that might be last in block */
        *ptr1 = scalar2 + scalar3;  /* Store that could be moved */
        
        scalar2 = helper2(scalar2, scalar3);
        
        /* More pointer manipulation */
        ptr1++;
        if (ptr1 >= &data[8]) ptr1 = &data[0];
        
        int val2 = *ptr2;
        scalar3 = helper3(scalar3, val2);
        
        /* Another potential end-of-block instruction */
        *ptr2 = scalar4 * scalar5;
        
        scalar4 = helper1(scalar4, scalar5);
        
        ptr2--;
        if (ptr2 < &data[0]) ptr2 = &data[7];
        
        global_accumulator += scalar1 + scalar2 + scalar3 + scalar4 + scalar5 + *ptr1 + *ptr2;
    }
}

/* Test function 4: Nested loops with calls at different levels */
void test4(int iterations) {
    int a = global_seed;
    int b = global_seed + 1;
    int c = global_seed + 2;
    int d = global_seed + 3;
    int e = global_seed + 4;
    int f = global_seed + 5;
    
    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j < 2; j++) {
            a = helper1(a, b);
            b = helper2(b, c);
            
            /* Instruction in inner loop that could be block-end */
            c = d + e;  /* Candidate for movement */
            
            d = helper3(d, e);
            
            if (j == 0) {
                e = helper1(e, f);
                /* Another potential block-end instruction */
                f = a * b;
            } else {
                e = helper2(e, f);
                f = c / (d + 1);
            }
        }
        
        global_accumulator += a + b + c + d + e + f;
    }
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int seed = global_seed;
    
    /* Call test functions with dynamic iteration counts */
    test1(seed % 5 + 3);  /* 3-7 iterations */
    test2(seed % 4 + 2);  /* 2-5 iterations */
    test3(seed % 6 + 1);  /* 1-6 iterations */
    test4(seed % 3 + 2);  /* 2-4 iterations */
    
    /* Additional calls to increase pressure */
    for (int i = 0; i < 10; i++) {
        test1(1);
        test2(1);
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
