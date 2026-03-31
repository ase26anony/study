/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller_save_test.c -o caller_save_test */

#include <stdio.h>
#include <stdlib.h>

/* Global accumulator to prevent dead code elimination */
volatile long global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    volatile int result = a + b + 1;
    return result;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    volatile int result = a * b - 1;
    return result;
}

__attribute__((noinline, noipa)) long helper3(long a, long b) {
    volatile long result = (a ^ b) + (a & b);
    return result;
}

__attribute__((noinline, noipa)) void helper4(int *ptr) {
    *ptr = (*ptr * 3) / 2;
}

/* Test function 1: Many integer variables, consecutive calls */
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
        int tmp1 = helper1(v1, v2);
        
        /* Instruction that might need to be moved:
           Uses variables that must be preserved across calls */
        v3 = v4 + v5;  /* This could be the last instruction in BB */
        
        /* Second call - more register clobbering */
        int tmp2 = helper2(v6, v7);
        
        /* More computations to use all variables */
        v8 = v9 * v10;
        v1 = tmp1 + tmp2;
        v2 = v3 ^ v8;
        
        /* Use results to prevent elimination */
        global_accumulator += v1 + v2 + v3 + v8;
    }
}

/* Test function 2: Long variables with explicit asm clobbers */
void test2(long seed) {
    long l1 = seed * 2;
    long l2 = seed * 3;
    long l3 = seed * 4;
    long l4 = seed * 5;
    long l5 = seed * 6;
    long l6 = seed * 7;
    
    /* Inline asm to explicitly clobber call-used registers */
    asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
    
    for (int i = 0; i < 4; i++) {
        /* Call that uses long arguments */
        long tmp1 = helper3(l1, l2);
        
        /* Critical instruction - could be at end of BB */
        l3 = l4 - l5;
        
        /* Another computation that might force spill */
        l6 = helper3(l3, tmp1);
        
        /* Mix computations to keep variables live */
        l1 = l2 ^ l6;
        l2 = l3 + l4;
        l4 = l5 * 2;
        
        global_accumulator += l1 + l2 + l3 + l6;
        
        /* Additional asm to prevent optimizations */
        asm volatile("" : "+r"(l1), "+r"(l2));
    }
}

/* Test function 3: Mix of pointers and scalars */
void test3(int seed) {
    int data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[4];
    int scalar1 = seed * 2;
    int scalar2 = seed * 3;
    int scalar3 = seed * 4;
    
    for (int i = 0; i < 5; i++) {
        /* First call with pointer argument */
        helper4(ptr1);
        
        /* Instruction that might need moving - pointer arithmetic */
        ptr1++;  /* Could be last instruction in BB */
        
        /* Call that uses scalar values */
        scalar1 = helper1(scalar2, scalar3);
        
        /* More operations */
        *ptr2 = scalar1 + *ptr1;
        scalar2 = helper2(*ptr2, scalar3);
        scalar3 = scalar1 ^ scalar2;
        
        /* Use results */
        global_accumulator += *ptr1 + *ptr2 + scalar1 + scalar2;
        
        /* Conditional to create basic block boundaries */
        if (ptr1 < &data[7]) {
            ptr2--;
        }
    }
}

/* Test function 4: Complex live ranges across calls */
void test4(int seed) {
    /* Variables with complex interdependencies */
    int a = seed;
    int b = seed + 1;
    int c = seed + 2;
    int d = seed + 3;
    int e = seed + 4;
    int f = seed + 5;
    
    /* Loop with multiple basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Basic block 1 */
        a = helper1(b, c);
        
        /* This instruction's result is needed after the next call */
        d = e + f;  /* Candidate for movement */
        
        /* Basic block 2 - another call */
        b = helper2(c, d);
        
        /* Use all variables to keep them live */
        c = a ^ b;
        e = d * 2;
        f = helper1(e, c);
        
        global_accumulator += a + b + c + d + e + f;
        
        /* Create another basic block with a conditional */
        if (a > b) {
            /* Another potential movement location */
            c = d - e;  /* Could be at end of this BB */
            
            /* Call in conditional block */
            a = helper2(f, c);
        } else {
            e = f + a;
        }
    }
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times with varying inputs */
    for (int i = 0; i < 10; i++) {
        test1(seed + i * 100);
        test2(seed + i * 200);
        test3(seed + i * 300);
        test4(seed + i * 400);
        
        /* Modify seed to prevent constant propagation */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %ld\n", global_accumulator);
    return 0;
}
