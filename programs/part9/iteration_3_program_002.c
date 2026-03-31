/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    volatile int result = a + b + 1;
    return result;
}

__attribute__((noinline, noipa)) int helper2(int a, int b, int c) {
    volatile int result = a * b - c;
    return result;
}

__attribute__((noinline, noipa)) int helper3(int a, int b, int c, int d) {
    volatile int result = (a + b) * (c - d);
    return result;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    volatile long result = a ^ b;
    return result;
}

/* Test 1: Many integer variables with consecutive calls */
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
        int tmp1 = var1 + var2;
        int tmp2 = var3 * var4;
        int tmp3 = var5 - var6;
        int tmp4 = var7 ^ var8;
        int tmp5 = var9 | var10;
        
        /* First call - clobbers call-used registers */
        int res1 = helper1(tmp1, tmp2);
        
        /* Critical instruction that should be at the end of a basic block */
        /* This is the instruction that might need to be moved */
        var1 = tmp3 + tmp4;  /* This could be the last instruction before BB_END */
        
        /* Second call - more register clobbering */
        int res2 = helper2(res1, tmp5, var1);
        
        /* Use results to prevent elimination */
        global_acc += res1 + res2;
        
        /* Update variables for next iteration */
        var2 += res1;
        var3 += res2;
        var4 = var1 ^ var2;
        
        /* Another potential BB_END candidate */
        var5 = var3 * var4;  /* Could be last instruction in block */
    }
}

/* Test 2: Explicit register clobbering with inline asm */
__attribute__((noinline)) void test2(long seed) {
    register long r11_val asm ("r11") = seed;
    register long r12_val asm ("r12") = seed + 1;
    register long r13_val asm ("r13") = seed + 2;
    register long r14_val asm ("r14") = seed + 3;
    
    long var1 = seed * 2;
    long var2 = seed * 3;
    long var3 = seed * 4;
    long var4 = seed * 5;
    
    for (int i = 0; i < 4; i++) {
        /* Explicit asm to use specific registers */
        asm volatile ("" : "+r" (r11_val), "+r" (r12_val));
        
        /* Call that clobbers registers */
        long res1 = helper4(r11_val, r12_val);
        
        /* Instruction that might need to be moved to after spill code */
        var1 = r13_val ^ r14_val;  /* Potential BB_END candidate */
        
        /* Another call */
        long res2 = helper4(res1, var1);
        
        /* More asm to ensure registers are live */
        asm volatile ("" : : "r" (r13_val), "r" (r14_val));
        
        global_acc += (int)(res1 + res2);
        
        /* Update register values */
        r11_val += var2;
        r12_val += var3;
        
        /* Another potential last instruction */
        var2 = var1 * res2;  /* Could be BB_END */
        
        /* Force spill/restore around this call */
        long res3 = helper4(var2, var3);
        global_acc += (int)res3;
    }
}

/* Test 3: Mixed pointers and scalars */
__attribute__((noinline)) void test3(int seed) {
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = seed + i;
    }
    
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[5];
    int idx1 = 0;
    int idx2 = 1;
    
    int var1 = seed;
    int var2 = seed * 2;
    int var3 = seed * 3;
    int var4 = seed * 4;
    int var5 = seed * 5;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointers */
        int val1 = ptr1[idx1];
        int val2 = ptr2[idx2];
        
        /* Use variables */
        int sum1 = var1 + var2 + var3;
        int sum2 = var4 + var5 + val1;
        
        /* First call */
        int res1 = helper3(val1, val2, sum1, sum2);
        
        /* Critical instruction - pointer update that might be at block end */
        ptr1 = &arr[(idx1 + 1) % 10];  /* Potential BB_END candidate */
        
        /* Second call with more arguments */
        int res2 = helper2(res1, val1, val2);
        
        /* Third call */
        int res3 = helper1(res2, sum1);
        
        global_acc += res1 + res2 + res3;
        
        /* Update indices - could be last in block */
        idx1 = (idx1 + res1) % 10;  /* Potential BB_END */
        idx2 = (idx2 + res2) % 10;
        
        /* More computations */
        var1 = res1 ^ res2;
        var2 = res2 * res3;
        
        /* Another call sequence */
        int res4 = helper1(var1, var2);
        var3 = res4 + idx1;  /* Could be BB_END */
    }
}

/* Test 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 3; inner++) {
            /* Create complex live ranges */
            int t1 = a + b;
            int t2 = c * d;
            int t3 = e ^ f;
            int t4 = g | h;
            
            /* Call that forces spills */
            int r1 = helper2(t1, t2, t3);
            
            /* Instruction that could be at block end */
            a = t4 + r1;  /* Potential BB_END candidate */
            
            /* Another call */
            int r2 = helper1(a, b);
            
            /* More computations */
            b = r1 ^ r2;
            c = a * b;
            
            /* Call with many arguments */
            int r3 = helper3(c, d, e, f);
            
            /* Instruction that could be last */
            d = r3 - a;  /* Potential BB_END */
            
            global_acc += r1 + r2 + r3;
            
            /* Update variables */
            e += inner;
            f += outer;
            
            /* Final instruction in inner loop block */
            g = e * f;  /* Likely BB_END before loop backedge */
        }
        
        /* Reset some variables */
        h = a + b + c + d;
    }
}

int main() {
    volatile int seed = 12345;
    
    for (int iteration = 0; iteration < 10; iteration++) {
        int current_seed = seed + iteration * 100;
        
        test1(current_seed);
        test2((long)current_seed);
        test3(current_seed + 50);
        test4(current_seed + 100);
        
        /* Modify seed to create different patterns */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Global accumulator: %d\n", global_acc);
    return 0;
}
