/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -mtune=generic -fomit-frame-pointer caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    /* Use inline asm to ensure register clobbering */
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a - b + 2;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b + 3;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + b * 4;
}

/* Test function 1: High register pressure with int variables */
__attribute__((noinline)) int test1(int seed) {
    /* Declare many local variables to increase register pressure */
    register int v1 asm ("r10") = seed + 1;
    register int v2 asm ("r11") = seed + 2;
    int v3 = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    
    int result = 0;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Use all variables in computations */
        v1 = v2 + v3;
        v2 = v4 - v5;
        v3 = v6 * v7;
        v4 = v8 ^ v9;
        v5 = v10 + i;
        
        /* First call - clobbers call-used registers */
        int tmp1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last in its basic block */
        /* The caller-save pass may need to move this relative to spill code */
        v6 = v7 + v8;  /* This instruction might become BB_END */
        
        /* Second call - more register clobbering */
        int tmp2 = helper2(v3, v4);
        
        /* Use results to prevent elimination */
        result += tmp1 + tmp2 + v5 + v6;
        
        /* Update variables to create live ranges across calls */
        v7 = v9 + v10;
        v8 = v1 - v2;
        v9 = v3 ^ v4;
        v10 = v5 * i;
    }
    
    return result;
}

/* Test function 2: Explicit register clobbering with long variables */
__attribute__((noinline)) long test2(long seed) {
    /* Use explicit register variables for specific call-clobbered regs */
    register long r11_val asm ("r11") = seed * 2;
    register long r12_val asm ("r12") = seed * 3;
    long a = seed + 100;
    long b = seed + 200;
    long c = seed + 300;
    long d = seed + 400;
    long e = seed + 500;
    long f = seed + 600;
    long g = seed + 700;
    long h = seed + 800;
    
    long result = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Complex computation using all variables */
        a = b + c;
        b = d - e;
        c = f * g;
        d = h ^ a;
        
        /* Inline asm that explicitly clobbers registers */
        asm volatile ("# Clobber r11, r12" : : : "r11", "r12", "memory");
        
        /* Call that uses register variables */
        long tmp1 = helper4(r11_val, r12_val);
        
        /* Critical instruction - potential BB_END candidate */
        e = f + g;  /* This might be moved by caller-save */
        
        /* Another call */
        long tmp2 = helper4(a, b);
        
        /* Use asm to force specific register usage */
        asm volatile ("# Use values" : "+r"(r11_val), "+r"(r12_val));
        
        result += tmp1 + tmp2 + c + d + e;
        
        /* Update for next iteration */
        f = g + h;
        g = a - b;
        h = c ^ d;
        r11_val += i;
        r12_val -= i;
    }
    
    return result;
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) int test3(int seed) {
    int data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i * 10;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[4];
    
    int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    
    int result = 0;
    
    for (int i = 0; i < 3; i++) {
        /* Pointer dereference creating complex live ranges */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Computation */
        v1 = v2 + v3;
        v2 = v4 - val1;
        
        /* Call that might require saving pointer registers */
        int tmp1 = helper3(v1, v2);
        
        /* Critical store instruction - potential BB_END */
        *ptr1 = v3 + v4;  /* Store that might need moving */
        
        /* Another call */
        int tmp2 = helper1(val1, val2);
        
        /* Update pointers and values */
        ptr1++;
        ptr2--;
        v3 = v4 * tmp1;
        v4 = val2 + tmp2;
        
        result += v1 + v2 + v3 + v4;
    }
    
    return result;
}

/* Test function 4: Nested loops with multiple basic blocks */
__attribute__((noinline)) int test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    int result = 0;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Multiple instructions in basic block */
            a = b + c;
            b = d - e;
            
            /* Call that clobbers registers */
            int tmp1 = helper2(a, b);
            
            /* Critical instruction - last in inner loop body */
            c = d + e;  /* Potential BB_END before loop back-edge */
            
            /* Another call */
            int tmp2 = helper3(f, g);
            
            /* Conditional to create multiple basic blocks */
            if (tmp1 > tmp2) {
                d = e + f;
                /* Call in conditional block */
                int tmp3 = helper1(d, g);
                result += tmp3;
            } else {
                d = f - g;
            }
            
            e = f * g;
            f = g + h;
            g = h + i + j;
            h = a + b;
            
            result += tmp1 + tmp2 + c + d;
        }
    }
    
    return result;
}

int main() {
    volatile int seed = 12345;
    int total = 0;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        total += test1(seed + i * 100);
        total += test2(seed + i * 200);
        total += test3(seed + i * 300);
        total += test4(seed + i * 400);
        
        /* Update seed to prevent constant propagation */
        seed += 54321;
    }
    
    global_acc = total;
    printf("Result: %d\n", total);
    
    return 0;
}
