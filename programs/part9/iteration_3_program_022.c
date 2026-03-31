/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

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

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a + b * 2;
}

__attribute__((noinline, noipa)) long helper5(long a, long b) {
    return a - b / 2;
}

/* Test function 1: High integer register pressure with consecutive calls */
__attribute__((noinline)) void test1(int seed) {
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
    
    for (int i = 0; i < 3; i++) {
        /* Multiple computations to create register pressure */
        var1 = var2 + var3;
        var4 = var5 * var6;
        var7 = var8 - var9;
        
        /* First call - clobbers call-used registers */
        var2 = helper1(var1, var3);
        
        /* Critical instruction: This should be the last in its basic block
           and may need to be moved by caller-save */
        var10 = var4 + var7;  /* This instruction should be at block end */
        
        /* Second call - forces save/restore around first call */
        var3 = helper2(var2, var10);
        
        /* Use results to prevent elimination */
        global_accumulator += var1 + var2 + var3 + var10;
    }
}

/* Test function 2: Explicit register clobbering with inline asm */
__attribute__((noinline)) void test2(long seed) {
    register long r11_val asm ("r11") = seed;
    register long r12_val asm ("r12") = seed + 1;
    register long r13_val asm ("r13") = seed + 2;
    long var1 = seed + 3;
    long var2 = seed + 4;
    long var3 = seed + 5;
    long var4 = seed + 6;
    
    for (int i = 0; i < 4; i++) {
        /* Use register variables */
        r11_val = r12_val + r13_val;
        var1 = var2 * var3;
        
        /* Inline asm that clobbers specific registers */
        asm volatile ("" : "+r" (r11_val), "+r" (r12_val) : : "r13", "memory");
        
        /* Call that clobbers more registers */
        var2 = helper4(r11_val, var1);
        
        /* Critical instruction at block end */
        var4 = r12_val + var2;  /* Should be last instruction before next call */
        
        /* Another call forcing spill/restore */
        var3 = helper5(var2, var4);
        
        /* Force register variables to be live across calls */
        asm volatile ("" : : "r" (r11_val), "r" (r12_val));
        
        global_accumulator += var1 + var2 + var3 + var4;
    }
}

/* Test function 3: Mixed pointers and scalars */
__attribute__((noinline)) void test3(int seed) {
    int data[10];
    int *ptr = data;
    int var1 = seed;
    int var2 = seed + 1;
    int var3 = seed + 2;
    int var4 = seed + 3;
    int var5 = seed + 4;
    
    for (int i = 0; i < 5; i++) {
        /* Pointer arithmetic and dereference */
        *ptr = var1 + var2;
        ptr++;
        
        /* Computation */
        var3 = var4 * var5;
        
        /* Call that may clobber pointer register */
        var1 = helper3(var3, *data);
        
        /* Critical store instruction at block end */
        var5 = var2 + var1;  /* Last instruction before next call */
        
        /* Another call */
        var2 = helper1(var5, var3);
        
        /* More pointer manipulation */
        ptr--;
        *ptr = var1 + var2;
        
        global_accumulator += var1 + var2 + var3 + var5 + *data;
    }
}

/* Test function 4: Complex control flow within basic block */
__attribute__((noinline)) void test4(int seed) {
    int var1 = seed;
    int var2 = seed + 1;
    int var3 = seed + 2;
    int var4 = seed + 3;
    int var5 = seed + 4;
    int var6 = seed + 5;
    
    /* Create a basic block with multiple instructions ending with
       a candidate for movement */
    for (int i = 0; i < 2; i++) {
        /* Several instructions creating register pressure */
        var1 = var2 + var3;
        var4 = var5 - var6;
        
        /* Call that forces spills */
        var2 = helper2(var1, var4);
        
        /* Multiple instructions after call, with the last one
           being a candidate for movement by caller-save */
        var3 = var1 * 2;
        var5 = var4 + 7;
        var6 = var2 - var3;  /* This should be at block end */
        
        /* Another call - forces save/restore code that might
           need to move the previous instruction */
        var1 = helper3(var6, var5);
        
        global_accumulator += var1 + var2 + var3 + var4 + var5 + var6;
    }
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        test1(seed + i * 100);
        test2(seed + i * 200);
        test3(seed + i * 300);
        test4(seed + i * 400);
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
