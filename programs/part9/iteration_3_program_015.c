/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test */

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

__attribute__((noinline, noipa)) int helper3(int a, int b, int c) {
    return (a ^ b) | c;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a + b * 2;
}

__attribute__((noinline, noipa)) void helper5(int *ptr) {
    *ptr += 1;
}

/* Test 1: High register pressure with int variables */
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
        /* First call - clobbers call-used registers */
        int tmp1 = helper1(var1, var2);
        
        /* Critical instruction that should be at end of basic block */
        var1 = var2 + var3;  /* This should be the last instruction before BB_END */
        
        /* Second call - forces save/restore around it */
        int tmp2 = helper2(var3, var4);
        
        /* Use results to prevent elimination */
        var5 = tmp1 + tmp2;
        var6 = helper3(var5, var7, var8);
        
        /* More register pressure */
        var7 = var8 + var9;
        var8 = var9 + var10;
        var9 = var10 + var1;
        var10 = var1 + var2;
    }
    
    global_accumulator += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
}

/* Test 2: Use long variables and explicit register clobbering */
__attribute__((noinline)) void test2(int seed) {
    long var1 = seed * 2L;
    long var2 = seed * 3L;
    long var3 = seed * 4L;
    long var4 = seed * 5L;
    long var5 = seed * 6L;
    long var6 = seed * 7L;
    
    /* Use inline asm to explicitly clobber call-used registers */
    register long r11 asm ("r11") = var1;
    register long r12 asm ("r12") = var2;
    
    for (int i = 0; i < 4; i++) {
        /* Call that clobbers registers */
        long tmp1 = helper4(var1, var2);
        
        /* Asm that uses specific registers */
        asm volatile ("" : "+r" (r11), "+r" (r12));
        
        /* Critical instruction at block end */
        var3 = var4 + var5;  /* Should be BB_END before movement */
        
        /* Another call */
        long tmp2 = helper4(var3, var4);
        
        /* More operations to create pressure */
        var5 = tmp1 + tmp2;
        var6 = var5 * 2;
        
        /* Update register variables */
        r11 = var5;
        r12 = var6;
    }
    
    global_accumulator += (int)(var1 + var2 + var3 + var4 + var5 + var6 + r11 + r12);
}

/* Test 3: Mix pointers and scalars */
__attribute__((noinline)) void test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr = &data[0];
    int var1 = seed;
    int var2 = seed * 2;
    int var3 = seed * 3;
    int var4 = seed * 4;
    int var5 = seed * 5;
    
    for (int i = 0; i < 5; i++) {
        /* Dereference pointer */
        int val = *ptr;
        
        /* Call that might need to save registers */
        int tmp1 = helper1(val, var1);
        
        /* Critical store instruction at block end */
        *ptr = var2 + var3;  /* Store should be BB_END */
        
        /* Call that clobbers registers */
        helper5(ptr);
        
        /* More operations */
        var4 = helper2(var3, var4);
        var5 = helper3(var4, var5, val);
        
        /* Update pointer */
        ptr = &data[(i + 1) % 10];
        
        /* Create cross-call live ranges */
        var1 = var2;
        var2 = var3;
        var3 = var4;
    }
    
    for (int i = 0; i < 10; i++) {
        global_accumulator += data[i];
    }
    global_accumulator += var1 + var2 + var3 + var4 + var5;
}

/* Test 4: Complex control flow within basic blocks */
__attribute__((noinline)) void test4(int seed) {
    int var1 = seed;
    int var2 = seed + 1;
    int var3 = seed + 2;
    int var4 = seed + 3;
    int var5 = seed + 4;
    int var6 = seed + 5;
    
    for (int i = 0; i < 3; i++) {
        /* Multiple calls in sequence */
        int t1 = helper1(var1, var2);
        int t2 = helper2(var3, var4);
        
        /* Instruction that should be at block end */
        var5 = var6 + t1;  /* BB_END candidate */
        
        /* Another pair of calls */
        int t3 = helper3(t1, t2, var5);
        int t4 = helper1(t3, var6);
        
        /* Complex dependency chain */
        var1 = t2 + t3;
        var2 = t3 + t4;
        var3 = t4 + var1;
        var4 = var1 + var2;
        var6 = var5 * 2;
    }
    
    global_accumulator += var1 + var2 + var3 + var4 + var5 + var6;
}

int main() {
    volatile int seed = global_seed;
    
    /* Run tests multiple times to increase coverage probability */
    for (int i = 0; i < 10; i++) {
        test1(seed + i);
        test2(seed + i * 2);
        test3(seed + i * 3);
        test4(seed + i * 4);
        
        /* Modify seed to prevent constant propagation */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
