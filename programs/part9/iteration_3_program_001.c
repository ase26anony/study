/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
volatile int global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    /* Simple operation that uses registers */
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b, int c) {
    /* Different signature to prevent merging */
    return a * b - c;
}

__attribute__((noinline, noipa)) int helper3(int a, int b, int c, int d) {
    /* More arguments to increase register pressure */
    return (a + b) * (c - d);
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    /* Long type to use different registers */
    return a ^ b;
}

__attribute__((noinline, noipa)) void helper5(int *ptr, int val) {
    /* Pointer argument to create complex live ranges */
    *ptr = val;
}

/* Test function 1: Many integer variables with consecutive calls */
void test1(int seed) {
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
        int res1 = helper1(var1, var2);
        
        /* Critical instruction: This should be the last in basic block
           and might need to be moved by caller-save */
        var7 = var8 + var9;  /* This instruction is a candidate for movement */
        
        /* Second call immediately after - creates pressure */
        int res2 = helper2(var3, var4, var7);
        
        /* Use results to prevent elimination */
        var10 = res1 + res2;
        
        /* Update variables to create data dependencies */
        var2 = var10 + i;
        var3 = var1 - i;
    }
    
    /* Ensure all variables are used */
    global_accumulator += var1 + var2 + var3 + var4 + var5 + 
                         var6 + var7 + var8 + var9 + var10;
}

/* Test function 2: Explicit register clobbering with asm */
void test2(long seed) {
    /* Use long variables for different register usage */
    long l1 = seed * 2;
    long l2 = seed * 3;
    long l3 = seed * 4;
    long l4 = seed * 5;
    long l5 = seed * 6;
    long l6 = seed * 7;
    
    /* Loop with multiple basic blocks */
    for (int i = 0; i < 4; i++) {
        /* Inline asm to suggest specific register usage */
        register long r11_val asm ("r11") = l1 + l2;
        register long r12_val asm ("r12") = l3 + l4;
        
        /* Use the register values */
        l5 = r11_val * r12_val;
        
        /* Call that clobbers registers */
        long res = helper4(l5, l6);
        
        /* Critical instruction at block end */
        l6 = l1 ^ l2;  /* This should be last in basic block */
        
        /* Another call */
        long res2 = helper4(res, l6);
        
        /* Update variables */
        l1 = res2 + i;
        l2 = l3 - i;
        
        /* Inline asm that clobbers specific registers */
        asm volatile ("" : : "r" (r11_val), "r" (r12_val) : "r11", "r12");
    }
    
    global_accumulator += (int)(l1 + l2 + l3 + l4 + l5 + l6);
}

/* Test function 3: Mix of pointers and scalars */
void test3(int seed) {
    int data[10];
    int *ptr = data;
    int val1 = seed;
    int val2 = seed * 2;
    int val3 = seed * 3;
    int val4 = seed * 4;
    int val5 = seed * 5;
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    /* Loop with complex live ranges */
    for (int i = 0; i < 3; i++) {
        /* Pointer arithmetic and dereference */
        int temp = *ptr + val1;
        
        /* Call with pointer argument */
        helper5(ptr, temp);
        
        /* Critical instruction - should be at block end */
        ptr++;  /* This increment might need moving */
        
        /* Another call */
        int res = helper3(val2, val3, val4, val5);
        
        /* Update values creating cross-call dependencies */
        val1 = res + i;
        val2 = *ptr - i;
        
        /* Use all variables */
        val3 = val4 * val5;
        val4 = val1 ^ val2;
        val5 = temp + i;
    }
    
    /* Use all values */
    for (int i = 0; i < 10; i++) {
        global_accumulator += data[i];
    }
    global_accumulator += val1 + val2 + val3 + val4 + val5;
}

/* Additional test with volatile to prevent optimizations */
void test4(volatile int seed) {
    int v1 = seed;
    int v2 = seed + 1;
    int v3 = seed + 2;
    int v4 = seed + 3;
    int v5 = seed + 4;
    int v6 = seed + 5;
    
    /* Unrolled loop to create more basic blocks */
    for (int i = 0; i < 2; i++) {
        /* First basic block */
        v1 = v2 + v3;
        int r1 = helper1(v1, v2);
        
        /* Instruction that might be moved */
        v4 = v5 - v6;
        
        /* Conditional to create block boundaries */
        if (r1 > 0) {
            /* Second call in a different block */
            int r2 = helper2(v3, v4, v5);
            
            /* Another candidate for movement */
            v6 = v1 * v2;
            
            /* Third call */
            int r3 = helper3(v4, v5, v6, r2);
            
            v1 = r3;
        } else {
            v1 = helper1(v4, v5);
        }
        
        /* Update all variables */
        v2 = v1 + i;
        v3 = v2 * i;
    }
    
    global_accumulator += v1 + v2 + v3 + v4 + v5 + v6;
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test1(seed + i);
        test2(seed + i);
        test3(seed + i);
        test4(seed + i);
        
        /* Modify seed to create different execution paths */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
