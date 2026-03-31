/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) 
int helper1(int a, int b) {
    /* Use inline asm to ensure register clobbering */
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a + b + 1;
}

__attribute__((noinline, noipa))
int helper2(int a, int b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return a * b - 1;
}

__attribute__((noinline, noipa))
long helper3(long a, long b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return (a ^ b) + 1;
}

__attribute__((noinline, noipa))
long helper4(long a, long b) {
    asm volatile ("" : : "r"(a), "r"(b) : "memory");
    return (a | b) - 1;
}

/* Test function 1: High register pressure with int variables */
__attribute__((noinline))
void test1(int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed + 2;
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
        /* Use all variables in computations to keep them live */
        v1 = v2 + v3;
        v2 = v3 + v4;
        v3 = v4 + v5;
        v4 = v5 + v6;
        
        /* First call - clobbers call-used registers */
        int t1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last in its basic block
           and may need to be moved by caller-save */
        v5 = v6 + v7;  /* This instruction might be moved */
        
        /* Second call - more register clobbering */
        int t2 = helper2(v3, v4);
        
        /* Use results to prevent elimination */
        v6 = t1 + t2;
        v7 = v6 * v5;
        
        /* More computations to increase pressure */
        v8 = v7 ^ v1;
        v9 = v8 | v2;
        v10 = v9 & v3;
    }
    
    /* Accumulate results */
    global_acc += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 2: Use long variables and explicit register clobbering */
__attribute__((noinline))
void test2(long seed) {
    /* Use 'register' hints to suggest specific registers */
    register long r1 asm ("r11") = seed + 100;
    register long r2 asm ("r12") = seed + 200;
    register long r3 asm ("r13") = seed + 300;
    long r4 = seed + 400;
    long r5 = seed + 500;
    long r6 = seed + 600;
    long r7 = seed + 700;
    long r8 = seed + 800;
    
    for (int i = 0; i < 4; i++) {
        /* Complex computation chain */
        r1 = r2 ^ r3;
        r2 = r3 | r4;
        r3 = r4 & r5;
        
        /* Call that clobbers registers */
        long t1 = helper3(r1, r2);
        
        /* Critical instruction - last in block candidate */
        r4 = r5 + r6;  /* May be moved by caller-save */
        
        /* Another call */
        long t2 = helper4(r3, r4);
        
        /* More computations */
        r5 = t1 * t2;
        r6 = r5 - r1;
        r7 = r6 / (r2 + 1);
        r8 = r7 << 2;
        
        /* Explicit asm to clobber specific registers */
        asm volatile ("" : : "r"(r1), "r"(r2), "r"(r3), "r"(r4) : 
                     "r11", "r12", "r13", "r14", "r15");
    }
    
    global_acc += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
}

/* Test function 3: Mix pointers and scalars */
__attribute__((noinline))
void test3(int seed) {
    int data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i * 10;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[4];
    int sum1 = 0, sum2 = 0;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointers */
        int val1 = *ptr1;
        int val2 = *ptr2;
        
        /* Call with pointer-derived values */
        sum1 = helper1(val1, val2);
        
        /* Critical instruction - pointer update that might be moved */
        ptr1++;  /* Last instruction candidate */
        
        /* Another call */
        sum2 = helper2(sum1, val2);
        
        /* More pointer arithmetic */
        ptr2--;
        *ptr1 = sum1 + sum2;
        *ptr2 = sum1 - sum2;
        
        /* Use asm to prevent optimization */
        asm volatile ("" : : "r"(ptr1), "r"(ptr2) : "memory");
    }
    
    for (int i = 0; i < 8; i++) {
        global_acc += data[i];
    }
}

/* Test function 4: Nested loops with calls */
__attribute__((noinline))
void test4(int seed) {
    int a = seed, b = seed * 2, c = seed * 3;
    int d = seed * 4, e = seed * 5, f = seed * 6;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 2; inner++) {
            /* Multiple computations */
            a = b + c;
            b = c + d;
            c = d + e;
            
            /* First call */
            int tmp1 = helper1(a, b);
            
            /* Critical instruction - simple assignment */
            d = e + f;  /* May be moved */
            
            /* Second call */
            int tmp2 = helper2(c, d);
            
            /* Use results */
            e = tmp1 * tmp2;
            f = e >> 2;
            
            /* Conditional to create basic block boundaries */
            if (inner == 0) {
                a = helper1(b, c);
            } else {
                b = helper2(a, c);
            }
        }
    }
    
    global_acc += a + b + c + d + e + f;
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times with varying seeds */
    for (int iteration = 0; iteration < 10; iteration++) {
        int current_seed = seed + iteration * 100;
        
        test1(current_seed);
        test2(current_seed);
        test3(current_seed);
        test4(current_seed);
        
        /* Modify seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
