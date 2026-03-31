/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra -mtune=generic -fomit-frame-pointer */
/* This program is designed to trigger specific instruction movement logic in GCC's caller-save pass */

#include <stdio.h>
#include <stdlib.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

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

__attribute__((noinline, noipa)) int helper4(int a, int b) {
    return (a << 2) | (b & 0xF);
}

/* Test function 1: High register pressure with consecutive calls */
void test1(int seed) {
    /* Declare many local variables to create register pressure */
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
        /* First call - clobbers call-used registers */
        v1 = helper1(v1, v2);
        
        /* Critical instruction that should be at the end of basic block */
        /* This instruction uses v3 which is live across the call */
        v3 = v4 + v5;  /* This should be the last instruction before BB_END */
        
        /* Second call - forces save/restore around it */
        v2 = helper2(v2, v3);
        
        /* Use results to prevent elimination */
        v4 = v1 + v2;
        v5 = v3 + v4;
        
        /* More computations to increase pressure */
        v6 = helper3(v6, v7);
        v7 = helper4(v7, v8);
        v8 = v9 + v10;
        v9 = v8 + i;
    }
    
    /* Use all variables to prevent dead code elimination */
    global_accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 2: Explicit register clobbering with asm */
void test2(int seed) {
    int a = seed * 2;
    int b = seed * 3;
    int c = seed * 4;
    int d = seed * 5;
    int e = seed * 6;
    int f = seed * 7;
    int g = seed * 8;
    int h = seed * 9;
    
    for (int j = 0; j < 4; j++) {
        /* Inline asm that clobbers specific registers */
        asm volatile (
            "movl %0, %%r10d\n\t"
            "movl %1, %%r11d\n\t"
            : 
            : "r"(a), "r"(b)
            : "r10", "r11"
        );
        
        /* Call that will need to save r10, r11 */
        a = helper1(a, b);
        
        /* Critical instruction - should be at BB_END before movement */
        c = d + e;  /* This instruction should be moved */
        
        /* Another call */
        b = helper2(b, c);
        
        /* More operations */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r"(d)
            : "r"(e)
        );
        
        f = helper3(f, g);
        g = helper4(g, h);
        h = a + b + j;
    }
    
    global_accumulator += a + b + c + d + e + f + g + h;
}

/* Test function 3: Mixed pointer and scalar operations */
void test3(int seed) {
    int data[8];
    for (int k = 0; k < 8; k++) {
        data[k] = seed + k;
    }
    
    int *ptr = data;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < 5; i++) {
        /* Load from pointer - creates complex live ranges */
        tmp1 = *ptr++;
        tmp2 = *ptr++;
        
        /* Call that clobbers registers */
        tmp1 = helper1(tmp1, tmp2);
        
        /* Critical instruction - pointer arithmetic at BB_END */
        ptr--;  /* This should be the last instruction before movement */
        
        /* Another call */
        tmp2 = helper2(tmp2, tmp1);
        
        /* More operations */
        tmp3 = helper3(tmp1, tmp2);
        tmp4 = helper4(tmp3, i);
        
        sum1 += tmp1;
        sum2 += tmp2;
        sum3 += tmp3;
        sum4 += tmp4;
        
        /* Reset pointer occasionally */
        if (i % 2 == 0) {
            ptr = data;
        }
    }
    
    global_accumulator += sum1 + sum2 + sum3 + sum4;
}

/* Test function 4: Nested loops with calls at different levels */
void test4(int seed) {
    int x1 = seed, x2 = seed + 1, x3 = seed + 2, x4 = seed + 3;
    int y1 = seed + 4, y2 = seed + 5, y3 = seed + 6, y4 = seed + 7;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 3; inner++) {
            /* Multiple calls in sequence */
            x1 = helper1(x1, x2);
            x2 = helper2(x2, x3);
            
            /* Critical instruction - at the end of inner loop block */
            x3 = x4 + inner;  /* Should be at BB_END before movement */
            
            /* More calls */
            x4 = helper3(x4, x1);
            
            /* Complex expression that uses many variables */
            y1 = helper4(y1, y2);
            y2 = x1 + x2 + x3 + x4;
            y3 = y1 + y2 + inner;
            y4 = y3 * outer;
        }
        
        /* Call between loop iterations */
        x1 = helper1(x1, y4);
    }
    
    global_accumulator += x1 + x2 + x3 + x4 + y1 + y2 + y3 + y4;
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times with different seeds */
    for (int iter = 0; iter < 10; iter++) {
        int current_seed = seed + iter * 17;
        
        test1(current_seed);
        test2(current_seed + 100);
        test3(current_seed + 200);
        test4(current_seed + 300);
        
        /* Modify seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
