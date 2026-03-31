/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) 
uint64_t helper1(uint64_t a, uint64_t b, uint64_t c) {
    /* Use inline asm to ensure register clobbering */
    uint64_t result;
    asm volatile ("add %1, %2\n\t"
                  "add %3, %2\n\t"
                  "mov %2, %0"
                  : "=r" (result)
                  : "r" (a), "r" (b), "r" (c)
                  : "cc");
    return result;
}

__attribute__((noinline, noipa))
uint64_t helper2(uint64_t a, uint64_t b) {
    /* Another function that clobbers registers */
    uint64_t result;
    asm volatile ("imul %1, %2\n\t"
                  "lea 1(%2), %0"
                  : "=r" (result)
                  : "r" (a), "r" (b)
                  : "cc");
    return result;
}

__attribute__((noinline, noipa))
uint64_t helper3(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    /* Function with many parameters to increase register pressure */
    return (a ^ b) + (c & ~d);
}

/* Test function 1: Many local variables with consecutive calls */
__attribute__((noinline))
void test1(uint64_t seed) {
    /* Declare many local variables to create register pressure */
    register uint64_t v1 asm("r10") = seed + 1;
    register uint64_t v2 asm("r11") = seed + 2;
    register uint64_t v3 asm("r12") = seed + 3;
    uint64_t v4 = seed + 4;
    uint64_t v5 = seed + 5;
    uint64_t v6 = seed + 6;
    uint64_t v7 = seed + 7;
    uint64_t v8 = seed + 8;
    uint64_t v9 = seed + 9;
    uint64_t v10 = seed + 10;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers registers */
        uint64_t t1 = helper1(v1, v2, v3);
        
        /* Critical instruction that should be at the end of basic block */
        v4 = v5 + v6;  /* This instruction might need to be moved */
        
        /* Second call - more register clobbering */
        uint64_t t2 = helper2(v7, v8);
        
        /* Use results to prevent elimination */
        v9 = t1 + t2 + v4;
        v10 += v9;
        
        /* Modify variables to create live ranges across calls */
        v1++;
        v2 += i;
        v3 = v3 * 2 - 1;
    }
    
    /* Ensure all variables are used */
    global_acc += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline))
void test2(uint64_t seed) {
    uint64_t a = seed * 3;
    uint64_t b = seed * 5;
    uint64_t c = seed * 7;
    uint64_t d = seed * 11;
    uint64_t e = seed * 13;
    uint64_t f = seed * 17;
    uint64_t g = seed * 19;
    uint64_t h = seed * 23;
    
    /* Loop with multiple basic blocks */
    for (int j = 0; j < 4; j++) {
        /* Inline asm that explicitly clobbers call-used registers */
        asm volatile ("mov %0, %%r11\n\t"
                      "mov %1, %%r12\n\t"
                      "add %%r12, %%r11"
                      : 
                      : "r" (a), "r" (b)
                      : "r11", "r12", "cc");
        
        /* Call that will need save/restore around it */
        uint64_t tmp = helper3(a, b, c, d);
        
        /* Instruction that might be last in basic block */
        e = f + g;  /* Potential candidate for movement */
        
        /* Another asm clobber */
        asm volatile ("mov %0, %%r13\n\t"
                      "mov %1, %%r14\n\t"
                      "sub %%r14, %%r13"
                      :
                      : "r" (c), "r" (d)
                      : "r13", "r14", "cc");
        
        /* Use variables to keep them live */
        h += tmp + e + j;
        a ^= b;
        b = c + d;
    }
    
    global_acc += a + b + c + d + e + f + g + h;
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline))
void test3(uint64_t seed) {
    uint64_t arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = seed + i;
    }
    
    uint64_t *ptr = arr;
    uint64_t sum = 0;
    uint64_t x1 = seed * 2;
    uint64_t x2 = seed * 3;
    uint64_t x3 = seed * 4;
    uint64_t x4 = seed * 5;
    
    /* Loop with pointer manipulation around calls */
    for (int k = 0; k < 5; k++) {
        /* Dereference and use */
        uint64_t val = *ptr;
        
        /* Call with many arguments */
        uint64_t res1 = helper3(val, x1, x2, x3);
        
        /* Pointer arithmetic that might be last in block */
        ptr++;  /* This could be the instruction that gets moved */
        
        /* Another call */
        uint64_t res2 = helper2(x4, val);
        
        /* Complex live range */
        x1 = res1 + k;
        x2 = res2 - k;
        x3 = x1 ^ x2;
        x4 = x3 & val;
        
        sum += x1 + x2 + x3 + x4;
        
        /* Conditional to create more basic blocks */
        if (k & 1) {
            ptr = arr + (k % 3);
        }
    }
    
    global_acc += sum + x1 + x2 + x3 + x4 + (uint64_t)ptr;
}

/* Main driver */
int main() {
    volatile uint64_t seed = 12345;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int iter = 0; iter < 10; iter++) {
        test1(seed + iter * 100);
        test2(seed + iter * 200);
        test3(seed + iter * 300);
        
        /* Modify seed to prevent constant propagation */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
