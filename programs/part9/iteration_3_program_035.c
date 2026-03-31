/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save optimization pass
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -S caller-save-test.c
 * Or for more pressure: gcc -O3 -fno-ipa-ra -mtune=generic -fomit-frame-pointer -S caller-save-test.c
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
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

__attribute__((noinline, noipa)) long helper_long1(long a, long b) {
    return a + b + 10;
}

__attribute__((noinline, noipa)) long helper_long2(long a, long b) {
    return a - b + 20;
}

/* Test 1: Many integer variables with consecutive calls */
__attribute__((noinline)) void test1(int seed) {
    /* Declare many local variables to create register pressure */
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
        /* Use variables in computations to keep them live */
        var1 = var2 + var3;
        var4 = var5 * var6;
        
        /* First call - clobbers call-used registers */
        var7 = helper1(var1, var4);
        
        /* Critical instruction: This should be the last in basic block
         * and might need to be moved by caller-save */
        var8 = var7 + var2 + i;  /* This instruction should be at block end */
        
        /* Second call - forces save/restore around first call */
        var9 = helper2(var8, var3);
        
        /* Use result to prevent elimination */
        var10 += var9;
        
        /* Rotate variables to create complex live ranges */
        int tmp = var1;
        var1 = var2;
        var2 = var3;
        var3 = tmp;
    }
    
    /* Use all variables to prevent dead code elimination */
    global_acc += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
}

/* Test 2: Long variables with explicit asm clobbering */
__attribute__((noinline)) void test2(int seed) {
    long lvar1 = seed * 2L;
    long lvar2 = seed * 3L;
    long lvar3 = seed * 4L;
    long lvar4 = seed * 5L;
    long lvar5 = seed * 6L;
    long lvar6 = seed * 7L;
    
    /* Use inline asm to suggest specific register usage */
    register long r11_val asm ("r11") = lvar1;
    register long r12_val asm ("r12") = lvar2;
    
    for (int i = 0; i < 4; i++) {
        /* Complex computation keeping values in registers */
        r11_val = r11_val * 3 + i;
        r12_val = r12_val / 2 + i;
        
        /* Call that clobbers registers */
        lvar3 = helper_long1(r11_val, r12_val);
        
        /* Instruction that should be at block end and might move */
        lvar4 = lvar3 + r11_val - i;
        
        /* Another call */
        lvar5 = helper_long2(lvar4, r12_val);
        
        /* Use asm to explicitly clobber call-used registers */
        asm volatile ("" : : "r" (r11_val), "r" (r12_val) : "r11", "r12");
        
        lvar6 += lvar5;
        
        /* Swap values to create register pressure */
        long tmp = r11_val;
        r11_val = r12_val;
        r12_val = tmp;
    }
    
    global_acc += (int)(lvar1 + lvar2 + lvar3 + lvar4 + lvar5 + lvar6 + r11_val + r12_val);
}

/* Test 3: Mixed scalar and pointer operations */
__attribute__((noinline)) void test3(int seed) {
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = seed + i;
    }
    
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[5];
    int scalar1 = seed;
    int scalar2 = seed * 2;
    int scalar3 = seed * 3;
    int scalar4 = seed * 4;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointer - creates complex addressing */
        scalar1 = *ptr1 + scalar2;
        
        /* Call that might clobber pointer registers */
        scalar3 = helper1(scalar1, scalar2);
        
        /* Critical store instruction at block end */
        *ptr2 = scalar3 + i;  /* This store should be last in basic block */
        
        /* Another call */
        scalar4 = helper2(*ptr2, scalar1);
        
        /* Update pointers to create live ranges across calls */
        ptr1++;
        if (ptr1 >= &arr[10]) ptr1 = &arr[0];
        
        scalar2 = scalar4 + i;
    }
    
    /* Use all values */
    for (int i = 0; i < 10; i++) {
        global_acc += arr[i];
    }
    global_acc += scalar1 + scalar2 + scalar3 + scalar4;
}

/* Test 4: Nested loops with multiple basic blocks */
__attribute__((noinline)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 3; inner++) {
            /* First basic block with computation */
            a = b + c;
            b = c + d;
            
            /* Conditional to create multiple blocks */
            if (a > b) {
                /* Call in one branch */
                e = helper1(a, b);
                
                /* Instruction at block end */
                f = e + inner;  /* Should be last in this block */
                
                /* Another call */
                g = helper2(f, a);
            } else {
                /* Different call in other branch */
                e = helper3(b, a);
                
                /* Different instruction at block end */
                f = e - inner;  /* Should be last in this block */
                
                /* Another call */
                g = helper1(f, b);
            }
            
            /* Merge point - use values */
            h += g + f;
            
            /* Update variables for next iteration */
            c = d + e;
            d = e + f;
        }
    }
    
    global_acc += a + b + c + d + e + f + g + h;
}

int main() {
    /* Volatile seed to prevent constant propagation */
    volatile int seed = 12345;
    
    /* Call test functions multiple times */
    for (int i = 0; i < 10; i++) {
        int current_seed = seed + i * 100;
        test1(current_seed);
        test2(current_seed);
        test3(current_seed);
        test4(current_seed);
    }
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", global_acc);
    
    return 0;
}
