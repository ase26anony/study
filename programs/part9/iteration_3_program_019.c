/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) uint64_t helper1(uint64_t a, uint64_t b) {
    /* Use inline asm to ensure register clobbering */
    uint64_t res;
    asm volatile ("addq %1, %2\n\t"
                  "movq %2, %0"
                  : "=r" (res)
                  : "r" (a), "r" (b)
                  : "cc");
    return res;
}

__attribute__((noinline, noipa)) uint64_t helper2(uint64_t a, uint64_t b, uint64_t c) {
    /* Clobber multiple call-used registers */
    uint64_t res;
    asm volatile ("imulq %1, %2\n\t"
                  "addq %3, %2\n\t"
                  "movq %2, %0"
                  : "=r" (res)
                  : "r" (a), "r" (b), "r" (c)
                  : "cc", "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return res;
}

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a) {
    /* Simple operation that can't be optimized away */
    return a ^ 0xDEADBEEF;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) uint64_t test1(uint64_t seed) {
    /* Declare many local variables to increase register pressure */
    register uint64_t v1 asm("r10") = seed + 1;
    register uint64_t v2 asm("r11") = seed + 2;
    uint64_t v3 = seed + 3;
    uint64_t v4 = seed + 4;
    uint64_t v5 = seed + 5;
    uint64_t v6 = seed + 6;
    uint64_t v7 = seed + 7;
    uint64_t v8 = seed + 8;
    uint64_t v9 = seed + 9;
    uint64_t v10 = seed + 10;
    
    uint64_t acc = 0;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers registers */
        uint64_t t1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last in basic block
           and may need to be moved by caller-save */
        v3 = v4 + v5;  /* This instruction should be at BB end */
        
        /* Second call - more register clobbering */
        uint64_t t2 = helper2(v3, v6, v7);
        
        /* Use results to prevent elimination */
        acc += t1 + t2;
        
        /* Modify variables to create live ranges across calls */
        v1 = v2 ^ v8;
        v2 = v3 + v9;
        v4 = v5 * v10;
        v5 = v6 - v7;
        
        /* Another instruction that could be at BB end */
        v8 = v9 << 2;
    }
    
    /* Final computation that uses all variables */
    acc += helper3(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10);
    
    return acc;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) uint64_t test2(uint64_t seed) {
    uint64_t a = seed * 3;
    uint64_t b = seed * 5;
    uint64_t c = seed * 7;
    uint64_t d = seed * 11;
    uint64_t e = seed * 13;
    uint64_t f = seed * 17;
    uint64_t g = seed * 19;
    uint64_t h = seed * 23;
    
    uint64_t acc = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Call that explicitly clobbers specific registers */
        uint64_t r1 = helper2(a, b, c);
        
        /* Instruction that should be at BB end */
        d = e + f;  /* Candidate for movement */
        
        /* Inline asm that clobbers call-used registers */
        uint64_t r2;
        asm volatile ("movq %1, %%r11\n\t"
                      "movq %2, %%r12\n\t"
                      "addq %%r11, %%r12\n\t"
                      "movq %%r12, %0\n\t"
                      : "=r" (r2)
                      : "r" (d), "r" (g)
                      : "r11", "r12", "cc");
        
        acc += r1 + r2;
        
        /* Rotate values to create different live ranges */
        uint64_t tmp = a;
        a = b; b = c; c = d; d = e; e = f; f = g; g = h; h = tmp;
        
        /* Another potential BB-end instruction */
        h = a ^ b;
    }
    
    return acc;
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) uint64_t test3(uint64_t seed) {
    uint64_t arr[8] = {seed, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7};
    uint64_t *ptr = arr;
    uint64_t idx = 0;
    uint64_t sum = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Load from pointer - creates register pressure */
        uint64_t val1 = ptr[idx];
        
        /* Call that may require saving registers */
        uint64_t t1 = helper1(val1, seed);
        
        /* Store operation that could be at BB end */
        ptr[(idx + 1) & 7] = t1;  /* Candidate for movement */
        
        /* Another call */
        uint64_t t2 = helper3(ptr[(idx + 2) & 7]);
        
        sum += t1 + t2;
        
        /* Update index and pointer arithmetic */
        idx = (idx + 3) & 7;
        
        /* Pointer arithmetic that could be at BB end */
        ptr = arr + idx;
    }
    
    return sum;
}

/* Main function that drives all tests */
int main() {
    volatile uint64_t seed = 12345;
    uint64_t total = 0;
    
    /* Run tests multiple times to increase coverage chances */
    for (int iter = 0; iter < 100; iter++) {
        total += test1(seed + iter);
        total += test2(seed + iter * 2);
        total += test3(seed + iter * 3);
        
        /* Modify seed to prevent constant propagation */
        seed = seed * 1103515245 + 12345;
    }
    
    global_acc = total;
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    
    return 0;
}
