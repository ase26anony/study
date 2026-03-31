/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra -mtune=generic -fomit-frame-pointer caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return a - b + 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return a * b + 3;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12");
    return a + b * 2;
}

/* Test function 1: Many integer variables with consecutive calls */
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
        /* Create register pressure */
        var1 = helper1(var1, var2);
        var2 = helper2(var2, var3);
        
        /* This instruction should be at the end of a basic block */
        var3 = var4 + var5;  /* Candidate for movement */
        
        var4 = helper3(var4, var5);
        var5 = helper1(var5, var6);
        
        /* Another potential end-of-block instruction */
        var6 = var7 * var8;  /* Candidate for movement */
        
        var7 = helper2(var7, var8);
        var8 = helper3(var8, var9);
        
        /* Force use of results to prevent elimination */
        global_accumulator += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8;
    }
}

/* Test function 2: Long variables with explicit register clobbering */
__attribute__((noinline)) void test2(int seed) {
    register long r11_var asm("r11") = seed + 100;
    register long r12_var asm("r12") = seed + 200;
    long var1 = seed + 11;
    long var2 = seed + 12;
    long var3 = seed + 13;
    long var4 = seed + 14;
    long var5 = seed + 15;
    long var6 = seed + 16;
    
    for (int i = 0; i < 4; i++) {
        /* Use register variables to force specific register usage */
        asm volatile("" : "+r"(r11_var), "+r"(r12_var));
        
        var1 = helper4(var1, var2);
        var2 = helper4(var2, var3);
        
        /* Instruction at potential block end */
        var3 = var4 - var5;  /* Candidate for movement */
        
        var4 = helper4(var4, var5);
        var5 = helper4(var5, var6);
        
        /* Mix with register variables */
        r11_var = var1 + var2;
        r12_var = var3 + var4;
        
        global_accumulator += (int)(r11_var + r12_var + var5 + var6);
    }
}

/* Test function 3: Mixed scalar and pointer operations */
__attribute__((noinline)) void test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr = data;
    int var1 = seed + 21;
    int var2 = seed + 22;
    int var3 = seed + 23;
    int var4 = seed + 24;
    int var5 = seed + 25;
    
    for (int i = 0; i < 5; i++) {
        /* Pointer dereference creates complex live ranges */
        var1 = *ptr + helper1(var1, var2);
        ptr++;
        
        var2 = helper2(var2, var3);
        
        /* Potential end-of-block instruction */
        var3 = var4 * var5;  /* Candidate for movement */
        
        var4 = helper3(var4, var5);
        var5 = helper1(var5, var1);
        
        /* Another pointer operation */
        *ptr = var2 + var3;
        
        global_accumulator += var1 + var2 + var3 + var4 + var5 + *ptr;
    }
}

/* Test function 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(int seed) {
    int var1 = seed + 31;
    int var2 = seed + 32;
    int var3 = seed + 33;
    int var4 = seed + 34;
    int var5 = seed + 35;
    int var6 = seed + 36;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            var1 = helper1(var1, var2);
            
            if (j % 2 == 0) {
                var2 = helper2(var2, var3);
                /* This could be at the end of a basic block */
                var3 = var4 + var5;  /* Candidate for movement */
            } else {
                var4 = helper3(var4, var5);
                /* Another potential end-of-block instruction */
                var5 = var6 * var1;  /* Candidate for movement */
            }
            
            var6 = helper1(var6, var1);
            global_accumulator += var1 + var2 + var3 + var4 + var5 + var6;
        }
    }
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test1(seed + i * 10);
        test2(seed + i * 20);
        test3(seed + i * 30);
        test4(seed + i * 40);
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
