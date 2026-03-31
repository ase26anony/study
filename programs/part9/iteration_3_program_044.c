/* Test program for GCC caller-save optimization pass coverage */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c */

#include <stdio.h>

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
    return (a << 2) | (b & 0xFF);
}

/* Test 1: High register pressure with consecutive calls */
void test1(int iterations) {
    /* Declare many local variables to pressure registers */
    register int v1 asm("r10") = global_seed + 1;
    register int v2 asm("r11") = global_seed + 2;
    int v3 = global_seed + 3;
    int v4 = global_seed + 4;
    int v5 = global_seed + 5;
    int v6 = global_seed + 6;
    int v7 = global_seed + 7;
    int v8 = global_seed + 8;
    int v9 = global_seed + 9;
    int v10 = global_seed + 10;
    
    for (int i = 0; i < iterations; i++) {
        /* First call - clobbers call-used registers */
        v1 = helper1(v1, v2);
        
        /* Instruction that might need to be moved */
        /* This could be the last instruction in basic block */
        v3 = v4 + v5 + i;
        
        /* Second call - forces save/restore around calls */
        v2 = helper2(v3, v1);
        
        /* More computations to create live ranges across calls */
        v4 = v6 * v7 - v8;
        v5 = v9 ^ v10;
        
        /* Third call */
        v6 = helper3(v4, v5);
        
        /* Another potential instruction to move */
        v7 = v8 + v9;
        
        /* Fourth call */
        v8 = helper4(v6, v7);
        
        /* Use results to prevent elimination */
        global_accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    }
}

/* Test 2: Explicit register clobbering with inline asm */
void test2(int iterations) {
    int a = global_seed * 2;
    int b = global_seed * 3;
    int c = global_seed * 4;
    int d = global_seed * 5;
    int e = global_seed * 6;
    int f = global_seed * 7;
    int g = global_seed * 8;
    int h = global_seed * 9;
    
    for (int i = 0; i < iterations; i++) {
        /* Inline asm that clobbers specific registers */
        asm volatile (
            "movl %0, %%r10d\n\t"
            "movl %1, %%r11d\n\t"
            :
            : "r" (a), "r" (b)
            : "r10", "r11"
        );
        
        /* Call that forces register saves */
        a = helper1(b, c);
        
        /* Instruction that could be at block end */
        b = c + d + i;
        
        /* Another asm clobber */
        asm volatile (
            "addl %1, %0\n\t"
            : "+r" (c)
            : "r" (d)
            : "cc"
        );
        
        /* Another call */
        d = helper2(a, b);
        
        /* Potential block-end instruction */
        e = f * g;
        
        /* Use results */
        global_accumulator += a + b + c + d + e + f + g + h;
        
        /* Update variables for next iteration */
        f = g + h;
        g = h + i;
        h = a + b;
    }
}

/* Test 3: Mixed pointer and scalar operations */
void test3(int iterations) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = global_seed + i;
    }
    
    int *ptr = data;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int tmp1, tmp2, tmp3, tmp4;
    
    for (int i = 0; i < iterations; i++) {
        /* Dereference and computation before call */
        tmp1 = *ptr + *(ptr + 1);
        
        /* Call that clobbers registers */
        sum1 = helper1(tmp1, i);
        
        /* Instruction that might be last in block */
        tmp2 = *(ptr + 2) + *(ptr + 3);
        
        /* Another call */
        sum2 = helper2(tmp2, sum1);
        
        /* More pointer arithmetic */
        ptr += (i & 1);
        
        /* Instruction that could be moved */
        tmp3 = *ptr * 2;
        
        /* Call */
        sum3 = helper3(tmp3, sum2);
        
        /* Final computation in block */
        tmp4 = sum3 + *ptr;
        
        /* One more call */
        sum4 = helper4(tmp4, sum3);
        
        /* Use results */
        global_accumulator += sum1 + sum2 + sum3 + sum4 + tmp1 + tmp2 + tmp3 + tmp4;
    }
}

/* Test 4: Nested loops with calls at different levels */
void test4(int outer_iter, int inner_iter) {
    int x1 = global_seed, x2 = global_seed + 1;
    int x3 = global_seed + 2, x4 = global_seed + 3;
    int x5 = global_seed + 4, x6 = global_seed + 5;
    
    for (int i = 0; i < outer_iter; i++) {
        /* Outer loop computations */
        x1 = helper1(x1, i);
        x2 = helper2(x2, x1);
        
        for (int j = 0; j < inner_iter; j++) {
            /* Inner loop - creates basic blocks with calls */
            x3 = helper3(x3, j);
            
            /* Instruction that could be at block end */
            x4 = x5 + x6 + j;
            
            /* Another call */
            x5 = helper4(x4, x3);
            
            /* Computation between calls */
            x6 = x1 + x2 + x3;
            
            /* Use results */
            global_accumulator += x3 + x4 + x5 + x6;
        }
        
        /* Update outer variables */
        x1 = x2 + x3;
        x2 = x4 + x5;
    }
}

/* Test 5: Conditional calls and register pressure */
void test5(int iterations) {
    int a = global_seed, b = global_seed * 2;
    int c = global_seed * 3, d = global_seed * 4;
    int e = global_seed * 5, f = global_seed * 6;
    int g = global_seed * 7, h = global_seed * 8;
    
    for (int i = 0; i < iterations; i++) {
        /* Conditional that creates multiple basic blocks */
        if (i & 1) {
            a = helper1(b, c);
            /* Instruction that could be last in this block */
            b = c + d;
        } else {
            c = helper2(d, e);
            /* Another potential block-end instruction */
            d = e + f;
        }
        
        /* Common code with calls */
        e = helper3(f, g);
        
        /* Instruction that might need moving */
        f = g + h + i;
        
        /* Another call */
        g = helper4(h, a);
        
        /* Use all variables */
        global_accumulator += a + b + c + d + e + f + g + h;
        
        /* Update for next iteration */
        h = a + b + c;
    }
}

int main() {
    /* Call test functions with different parameters */
    test1(5);
    test2(4);
    test3(3);
    test4(2, 3);
    test5(4);
    
    /* Print result to ensure code isn't eliminated */
    printf("Result: %d\n", global_accumulator);
    
    return 0;
}
