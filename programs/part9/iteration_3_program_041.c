/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    return a - b + 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    return a * b + 3;
}

__attribute__((noinline, noipa)) int helper4(int a, int b) {
    return (a << 2) | (b & 0xFF);
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(int seed) {
    /* Declare many local variables to pressure registers */
    register int var1 asm("r10") = seed + 1;
    register int var2 asm("r11") = seed + 2;
    register int var3 asm("r12") = seed + 3;
    int var4 = seed + 4;
    int var5 = seed + 5;
    int var6 = seed + 6;
    int var7 = seed + 7;
    int var8 = seed + 8;
    int var9 = seed + 9;
    int var10 = seed + 10;
    
    /* Loop to create basic blocks with calls */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        var1 = helper1(var1, var2);
        
        /* Critical instruction: should be at end of basic block */
        var3 = var4 + var5;  /* This may need to be moved by caller-save */
        
        /* Second call - more register clobbering */
        var2 = helper2(var3, var4);
        
        /* Use results to prevent elimination */
        global_accumulator += var1 + var2 + var3;
        
        /* Modify variables to create live ranges across calls */
        var4 = var5 + i;
        var5 = var6 - i;
        var6 = var7 * (i + 1);
    }
}

/* Test function 2: Explicit register clobbering with inline asm */
__attribute__((noinline)) void test2(int seed) {
    int a = seed * 2;
    int b = seed * 3;
    int c = seed * 4;
    int d = seed * 5;
    int e = seed * 6;
    int f = seed * 7;
    int g = seed * 8;
    int h = seed * 9;
    
    for (int i = 0; i < 4; i++) {
        /* Call that clobbers specific registers */
        a = helper3(a, b);
        
        /* Inline asm that uses and clobbers registers */
        asm volatile (
            "addl %%ebx, %%eax\n\t"
            "movl %%eax, %%ecx"
            : "=a"(c), "=c"(d)
            : "a"(c), "b"(d), "r"(e)
            : "memory"
        );
        
        /* Instruction that may need moving */
        e = f + g;  /* Potential candidate for BB_END update */
        
        /* Another call */
        f = helper4(g, h);
        
        global_accumulator += a + b + c + d + e + f;
        
        /* Rotate values to create complex live ranges */
        int tmp = a;
        a = b; b = c; c = d; d = e; e = f; f = g; g = h; h = tmp;
    }
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) void test3(int seed) {
    int data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i;
    }
    
    int *ptr = data;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int idx1 = 0, idx2 = 1, idx3 = 2, idx4 = 3;
    
    for (int i = 0; i < 5; i++) {
        /* Load from pointer, then call */
        sum1 = helper1(ptr[idx1], ptr[idx2]);
        
        /* Pointer arithmetic that may be at block end */
        ptr = data + ((i + 1) % 8);
        
        /* Another call */
        sum2 = helper2(ptr[idx3], ptr[idx4]);
        
        /* More operations */
        sum3 = sum1 + sum2;
        sum4 = helper3(sum3, i);
        
        global_accumulator += sum4;
        
        /* Update indices */
        idx1 = (idx1 + 1) % 8;
        idx2 = (idx2 + 2) % 8;
        idx3 = (idx3 + 3) % 8;
        idx4 = (idx4 + 4) % 8;
    }
}

/* Test function 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(int seed) {
    int x1 = seed, x2 = seed + 1, x3 = seed + 2;
    int y1 = seed + 3, y2 = seed + 4, y3 = seed + 5;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            /* Call in inner loop */
            x1 = helper1(x1, x2);
            
            /* Instruction that could be last in basic block */
            x3 = y1 + y2;
            
            /* Another call */
            y1 = helper2(x3, y3);
            
            global_accumulator += x1 + x3 + y1;
            
            /* Branch creates basic block boundaries */
            if (j % 2 == 0) {
                x2 = helper3(x2, y2);
                y2 = x2 + 1;  /* Another potential BB_END candidate */
            }
        }
        
        /* Outer loop update */
        y3 = helper4(y3, i);
    }
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times with varying seeds */
    for (int iter = 0; iter < 10; iter++) {
        int current_seed = seed + iter * 17;
        
        test1(current_seed);
        test2(current_seed + 100);
        test3(current_seed + 200);
        test4(current_seed + 300);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
