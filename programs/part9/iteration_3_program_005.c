/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

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

/* Test function 1: High register pressure with int variables */
__attribute__((noinline)) void test1(int seed) {
    /* Declare many local variables to create register pressure */
    int var1 = seed + 1;
    int var2 = seed + 2;
    int var3 = seed + 3;
    int var4 = seed + 4;
    int var5 = seed + 5;
    int var6 = seed + 6;
    int var7 = seed + 7;
    int var8 = seed + 8;
    int var9 = seed + 9;
    int var10 = seed + 10;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Use all variables in computations to keep them live */
        var1 = var2 + var3;
        var4 = var5 - var6;
        var7 = var8 * var9;
        
        /* First call - clobbers call-used registers */
        int tmp1 = helper1(var1, var2);
        
        /* Critical instruction: This should be at the end of a basic block */
        /* The caller-save pass may need to move this relative to spill code */
        var10 = var1 + var3 + i;  /* This instruction might be moved */
        
        /* Second call - forces save/restore around calls */
        int tmp2 = helper2(var4, var5);
        
        /* Use results to prevent elimination */
        global_acc += tmp1 + tmp2 + var10;
        
        /* Rotate variables to create complex live ranges */
        int temp = var1;
        var1 = var2;
        var2 = var3;
        var3 = var4;
        var4 = var5;
        var5 = var6;
        var6 = var7;
        var7 = var8;
        var8 = var9;
        var9 = var10;
        var10 = temp;
    }
}

/* Test function 2: Use long variables and explicit register clobbering */
__attribute__((noinline)) void test2(long seed) {
    /* Use 'register' hints to suggest specific registers */
    register long r1 asm("r11") = seed + 100;
    register long r2 asm("r12") = seed + 200;
    long r3 = seed + 300;
    long r4 = seed + 400;
    long r5 = seed + 500;
    long r6 = seed + 600;
    long r7 = seed + 700;
    long r8 = seed + 800;
    
    for (int i = 0; i < 4; i++) {
        /* Inline asm to explicitly clobber call-used registers */
        asm volatile ("" : "+r" (r1), "+r" (r2) : : "r11", "r12");
        
        /* Computation that should be at block end */
        r3 = r1 + r2 + i;
        
        /* Call that clobbers registers */
        long tmp1 = helper4(r1, r2);
        
        /* Another computation - potential candidate for movement */
        r4 = r3 * 2;
        
        /* Second call */
        long tmp2 = helper5(r3, r4);
        
        global_acc += tmp1 + tmp2 + r4;
        
        /* Complex data flow to prevent optimization */
        r1 = r2 ^ r3;
        r2 = r3 + r4;
        r3 = r4 - r1;
        r4 = r5 * r6;
        r5 = r6 / 2;
        r6 = r7 + 1;
        r7 = r8 - 1;
        r8 = r1;
    }
}

/* Test function 3: Mix pointers and scalars */
__attribute__((noinline)) void test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int scalar1 = seed * 2;
    int scalar2 = seed * 3;
    int scalar3 = seed * 4;
    int scalar4 = seed * 5;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointer - creates memory pressure */
        scalar1 = *ptr1 + *ptr2;
        
        /* Call that might require saving pointer registers */
        int tmp1 = helper3(scalar1, scalar2);
        
        /* Pointer arithmetic that should be at block end */
        ptr1++;  /* This instruction might be moved by caller-save */
        
        /* Another call */
        int tmp2 = helper1(scalar3, scalar4);
        
        global_acc += tmp1 + tmp2 + scalar1;
        
        /* Update scalars to keep them live */
        scalar2 = scalar3 + i;
        scalar3 = scalar4 * 2;
        scalar4 = scalar1 - i;
        
        /* Conditional to create basic block boundaries */
        if (ptr1 < &data[9]) {
            ptr2--;
        }
    }
}

/* Test function 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Multiple computations */
            a = b + c;
            d = e - f;
            
            /* Call in inner loop - high pressure */
            int tmp1 = helper2(a, d);
            
            /* Instruction at potential block end */
            g = h + i + j;  /* Candidate for movement */
            
            /* Another call */
            int tmp2 = helper3(b, e);
            
            global_acc += tmp1 + tmp2 + g;
            
            /* Update variables */
            b = c + j;
            c = d + i;
            e = f * 2;
            f = g - 1;
            h = a;
        }
        
        /* Additional computation between outer loop iterations */
        a = helper1(b, c);
    }
}

/* Main function that drives all tests */
int main() {
    volatile int seed = 12345;  /* Volatile to prevent constant propagation */
    
    /* Run tests multiple times to increase coverage chances */
    for (int iter = 0; iter < 10; iter++) {
        int current_seed = seed + iter * 100;
        
        test1(current_seed);
        test2(current_seed);
        test3(current_seed);
        test4(current_seed);
        
        /* Modify seed to create different execution paths */
        seed = global_acc % 1000;
    }
    
    printf("Result: %d\n", global_acc);
    return 0;
}
