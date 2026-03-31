/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls caller-save-test.c -o test
 * For more coverage: gcc -O3 -fno-ipa-ra -mtune=generic -fomit-frame-pointer caller-save-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) 
int helper1(int a, int b) {
    /* Simple operation that uses registers */
    return a + b + 1;
}

__attribute__((noinline, noipa))
int helper2(int a, int b) {
    /* Different operation to prevent merging */
    return a * 2 + b;
}

__attribute__((noinline, noipa))
long helper3(long a, long b) {
    /* Uses different register types */
    return a - b;
}

__attribute__((noinline, noipa))
long helper4(long a, long b) {
    return a ^ b;
}

/* Test function 1: High integer register pressure */
__attribute__((noinline))
void test1(int seed) {
    /* Declare many local variables to create register pressure */
    register int v1 asm("r10") = seed;
    register int v2 asm("r11") = seed + 1;
    int v3 = seed + 2;
    int v4 = seed + 3;
    int v5 = seed + 4;
    int v6 = seed + 5;
    int v7 = seed + 6;
    int v8 = seed + 7;
    int v9 = seed + 8;
    int v10 = seed + 9;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Multiple calls that clobber registers */
        v1 = helper1(v1, v2);
        v2 = helper2(v2, v3);
        
        /* Critical instruction that should be at the end of a basic block */
        /* This increment uses v3 which is live across the calls */
        v3 = v4 + v5;  /* This could be moved by caller-save */
        
        /* More computations to keep variables live */
        v4 = helper1(v4, v5);
        v5 = helper2(v5, v6);
        
        v6 = v7 + v8;  /* Another potential move candidate */
        
        /* Use results to prevent elimination */
        global_acc += v1 + v2 + v3 + v4 + v5 + v6;
    }
}

/* Test function 2: Mixed register types with explicit clobbers */
__attribute__((noinline))
void test2(long seed) {
    long l1 = seed;
    long l2 = seed * 2;
    long l3 = seed * 3;
    long l4 = seed * 4;
    long l5 = seed * 5;
    long l6 = seed * 6;
    long l7 = seed * 7;
    long l8 = seed * 8;
    
    /* Use inline asm to explicitly clobber call-used registers */
    for (int i = 0; i < 4; i++) {
        /* First call */
        l1 = helper3(l1, l2);
        
        /* Inline asm that clobbers specific registers */
        asm volatile("" : : : "r11", "r12", "r13");
        
        /* Instruction that might need to be moved */
        l2 = l3 + l4;  /* Could become BB_END */
        
        /* Second call */
        l3 = helper4(l3, l4);
        
        /* More operations */
        l4 = l5 - l6;
        
        /* Third call - creates more pressure */
        l5 = helper3(l5, l6);
        
        /* Another potential end-of-block instruction */
        l6 = l7 * l8;
        
        global_acc += l1 + l2 + l3 + l4 + l5 + l6;
    }
}

/* Test function 3: Pointer arithmetic and memory operations */
__attribute__((noinline))
void test3(int seed) {
    int arr[20];
    for (int i = 0; i < 20; i++) {
        arr[i] = seed + i;
    }
    
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[10];
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    /* Complex loop with pointer updates around calls */
    for (int i = 0; i < 5; i++) {
        /* Dereference and computation before call */
        sum1 = *ptr1 + *(ptr1 + 1);
        
        /* Call that clobbers registers */
        sum1 = helper1(sum1, i);
        
        /* Pointer update that could be at block end */
        ptr1 += 2;  /* This might be BB_END before movement */
        
        /* Another dereference */
        sum2 = *ptr2 + *(ptr2 - 1);
        
        /* Second call */
        sum2 = helper2(sum2, i);
        
        /* Another pointer update */
        ptr2 -= 1;  /* Another potential BB_END */
        
        /* More computations */
        sum3 = helper1(sum1, sum2);
        sum4 = helper2(sum3, *ptr1);
        
        global_acc += sum1 + sum2 + sum3 + sum4;
    }
}

/* Test function 4: Nested loops with varying pressure */
__attribute__((noinline))
void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    /* Outer loop */
    for (int i = 0; i < 2; i++) {
        /* Inner loop with calls */
        for (int j = 0; j < 3; j++) {
            /* Multiple consecutive calls */
            a = helper1(a, b);
            b = helper2(b, c);
            
            /* Critical instruction - increment of loop variable */
            /* This is often at the end of a basic block */
            c = d + e;  /* Could be moved by caller-save */
            
            /* More calls */
            d = helper1(d, e);
            e = helper2(e, f);
            
            /* Another instruction that might end a block */
            f = g + h;
            
            /* Use results */
            global_acc += a + b + c + d + e + f;
        }
        
        /* Reset some values */
        g = helper1(g, h);
        h = helper2(h, a);
    }
}

int main() {
    volatile int seed = 12345;
    
    /* Call test functions multiple times with varying seeds */
    for (int i = 0; i < 10; i++) {
        int current_seed = seed + i * 100;
        
        test1(current_seed);
        test2(current_seed);
        test3(current_seed);
        test4(current_seed);
        
        /* Modify seed to create different patterns */
        seed = helper1(seed, i);
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
