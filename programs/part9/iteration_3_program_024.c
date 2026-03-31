/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    return a * b - 1;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    return (a << 3) | (b & 0xFF);
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a ^ b;
}

__attribute__((noinline, noipa)) long helper5(long a, long b) {
    return a + (b >> 2);
}

/* Test 1: High register pressure with int variables */
__attribute__((noinline)) void test1(int seed) {
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
        /* Use variables in computations to keep them live */
        v1 = v2 + v3;
        v4 = v5 * v6;
        
        /* First call - clobbers call-used registers */
        v7 = helper1(v1, v4);
        
        /* Critical instruction: This should be the last in its basic block
         * and may need to be moved by caller-save */
        v8 = v7 + v9;  /* This instruction might be moved */
        
        /* Second call - more register clobbering */
        v10 = helper2(v8, v7);
        
        /* Use result to prevent elimination */
        global_accumulator += v10;
        
        /* Rotate values to create data dependencies */
        v9 = v10;
        v2 = v3 + i;
    }
}

/* Test 2: Explicit register clobbering with asm and long variables */
__attribute__((noinline)) void test2(int seed) {
    long l1 = seed * 2L;
    long l2 = seed * 3L;
    long l3 = seed * 4L;
    long l4 = seed * 5L;
    long l5 = seed * 6L;
    long l6 = seed * 7L;
    
    /* Use inline asm to explicitly clobber call-used registers */
    for (int i = 0; i < 4; i++) {
        /* Force use of specific registers */
        asm volatile("" : "+r"(l1), "+r"(l2), "+r"(l3));
        
        /* Call that clobbers registers */
        l4 = helper4(l1, l2);
        
        /* Critical instruction - should be last in basic block */
        l5 = l3 + l4;  /* Candidate for movement */
        
        /* Another call */
        l6 = helper5(l5, l4);
        
        global_accumulator += (int)l6;
        
        /* Create data dependencies for next iteration */
        l3 = l6;
        l1 = l2 + i;
    }
}

/* Test 3: Mixed scalar and pointer operations */
__attribute__((noinline)) void test3(int seed) {
    int arr[8] = {seed, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7};
    int *ptr = arr;
    int idx = 0;
    int sum = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Load from pointer - creates register pressure */
        int val1 = ptr[idx];
        int val2 = ptr[(idx + 1) & 7];
        
        /* Call that clobbers registers */
        int tmp1 = helper1(val1, val2);
        
        /* Critical instruction - pointer update that might be moved */
        idx = (idx + tmp1) & 7;  /* This could be the last instruction in BB */
        
        /* Another call */
        int tmp2 = helper3(tmp1, val2);
        
        sum += tmp2;
        
        /* Update pointer to create live range across calls */
        ptr = arr + idx;
    }
    
    global_accumulator += sum;
}

/* Test 4: Complex live ranges spanning multiple calls */
__attribute__((noinline)) void test4(int seed) {
    /* Create complex web of dependencies */
    int a = seed;
    int b = seed * 2;
    int c = seed * 3;
    int d = seed * 4;
    int e = seed * 5;
    
    /* Unrolled loop to create more basic blocks */
    for (int i = 0; i < 2; i++) {
        /* First basic block with call */
        a = helper1(b, c);
        
        /* Instruction that should be at end of BB */
        d = a + e;  /* Candidate for movement */
        
        /* Second call in same BB */
        e = helper2(d, a);
        
        /* Use result */
        global_accumulator += e;
        
        /* Create new dependencies */
        b = c + i;
        c = d - i;
    }
}

/* Test 5: Nested calls with intervening instructions */
__attribute__((noinline)) void test5(int seed) {
    int x1 = seed;
    int x2 = seed + 11;
    int x3 = seed + 22;
    int x4 = seed + 33;
    int x5 = seed + 44;
    
    /* Create a basic block ending with arithmetic after a call */
    for (int j = 0; j < 3; j++) {
        /* Multiple calls in sequence */
        x1 = helper1(x2, x3);
        x4 = helper2(x1, x5);
        
        /* This instruction should be at the end of BB and might be moved */
        x5 = x4 - x3;  /* Critical instruction */
        
        /* Another call */
        x3 = helper3(x5, x1);
        
        global_accumulator += x3;
        
        /* Update for next iteration */
        x2 = x3 + j;
    }
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int iter = 0; iter < 2; iter++) {
        test1(seed + iter * 100);
        test2(seed + iter * 200);
        test3(seed + iter * 300);
        test4(seed + iter * 400);
        test5(seed + iter * 500);
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
