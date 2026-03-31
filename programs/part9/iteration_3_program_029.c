/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra -mtune=generic -fomit-frame-pointer caller_save_test.c -o caller_save_test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
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
        /* Use all variables to create register pressure */
        var1 = var2 + var3;
        var4 = var5 - var6;
        var7 = var8 * var9;
        
        /* First call - clobbers call-used registers */
        int tmp1 = helper1(var1, var2);
        
        /* Critical instruction: This should be the last in basic block
           and may need to be moved by caller-save */
        var10 = var1 + var3;  /* This instruction might be moved */
        
        /* Second call immediately after - creates pressure */
        int tmp2 = helper2(var4, var5);
        
        /* Use results to prevent elimination */
        global_acc += tmp1 + tmp2 + var10;
        
        /* Rotate variables to maintain liveness */
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

/* Test function 2: Long variables with explicit asm clobbers */
__attribute__((noinline)) void test2(int seed) {
    long var1 = seed * 2L;
    long var2 = seed * 3L;
    long var3 = seed * 4L;
    long var4 = seed * 5L;
    long var5 = seed * 6L;
    long var6 = seed * 7L;
    long var7 = seed * 8L;
    long var8 = seed * 9L;
    
    /* Use asm to suggest specific register usage */
    register long r11_var asm("r11") = var1;
    register long r12_var asm("r12") = var2;
    
    for (int i = 0; i < 4; i++) {
        /* Complex computation using suggested registers */
        r11_var = r12_var + var3;
        var4 = r11_var - var5;
        
        /* Call that clobbers registers */
        long tmp1 = helper4(var4, var5);
        
        /* Critical instruction at potential block end */
        var6 = var4 + var5;  /* May be moved by caller-save */
        
        /* Another call */
        long tmp2 = helper5(var6, var7);
        
        /* Explicit asm that clobbers specific registers */
        asm volatile("" : "+r"(r11_var), "+r"(r12_var) : : "r11", "r12");
        
        global_acc += (int)(tmp1 + tmp2 + var6);
        
        /* Rotate values */
        long temp = var1;
        var1 = var2;
        var2 = var3;
        var3 = var4;
        var4 = var5;
        var5 = var6;
        var6 = var7;
        var7 = var8;
        var8 = temp;
    }
}

/* Test function 3: Mixed scalar and pointer operations */
__attribute__((noinline)) void test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int var1 = seed;
    int var2 = seed * 2;
    int var3 = seed * 3;
    int var4 = seed * 4;
    
    for (int i = 0; i < 3; i++) {
        /* Pointer dereference creating complex live ranges */
        var1 = *ptr1 + var2;
        var3 = *ptr2 - var4;
        
        /* Call that may require saving pointer registers */
        int tmp1 = helper3(var1, var3);
        
        /* Critical store instruction - may be last in block */
        *ptr1 = var1 + var3;  /* This store might be moved */
        
        /* Another call */
        int tmp2 = helper1(var2, var4);
        
        global_acc += tmp1 + tmp2 + *ptr1;
        
        /* Update pointers to maintain liveness */
        ptr1++;
        ptr2--;
        if (ptr1 >= &data[10]) ptr1 = &data[0];
        if (ptr2 < &data[0]) ptr2 = &data[9];
        
        var4 = var3;
        var3 = var2;
        var2 = var1;
        var1 = tmp1 + tmp2;
    }
}

/* Test function 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(int seed) {
    int var1 = seed;
    int var2 = seed + 1;
    int var3 = seed + 2;
    int var4 = seed + 3;
    int var5 = seed + 4;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Multiple computations */
            var1 = var2 + var3;
            var4 = var5 - var1;
            
            /* Call in inner loop */
            int tmp1 = helper2(var1, var4);
            
            /* Critical instruction before loop back-edge */
            var5 = var1 * var4;  /* May be moved if at block end */
            
            /* Another call */
            int tmp2 = helper3(var5, var3);
            
            global_acc += tmp1 + tmp2;
            
            var3 = var2;
            var2 = var1;
        }
        
        /* Additional computation between outer loop iterations */
        var1 = helper1(var4, var5);
        var5 = var4 + 1;  /* Another potential block-end instruction */
    }
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times with varying seeds */
    for (int i = 0; i < 10; i++) {
        int current_seed = seed + i * 100;
        test1(current_seed);
        test2(current_seed);
        test3(current_seed);
        test4(current_seed);
    }
    
    printf("Global accumulator: %d\n", global_acc);
    return 0;
}
