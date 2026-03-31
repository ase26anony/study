/* caller-save-test.c
 * Designed to trigger specific instruction movement logic in GCC's caller-save pass
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fdump-rtl-all -S caller-save-test.c
 */

#include <stdio.h>
#include <stdint.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inlineable helper functions that clobber registers */
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
    return a + (b << 2);
}

__attribute__((noinline, noipa)) void helper5(int *ptr, int val) {
    *ptr = val;
    asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
}

/* Test 1: High register pressure with consecutive calls */
__attribute__((noinline)) int test1(int iterations) {
    /* Declare many local variables to pressure registers */
    int v1 = global_seed + 1;
    int v2 = global_seed + 2;
    int v3 = global_seed + 3;
    int v4 = global_seed + 4;
    int v5 = global_seed + 5;
    int v6 = global_seed + 6;
    int v7 = global_seed + 7;
    int v8 = global_seed + 8;
    int v9 = global_seed + 9;
    int v10 = global_seed + 10;
    
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Use all variables in computations */
        v1 = v2 + v3;
        v2 = v3 - v4;
        v3 = v4 * v5;
        v4 = v5 ^ v6;
        v5 = v6 | v7;
        v6 = v7 & v8;
        v7 = v8 << 2;
        v8 = v9 >> 1;
        v9 = v10 + i;
        v10 = v1 * 2;
        
        /* First call - clobbers call-used registers */
        int t1 = helper1(v1, v2);
        
        /* Critical instruction that should be at end of basic block */
        int critical = v3 + v4;  /* This should be the last instruction before BB_END */
        
        /* Second call - forces spill/restore around critical instruction */
        int t2 = helper2(t1, critical);
        
        /* Use result to prevent elimination */
        sum += t2 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Loop update - creates back edge */
        v1 += i;
    }
    
    return sum;
}

/* Test 2: Explicit register clobbering with asm */
__attribute__((noinline)) int test2(int iterations) {
    long l1 = global_seed * 2;
    long l2 = global_seed * 3;
    long l3 = global_seed * 4;
    long l4 = global_seed * 5;
    long l5 = global_seed * 6;
    
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex computation using longs */
        l1 = l2 + l3;
        l2 = l3 - l4;
        l3 = l4 * l5;
        l4 = l5 ^ l1;
        l5 = l1 | l2;
        
        /* Call that uses long arguments */
        long t1 = helper4(l1, l2);
        
        /* Critical store instruction at block end */
        long critical = l3 + l4;
        
        /* Inline asm that clobbers specific registers */
        asm volatile("add %1, %0\n\t"
                     : "+r" (critical)
                     : "r" (t1)
                     : "r11", "r12");
        
        /* Another call */
        int t2 = helper3((int)critical, (int)l5, i);
        
        result += t2 + (int)l1 + (int)l2;
        
        /* Force register pressure */
        l1 += i;
        l2 -= i;
    }
    
    return result;
}

/* Test 3: Pointer manipulation with calls */
__attribute__((noinline)) int test3(int iterations) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = global_seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int *ptr3 = &data[9];
    
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pointer arithmetic */
        int val1 = *ptr1;
        int val2 = *ptr2;
        int val3 = *ptr3;
        
        /* First call */
        int t1 = helper1(val1, val2);
        
        /* Critical pointer update at block end */
        *ptr1 = val3 + i;  /* This store should be at BB_END */
        
        /* Call that clobbers registers */
        helper5(ptr2, t1);
        
        /* More computations */
        sum += *ptr1 + *ptr2 + val3;
        
        /* Update pointers */
        ptr1 = (ptr1 == &data[9]) ? &data[0] : ptr1 + 1;
        ptr2 = (ptr2 == &data[9]) ? &data[0] : ptr2 + 1;
    }
    
    return sum;
}

/* Test 4: Nested loops with calls */
__attribute__((noinline)) int test4(int outer_iter, int inner_iter) {
    int a1 = global_seed + 1;
    int a2 = global_seed + 2;
    int a3 = global_seed + 3;
    int a4 = global_seed + 4;
    int a5 = global_seed + 5;
    int a6 = global_seed + 6;
    
    int total = 0;
    
    for (int i = 0; i < outer_iter; i++) {
        for (int j = 0; j < inner_iter; j++) {
            /* Register pressure */
            a1 = a2 + a3;
            a2 = a3 * a4;
            a3 = a4 ^ a5;
            a4 = a5 | a6;
            a5 = a6 << (j & 3);
            a6 = a1 >> 1;
            
            /* Call sequence */
            int t1 = helper1(a1, a2);
            int t2 = helper2(a3, a4);
            
            /* Critical computation at block end */
            int critical = a5 + a6;
            
            /* Another call */
            int t3 = helper3(t1, t2, critical);
            
            total += t3;
            
            /* Loop update */
            a1 += j;
        }
        
        /* Outer loop update */
        a2 += i;
    }
    
    return total;
}

int main() {
    int iterations = 5;
    
    /* Call all test functions to exercise different patterns */
    int r1 = test1(iterations);
    int r2 = test2(iterations);
    int r3 = test3(iterations);
    int r4 = test4(iterations, 3);
    
    global_accumulator = r1 + r2 + r3 + r4;
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", global_accumulator);
    
    return 0;
}
