/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile int global_acc = 0;

/* Non-inlineable helper functions that clobber registers */
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

__attribute__((noinline, noipa)) int helper4(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return (a << 2) | (b & 0xF);
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(int seed) {
    /* Declare many local variables to create register pressure */
    register int v1 asm ("r10") = seed + 1;
    register int v2 asm ("r11") = seed + 2;
    register int v3 asm ("r12") = seed + 3;
    int v4 = seed + 4;
    int v5 = seed + 5;
    int v6 = seed + 6;
    int v7 = seed + 7;
    int v8 = seed + 8;
    int v9 = seed + 9;
    int v10 = seed + 10;
    
    /* Loop to create basic blocks with calls */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        int t1 = helper1(v1, v2);
        
        /* Critical instruction that should be at the end of basic block */
        /* This instruction uses v3 which must be preserved across calls */
        v3 = v4 + v5 + i;  /* This could be moved by caller-save */
        
        /* Second call - forces spill/restore around v3 usage */
        int t2 = helper2(v3, v6);
        
        /* Use results to prevent elimination */
        v7 = t1 + t2;
        v8 = v7 * v9;
        v9 = v8 - v10;
        v10 = v9 ^ v1;
        
        /* Another pair of calls with different register usage */
        int t3 = helper3(v2, v3);
        v4 = v5 + v6;  /* Another candidate for movement */
        int t4 = helper4(v4, v7);
        
        v1 = t3 + t4;
        v2 = v1 * 3;
    }
    
    /* Accumulate to global to ensure code isn't dead */
    global_acc += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) void test2(int seed) {
    int a = seed * 2;
    int b = seed * 3;
    int c = seed * 4;
    int d = seed * 5;
    int e = seed * 6;
    int f = seed * 7;
    int g = seed * 8;
    int h = seed * 9;
    
    /* Loop with mixed operations */
    for (int j = 0; j < 4; j++) {
        /* Call that clobbers specific registers */
        int r1 = helper1(a, b);
        
        /* Instruction that should be at block end */
        c = d + e + j;  /* Candidate for movement */
        
        /* Inline asm that explicitly clobbers call-used registers */
        asm volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0"
            : "+r"(c)
            : "r"(r1)
            : "eax", "memory"
        );
        
        /* Another call */
        int r2 = helper2(c, f);
        
        /* More operations */
        g = h + r2;
        h = g - a;
        a = b + c;
        b = a * 2;
        
        /* Final call in the block */
        int r3 = helper3(g, h);
        d = e + f;  /* Another end-of-block candidate */
        int r4 = helper4(d, r3);
        
        e = r4 ^ 0xFF;
        f = e >> 2;
    }
    
    global_acc += a + b + c + d + e + f + g + h;
}

/* Test function 3: Pointer manipulation creating complex live ranges */
__attribute__((noinline)) void test3(int seed) {
    int data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[4];
    int sum1 = 0, sum2 = 0;
    
    /* Unrolled loop to create specific basic block structure */
    for (int k = 0; k < 2; k++) {
        /* Load values into registers */
        int val1 = *ptr1;
        int val2 = *(ptr1 + 1);
        
        /* Call that clobbers registers */
        int t1 = helper1(val1, val2);
        
        /* Pointer arithmetic that should be at block end */
        ptr1 += 2;  /* Candidate for movement */
        
        /* Another call */
        int t2 = helper2(t1, *ptr2);
        
        /* More operations */
        sum1 += t2;
        *ptr2 = sum1;
        
        /* Second pair of calls */
        int val3 = *ptr2;
        int val4 = *(ptr2 + 1);
        int t3 = helper3(val3, val4);
        ptr2 += 1;  /* Another candidate */
        int t4 = helper4(t3, sum1);
        
        sum2 += t4;
    }
    
    global_acc += sum1 + sum2 + *ptr1 + *ptr2;
}

/* Test function 4: Nested loops with varying pressure */
__attribute__((noinline)) void test4(int seed) {
    int x1 = seed, x2 = seed + 1, x3 = seed + 2;
    int y1 = seed + 3, y2 = seed + 4, y3 = seed + 5;
    int z1 = seed + 6, z2 = seed + 7, z3 = seed + 8;
    
    /* Outer loop */
    for (int i = 0; i < 2; i++) {
        /* Inner loop to create more basic blocks */
        for (int j = 0; j < 2; j++) {
            /* First call in the block */
            int r1 = helper1(x1, x2);
            
            /* Critical instruction - should be last in block */
            x3 = y1 + y2 + j;  /* Candidate for movement */
            
            /* Second call */
            int r2 = helper2(x3, y3);
            
            /* Update variables */
            z1 = r1 + r2;
            z2 = z1 * x1;
            z3 = z2 - x2;
            
            /* Another sequence */
            int r3 = helper3(y1, y2);
            y3 = z1 + z2;  /* Candidate */
            int r4 = helper4(y3, z3);
            
            x1 = r3 ^ r4;
            x2 = x1 + i;
        }
        
        /* Additional operations between loops */
        y1 = helper1(z1, z2);
        z1 = y1 + y2;  /* Candidate */
        y2 = helper2(z1, z3);
    }
    
    global_acc += x1 + x2 + x3 + y1 + y2 + y3 + z1 + z2 + z3;
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times with varying inputs */
    for (int iter = 0; iter < 10; iter++) {
        int base = seed + iter * 100;
        
        test1(base);
        test2(base + 1);
        test3(base + 2);
        test4(base + 3);
        
        /* Modify seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Result: %d\n", global_acc);
    return 0;
}
