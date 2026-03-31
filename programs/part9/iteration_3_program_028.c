/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test
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

__attribute__((noinline, noipa)) void helper5(int *ptr) {
    *ptr += 1;
}

/* Test 1: Many integer variables with consecutive calls */
__attribute__((noinline)) void test1(int seed) {
    int var1 = seed;
    int var2 = seed + 1;
    int var3 = seed + 2;
    int var4 = seed + 3;
    int var5 = seed + 4;
    int var6 = seed + 5;
    int var7 = seed + 6;
    int var8 = seed + 7;
    int var9 = seed + 8;
    int var10 = seed + 9;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Use all variables to create register pressure */
        var1 = var2 + var3;
        var4 = var5 - var6;
        var7 = var8 * var9;
        
        /* First call - clobbers call-used registers */
        var1 = helper1(var1, var2);
        
        /* Critical instruction: This should be the last in basic block
         * and may need to be moved by caller-save */
        var10 = var1 + var3;  /* This instruction should be at BB end */
        
        /* Second call - forces save/restore around calls */
        var2 = helper2(var4, var5);
        
        /* Use result to prevent elimination */
        global_accumulator += var10 + var2;
    }
}

/* Test 2: Long variables with asm clobbering */
__attribute__((noinline)) void test2(int seed) {
    register long r11_val asm ("r11") = seed;
    register long r12_val asm ("r12") = seed + 100;
    register long r13_val asm ("r13") = seed + 200;
    long var1 = seed;
    long var2 = seed * 2;
    long var3 = seed * 3;
    long var4 = seed * 4;
    
    for (int i = 0; i < 4; i++) {
        /* Use register variables */
        r11_val = r12_val + r13_val;
        var1 = var2 ^ var3;
        
        /* Inline asm that clobbers specific registers */
        asm volatile ("" : "+r" (r11_val), "+r" (r12_val) : : "r13", "memory");
        
        /* Call that uses long arguments */
        var4 = helper4(var1, var2);
        
        /* Critical instruction at BB end */
        r13_val = var4 + r11_val;  /* Should be last in BB */
        
        /* Another call */
        var3 = helper4(var4, r12_val);
        
        global_accumulator += (int)(r13_val + var3);
    }
}

/* Test 3: Mix of pointers and scalars */
__attribute__((noinline)) void test3(int seed) {
    int data[10];
    int *ptr = data;
    int var1 = seed;
    int var2 = seed + 1;
    int var3 = seed + 2;
    int var4 = seed + 3;
    int var5 = seed + 4;
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    for (int i = 0; i < 5; i++) {
        /* Complex live ranges with pointers */
        var1 = *ptr + var2;
        var3 = var4 * var5;
        
        /* Call with pointer argument */
        helper5(&var1);
        
        /* Critical instruction - pointer update at BB end */
        ptr = &data[var1 % 10];  /* Should be last in BB */
        
        /* Another call */
        var2 = helper3(var1, var3, var4);
        
        /* Use pointer to prevent elimination */
        global_accumulator += *ptr + var2;
        
        /* Modify variables to create new live ranges */
        var4 = var5 + 1;
        var5 = var1 - 1;
    }
}

/* Test 4: Nested loops with varying pressure */
__attribute__((noinline)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            /* Create register pressure */
            a = b + c;
            d = e * f;
            g = h ^ a;
            
            /* First call */
            b = helper1(a, d);
            
            /* Critical instruction at inner loop BB end */
            c = g + b;  /* Should be last in BB */
            
            /* Second call */
            e = helper2(d, g);
            
            /* Third call to increase pressure */
            f = helper3(b, e, c);
            
            global_accumulator += a + d + g + f;
            
            /* Update variables for next iteration */
            h = c + 1;
        }
        
        /* Additional computation between outer loop iterations */
        a = helper1(b, c);
        d = helper2(e, f);
    }
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 10; iteration++) {
        int base = seed + iteration * 100;
        
        test1(base);
        test2(base + 1);
        test3(base + 2);
        test4(base + 3);
        
        /* Mix up seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
