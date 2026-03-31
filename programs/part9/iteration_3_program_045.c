/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) 
int helper1(int a, int b, int c, int d, int e, int f) {
    /* Use all arguments to force register pressure */
    return a + b - c + d - e + f;
}

__attribute__((noinline, noipa))
long helper2(long a, long b, long c, long d) {
    /* Force register usage with long operations */
    return (a * b) + (c * d);
}

__attribute__((noinline, noipa))
void* helper3(void* p, int offset) {
    /* Pointer arithmetic that might use different registers */
    return (char*)p + offset;
}

__attribute__((noinline, noipa))
int helper4(int a, int b) {
    /* Simple operation but marked noipa to prevent interprocedural analysis */
    asm volatile("" : "+r"(a), "+r"(b) : : "memory");
    return a ^ b;
}

/* Test function 1: High integer register pressure */
__attribute__((noinline))
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
    
    /* Loop to create basic blocks with calls */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        int res1 = helper1(v1, v2, v3, v4, v5, v6);
        
        /* Critical instruction: This should be the last in basic block
         * before the second call. The caller-save pass may need to move it
         * when inserting spill/restore code. */
        v7 = v8 + v9;  /* This instruction might need to be moved */
        
        /* Second call - more register pressure */
        int res2 = helper4(v7, res1);
        
        /* Use results to prevent elimination */
        global_acc += res1 + res2 + v10;
        
        /* Modify variables to create live ranges across calls */
        v1++;
        v2 += res1;
        v3 = v4 ^ v5;
        
        /* This increment might also be a candidate for movement */
        v10 = v9 - v8;
    }
}

/* Test function 2: Mixed long and pointer operations */
__attribute__((noinline))
void test2(long seed) {
    long l1 = seed * 1;
    long l2 = seed * 2;
    long l3 = seed * 3;
    long l4 = seed * 4;
    long l5 = seed * 5;
    long l6 = seed * 6;
    int arr[10] = {0};
    int* ptr = arr;
    
    for (int i = 0; i < 4; i++) {
        /* First helper call with long arguments */
        long res1 = helper2(l1, l2, l3, l4);
        
        /* Critical store instruction - should be at end of basic block */
        *ptr = (int)res1 + i;
        
        /* Explicit asm to clobber specific registers */
        asm volatile("" : : "r"(l5), "r"(l6) : "r11", "r12", "memory");
        
        /* Second call */
        void* new_ptr = helper3(ptr, sizeof(int));
        
        /* Instruction that might need moving relative to spill code */
        l5 = l6 * 2;
        
        /* Use results */
        global_acc += (uint64_t)new_ptr + res1 + l5;
        
        /* Update variables for next iteration */
        ptr = (int*)new_ptr;
        l1 += l2;
        l3 = l4 ^ l5;
        
        /* Another potential instruction for movement */
        l6 = l1 - l2;
    }
}

/* Test function 3: Complex live ranges with volatile */
__attribute__((noinline))
void test3(int seed) {
    volatile int v1 = seed;
    int v2 = seed * 2;
    int v3 = seed * 3;
    int v4 = seed * 4;
    register int v5 asm("r13") = seed * 5;
    register int v6 asm("r14") = seed * 6;
    
    /* Create a small basic block ending with a modifiable instruction */
    if (v1 > 0) {
        int temp = v2 + v3;
        
        /* Call that clobbers registers */
        int res = helper4(temp, v4);
        
        /* This assignment should be at the end of the basic block
         * and might need moving when caller-save inserts code */
        v5 = v6 * res;
        
        /* Another call */
        int res2 = helper1(v5, v6, v2, v3, v4, res);
        
        global_acc += res2;
        
        /* Branch creates basic block boundaries */
        if (res2 > 100) {
            v6 = v5 + 1;  /* Another end-of-block candidate */
        } else {
            v6 = v5 - 1;
        }
    }
    
    /* Loop with multiple basic blocks */
    for (int i = 0; i < 2; i++) {
        /* Multiple calls in sequence */
        int a = helper4(v1, v2);
        int b = helper4(v3, v4);
        
        /* Instruction between calls - candidate for movement */
        v2 = v3 + v4;
        
        int c = helper4(v5, v6);
        
        /* Complex expression that might be split */
        v1 = (a * b) / (c + 1);
        
        global_acc += a + b + c + v1 + v2;
    }
}

/* Main driver */
int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test1(seed + i * 100);
        test2(seed + i * 200);
        test3(seed + i * 300);
        
        /* Modify seed to prevent constant propagation */
        seed += 777;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
