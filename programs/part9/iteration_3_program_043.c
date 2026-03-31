/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test */

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
__attribute__((noinline)) void test1(int iterations) {
    int var1 = global_seed + 1;
    int var2 = global_seed + 2;
    int var3 = global_seed + 3;
    int var4 = global_seed + 4;
    int var5 = global_seed + 5;
    int var6 = global_seed + 6;
    int var7 = global_seed + 7;
    int var8 = global_seed + 8;
    int var9 = global_seed + 9;
    int var10 = global_seed + 10;
    
    for (int i = 0; i < iterations; i++) {
        /* Create register pressure */
        var1 = helper1(var2, var3);
        var2 = helper2(var3, var4);
        var3 = helper3(var4, var5);
        
        /* Critical sequence: call, then operation that might be moved */
        var4 = helper1(var5, var6);
        /* This increment should be at the end of a basic block */
        var5 = var6 + var7;  /* Candidate for movement */
        
        var6 = helper2(var7, var8);
        var7 = var8 + var9;  /* Another candidate */
        
        var8 = helper3(var9, var10);
        var9 = var10 + var1;  /* Another candidate */
        
        /* Use results to prevent elimination */
        global_accumulator += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9;
    }
}

/* Test function 2: Use long variables and asm clobbers */
__attribute__((noinline)) void test2(int iterations) {
    long var1 = global_seed + 100;
    long var2 = global_seed + 200;
    long var3 = global_seed + 300;
    long var4 = global_seed + 400;
    long var5 = global_seed + 500;
    long var6 = global_seed + 600;
    register long var7 asm ("r11") = global_seed + 700;
    register long var8 asm ("r12") = global_seed + 800;
    
    for (int i = 0; i < iterations; i++) {
        /* Use asm to suggest specific registers */
        asm volatile ("" : "+r" (var7), "+r" (var8));
        
        var1 = helper4(var2, var3);
        var2 = helper5(var3, var4);
        
        /* Critical sequence */
        var3 = helper4(var4, var5);
        var4 = var5 + var6;  /* Candidate - should be last in BB */
        
        var5 = helper5(var6, var7);
        var6 = var7 + var8;  /* Candidate */
        
        /* Clobber call-used registers explicitly */
        asm volatile ("" : : : "r11", "r12", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
        
        global_accumulator += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8;
    }
}

/* Test function 3: Mix pointers and scalars */
__attribute__((noinline)) void test3(int iterations) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = global_seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int var1 = global_seed;
    int var2 = global_seed * 2;
    int var3 = global_seed * 3;
    int var4 = global_seed * 4;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex live ranges with pointers */
        var1 = *ptr1 + helper1(var2, var3);
        ptr1++;  /* Pointer update - creates complex liveness */
        
        var2 = helper2(var3, var4);
        var3 = *ptr2 + var4;  /* Candidate - load + arithmetic */
        
        var4 = helper3(var1, var2);
        ptr2--;  /* Another pointer update */
        
        /* Critical: call followed by store that might be moved */
        *ptr1 = helper1(var3, var4);
        var1 = var2 + var3;  /* Candidate - should be last in BB */
        
        global_accumulator += var1 + var2 + var3 + var4 + *ptr1 + *ptr2;
    }
}

/* Test function 4: Nested loops with calls */
__attribute__((noinline)) void test4(int outer_iter) {
    int a = global_seed;
    int b = global_seed + 1;
    int c = global_seed + 2;
    int d = global_seed + 3;
    int e = global_seed + 4;
    int f = global_seed + 5;
    
    for (int i = 0; i < outer_iter; i++) {
        for (int j = 0; j < 3; j++) {
            /* Multiple calls in inner loop */
            a = helper1(b, c);
            b = helper2(c, d);
            
            /* Critical: call then operation at block end */
            c = helper3(d, e);
            d = e + f;  /* Candidate for movement */
            
            e = helper1(f, a);
            f = a + b;  /* Another candidate */
        }
        
        /* Use results */
        global_accumulator += a + b + c + d + e + f;
    }
}

/* Test function 5: Conditional with calls in both branches */
__attribute__((noinline)) void test5(int iterations) {
    int x = global_seed;
    int y = global_seed * 2;
    int z = global_seed * 3;
    int w = global_seed * 4;
    
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            x = helper1(y, z);
            y = x + z;  /* Candidate in then-branch */
            z = helper2(w, x);
        } else {
            w = helper3(x, y);
            x = y + w;  /* Candidate in else-branch */
            y = helper1(z, w);
        }
        
        /* Common tail with operation after call */
        z = helper2(x, y);
        w = x + y;  /* Candidate - should be last in BB */
        
        global_accumulator += x + y + z + w;
    }
}

int main() {
    int iterations = 5;
    
    /* Call all test functions to increase coverage chances */
    for (int run = 0; run < 3; run++) {
        test1(iterations);
        test2(iterations);
        test3(iterations);
        test4(iterations);
        test5(iterations);
        
        /* Modify seed to create different register pressure patterns */
        global_seed += 1000;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
