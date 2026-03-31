/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

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

__attribute__((noinline, noipa)) void helper5(int *ptr) {
    *ptr += 1;
}

/* Test 1: High register pressure with int variables */
__attribute__((noinline, noipa)) void test1(int seed) {
    /* Declare many local variables to pressure registers */
    register int v1 asm("r10") = seed + 1;
    register int v2 asm("r11") = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Use all variables in computations */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 ^ v9;
        
        /* Call that clobbers registers - forces spills */
        int res1 = helper1(v1, v2);
        
        /* Critical instruction: This should be at the end of a basic block */
        /* The caller-save pass may need to move this relative to spill code */
        v10 = v3 + v4 + i;  /* This could become BB_END */
        
        /* Another call that clobbers registers */
        int res2 = helper2(v7, v10);
        
        /* Use results to prevent elimination */
        global_acc += res1 + res2 + v10;
        
        /* Modify variables for next iteration */
        v2++;
        v3 += res1;
        v4 = res2;
    }
}

/* Test 2: Explicit register clobbering with asm */
__attribute__((noinline, noipa)) void test2(long seed) {
    long l1 = seed * 2;
    long l2 = seed * 3;
    long l3 = seed * 4;
    long l4 = seed * 5;
    long l5 = seed * 6;
    long l6 = seed * 7;
    
    /* Use inline asm to explicitly clobber call-used registers */
    for (int i = 0; i < 4; i++) {
        /* Complex computation using all variables */
        l1 = l2 + l3;
        l4 = l5 - l6;
        
        /* Call helper that returns long */
        long res1 = helper4(l1, l2);
        
        /* Instruction that should be at block end */
        l6 = l3 + l4 + i;  /* Potential BB_END candidate */
        
        /* Inline asm that clobbers specific registers */
        asm volatile (
            "mov %1, %%r11\n\t"
            "add %2, %%r11\n\t"
            "mov %%r11, %0\n\t"
            : "=r" (l5)
            : "r" (l6), "r" (res1)
            : "r11"
        );
        
        /* Another computation that uses the result */
        l2 = l5 * l6;
        
        global_acc += (int)(l1 + l2 + l3 + l4);
    }
}

/* Test 3: Mix of pointers and scalars */
__attribute__((noinline, noipa)) void test3(int seed) {
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = seed + i;
    }
    
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[5];
    int val1 = seed;
    int val2 = seed * 2;
    int val3 = seed * 3;
    int val4 = seed * 4;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointers */
        val1 = *ptr1 + *ptr2;
        
        /* Call that takes pointer argument */
        helper5(ptr1);
        
        /* Critical store instruction - could be BB_END */
        *ptr2 = val1 + val2 + i;  /* Store at end of block */
        
        /* Another call */
        int res = helper3(val1, val2, val3);
        
        /* Update pointer - creates complex live range */
        ptr1 += (res & 1);
        ptr2 -= (res & 1);
        
        val3 = val4 * res;
        global_acc += val1 + val3 + *ptr1;
    }
}

/* Test 4: Nested loops with multiple basic blocks */
__attribute__((noinline, noipa)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    for (int i = 0; i < 2; i++) {
        /* First basic block in loop */
        a = b + c;
        b = c + d;
        
        /* Call that forces spills */
        int tmp1 = helper1(a, b);
        
        /* Conditional to create multiple blocks */
        if (tmp1 > 100) {
            c = d + e;
            /* Another call in conditional block */
            int tmp2 = helper2(c, f);
            
            /* Instruction at end of conditional block */
            f = g + h + i;  /* Could be BB_END */
            
            d = tmp1 + tmp2;
        } else {
            e = f + g;
            /* Different call sequence */
            int tmp3 = helper3(e, h, a);
            
            /* Another end-of-block candidate */
            g = h + tmp3 + i;  /* Could be BB_END */
            
            f = tmp1 * 2;
        }
        
        /* Loop back edge computation */
        h = a + b + c + d + e + f + g;
        global_acc += h;
    }
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times with varying seeds */
    for (int i = 0; i < 10; i++) {
        int current_seed = seed + i * 100;
        
        test1(current_seed);
        test2((long)current_seed);
        test3(current_seed + i);
        test4(current_seed * (i + 1));
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Result: %d\n", global_acc);
    return 0;
}
