/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -S caller-save-test.c
 * Or for more pressure: gcc -O3 -fno-ipa-ra -mtune=generic -fomit-frame-pointer -S caller-save-test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    /* Simple operation that uses registers */
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b, int c) {
    /* Different signature to prevent merging */
    return a * b - c;
}

__attribute__((noinline, noipa)) int helper3(int64_t a, int64_t b) {
    /* Uses 64-bit values to pressure different registers */
    return (int)(a + b);
}

__attribute__((noinline, noipa)) void helper4(int *ptr, int val) {
    /* Takes pointer to create memory pressure */
    *ptr = val;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) int test1(int seed) {
    /* Declare many local variables to pressure registers */
    int var1 = seed + 1;
    int var2 = seed * 2;
    int var3 = seed / 3;
    int var4 = seed - 4;
    int var5 = seed + 5;
    int var6 = seed * 6;
    int var7 = seed / 7;
    int var8 = seed - 8;
    int var9 = seed + 9;
    int var10 = seed * 10;
    
    int result = 0;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Basic block starts here */
        var1 = helper1(var1, var2);
        var3 = helper2(var3, var4, var5);
        
        /* CRITICAL: This instruction should be at the end of a basic block
         * and may need to be moved by caller-save */
        var6 = var7 + var8;  /* Simple instruction that could be last in BB */
        
        var9 = helper1(var9, var10);
        var2 = helper2(var2, var3, var4);
        
        /* Another potential end-of-block instruction */
        var5 = var6 - var7;
        
        result += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
    }
    
    return result;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) int test2(int seed) {
    /* Use 64-bit variables for different register pressure */
    int64_t a = seed * 100LL;
    int64_t b = seed * 200LL;
    int64_t c = seed * 300LL;
    int64_t d = seed * 400LL;
    int64_t e = seed * 500LL;
    int64_t f = seed * 600LL;
    
    int result = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Call that uses 64-bit values */
        int tmp1 = helper3(a, b);
        
        /* Inline asm to explicitly clobber call-used registers */
        asm volatile ("" : : : "r11", "r12", "r13", "r14", "r15");
        
        /* Instruction that might end up at BB end */
        c = d + e;
        
        int tmp2 = helper3(f, a);
        
        /* Another instruction that could be last in BB */
        b = c - d;
        
        /* Use results to prevent elimination */
        result += tmp1 + tmp2 + (int)c + (int)b;
        
        /* Modify variables for next iteration */
        a += 1;
        b += 2;
        c += 3;
        d += 4;
        e += 5;
        f += 6;
    }
    
    return result;
}

/* Test function 3: Mix of pointers and scalars */
__attribute__((noinline)) int test3(int seed) {
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
    
    int result = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Call with pointer argument */
        helper4(ptr1, var1);
        
        /* Arithmetic that might be at BB end */
        var2 = var3 + var4;
        
        /* Another call */
        var1 = helper1(var1, var2);
        
        /* Pointer arithmetic that could be last in BB */
        ptr1++;
        
        /* More calls to increase pressure */
        var3 = helper2(var3, var4, *ptr2);
        
        /* Another potential end-of-block instruction */
        var4 = var1 - var2;
        
        ptr2--;
        
        result += var1 + var2 + var3 + var4 + *ptr1 + *ptr2;
    }
    
    return result;
}

/* Test function 4: Complex control flow within basic blocks */
__attribute__((noinline)) int test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    int result = 0;
    
    /* Multiple basic blocks with different patterns */
    for (int i = 0; i < 2; i++) {
        /* First basic block in loop */
        a = helper1(a, b);
        b = helper2(b, c, d);
        
        /* CRITICAL: This could be the last instruction before
         * a conditional jump or loop back-edge */
        c = d + e;
        
        /* Second basic block */
        if (a > 0) {
            d = helper1(d, e);
            e = helper2(e, f, g);
            
            /* Another potential BB-end instruction */
            f = g + h;
            
            result += a + b + c + d + e + f;
        } else {
            g = helper1(g, h);
            h = helper2(h, a, b);
            
            /* Another potential BB-end instruction */
            a = b + c;
            
            result += g + h + a;
        }
        
        /* Loop update that could be at BB end */
        h = h + 1;
    }
    
    return result;
}

int main() {
    volatile int seed = 12345;
    int total = 0;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        total += test1(seed + i);
        total += test2(seed + i * 2);
        total += test3(seed + i * 3);
        total += test4(seed + i * 4);
        
        /* Modify seed to prevent constant propagation */
        seed += 777;
    }
    
    global_acc = total;
    
    printf("Result: %d\n", total);
    return 0;
}
