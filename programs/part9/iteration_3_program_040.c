/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -S caller-save-test.c
 * Or for more pressure: gcc -O3 -fno-ipa-ra -mtune=generic -S caller-save-test.c
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
    return (a + b) * c;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a ^ b;
}

/* Test 1: High register pressure with int variables */
void test1(int seed) {
    /* Declare many local variables to pressure registers */
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
        /* Use variables in computations */
        var1 = var2 + var3;
        var4 = var5 * var6;
        
        /* First call - clobbers call-used registers */
        int tmp1 = helper1(var1, var2);
        
        /* Critical instruction: This should be the last in its basic block
         * and might need to be moved by caller-save */
        var7 = var8 + var9;  /* This instruction should be at block end */
        
        /* Second call - forces spill/restore around it */
        int tmp2 = helper2(var3, var4);
        
        /* Use results to prevent elimination */
        var10 = tmp1 + tmp2 + var7;
        
        /* Rotate values to create live ranges across calls */
        int rot = var1;
        var1 = var2;
        var2 = var3;
        var3 = rot;
    }
    
    global_accumulator += var1 + var10;
}

/* Test 2: Use long variables and asm clobbers */
void test2(int seed) {
    long var1 = seed * 2L;
    long var2 = seed * 3L;
    long var3 = seed * 4L;
    long var4 = seed * 5L;
    long var5 = seed * 6L;
    
    /* Use inline asm to suggest specific register usage */
    register long r11_var asm ("r11") = var1;
    register long r12_var asm ("r12") = var2;
    
    for (int i = 0; i < 4; i++) {
        /* Complex computation spanning calls */
        r11_var = r12_var + var3;
        
        /* Call that clobbers registers */
        long tmp1 = helper4(r11_var, var4);
        
        /* Instruction that should be at block end */
        var5 = r11_var + tmp1;  /* Candidate for movement */
        
        /* Another call */
        long tmp2 = helper4(var5, var3);
        
        /* Asm that clobbers specific registers */
        asm volatile ("" : : "r" (r11_var), "r" (r12_var) : "r11", "r12");
        
        r12_var = tmp1 + tmp2;
    }
    
    global_accumulator += (int)(r11_var + r12_var);
}

/* Test 3: Mix pointers and scalars */
void test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointer */
        sum1 = *ptr1 + *ptr2;
        
        /* Call that might clobber pointer registers */
        int tmp1 = helper3(sum1, seed, i);
        
        /* Pointer arithmetic - could be at block end */
        ptr1++;  /* This instruction should be at block end */
        
        /* Another call */
        int tmp2 = helper1(*ptr2, tmp1);
        
        /* More computations */
        sum2 = tmp1 + tmp2;
        ptr2--;
        
        /* Use results */
        sum3 += sum1 + sum2;
    }
    
    global_accumulator += sum3;
}

/* Test 4: Nested loops with calls */
void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2;
    int d = seed + 3, e = seed + 4, f = seed + 5;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Multiple computations */
            a = b + c;
            d = e * f;
            
            /* Call sequence */
            int tmp1 = helper2(a, d);
            
            /* Instruction at potential block end */
            b = c + tmp1;  /* Should be last in block */
            
            /* Another call */
            int tmp2 = helper1(b, d);
            
            /* Update variables */
            c = tmp1 + tmp2;
            e = a + d;
            
            /* Conditional to create basic block boundaries */
            if (j == 0) {
                f = helper3(a, b, c);
            }
        }
    }
    
    global_accumulator += a + b + c;
}

/* Main driver */
int main() {
    volatile int seed = global_seed;
    
    /* Run tests multiple times to increase coverage chance */
    for (int iter = 0; iter < 2; iter++) {
        test1(seed + iter);
        test2(seed + iter * 2);
        test3(seed + iter * 3);
        test4(seed + iter * 4);
        
        /* Modify seed to prevent constant propagation */
        seed += 1000;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
