/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -S caller-save-test.c
 */

#include <stdio.h>

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
    return (a ^ b) | c;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a + b * 2;
}

/* Test 1: High register pressure with int variables */
void test1(int iterations) {
    /* Declare many local variables to create register pressure */
    int var1 = global_seed;
    int var2 = var1 + 1;
    int var3 = var2 * 2;
    int var4 = var3 - var1;
    int var5 = var4 ^ var2;
    int var6 = var5 | var3;
    int var7 = var6 & var4;
    int var8 = var7 + var5;
    int var9 = var8 - var6;
    int var10 = var9 * var7;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple calls that clobber call-used registers */
        var1 = helper1(var2, var3);
        var2 = helper2(var3, var4);
        
        /* Critical instruction: This should be the last in basic block
         * and might need to be moved by caller-save */
        var3 = var1 + var2;  /* Potential last instruction before BB end */
        
        var4 = helper1(var5, var6);
        var5 = helper2(var6, var7);
        var6 = var4 + var5;  /* Another candidate */
        
        /* Use results to prevent elimination */
        global_accumulator += var3 + var6;
    }
}

/* Test 2: Mix of int and long with explicit asm clobbers */
void test2(int iterations) {
    long lvar1 = global_seed;
    long lvar2 = lvar1 * 3;
    int ivar1 = global_seed;
    int ivar2 = ivar1 + 100;
    int ivar3 = ivar2 * 2;
    int ivar4 = ivar3 - 50;
    
    for (int i = 0; i < iterations; i++) {
        /* Use inline asm to explicitly clobber call-used registers */
        asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
        
        ivar1 = helper3(ivar2, ivar3, ivar4);
        lvar1 = helper4(lvar2, ivar1);
        
        /* Critical instruction that might be last in BB */
        ivar2 = ivar1 + (int)lvar1;
        
        /* Another asm clobber */
        asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
        
        ivar3 = helper1(ivar4, ivar2);
        ivar4 = helper2(ivar3, ivar1);
        
        /* Another potential last instruction */
        lvar2 = lvar1 + ivar3;
        
        global_accumulator += ivar2 + (int)lvar2;
    }
}

/* Test 3: Pointer arithmetic and complex live ranges */
void test3(int iterations) {
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = global_seed + i;
    }
    
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[5];
    int val1 = *ptr1;
    int val2 = *ptr2;
    int val3 = val1 + val2;
    int val4 = val1 * val2;
    int val5 = val3 - val4;
    
    for (int i = 0; i < iterations; i++) {
        /* Pointer dereference that creates complex live range */
        val1 = *ptr1;
        val2 = *ptr2;
        
        /* Call that clobbers registers */
        val3 = helper1(val1, val2);
        
        /* Critical: pointer increment that might be last in BB */
        ptr1++;  /* This could be moved by caller-save */
        
        val4 = helper2(val3, val1);
        val5 = helper3(val4, val2, val3);
        
        /* Another pointer operation */
        ptr2--;
        
        /* Use results and update array */
        *ptr1 = val5;
        global_accumulator += val5 + *ptr2;
    }
}

/* Test 4: Nested loops with multiple basic blocks */
void test4(int outer_iter, int inner_iter) {
    int a = global_seed;
    int b = a + 1;
    int c = b * 2;
    int d = c - a;
    int e = d ^ b;
    int f = e | c;
    int g = f & d;
    int h = g + e;
    
    for (int i = 0; i < outer_iter; i++) {
        for (int j = 0; j < inner_iter; j++) {
            /* Multiple calls in inner loop */
            a = helper1(b, c);
            b = helper2(c, d);
            
            /* This increment might be last in its BB */
            c = a + b;  /* Candidate for movement */
            
            d = helper3(e, f, g);
            e = helper1(f, g);
            
            /* Another candidate */
            f = d + e;
            
            g = helper2(h, a);
            h = helper3(b, c, d);
            
            global_accumulator += c + f + h;
        }
        
        /* Basic block boundary here - the last instruction
         * before the jump back might need adjustment */
        a = b + c;
    }
}

/* Main driver that exercises all tests */
int main() {
    int seed = global_seed;
    
    /* Run tests multiple times with varying parameters */
    for (int i = 0; i < 3; i++) {
        test1(seed % 5 + 2);
        test2(seed % 4 + 2);
        test3(seed % 6 + 2);
        test4(seed % 3 + 1, seed % 4 + 1);
        
        /* Modify seed to create different execution paths */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %d\n", global_accumulator);
    return global_accumulator != 0 ? 0 : 1;
}
