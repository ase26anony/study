/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save optimization pass
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are live */
volatile uint64_t global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) 
uint64_t helper1(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    /* Use inline asm to ensure register clobbering */
    uint64_t result;
    asm volatile ("add %1, %0\n\t"
                  "add %2, %0\n\t"
                  "add %3, %0"
                  : "=r" (result)
                  : "r" (a), "r" (b), "r" (c), "0" (d)
                  : "cc");
    return result;
}

__attribute__((noinline, noipa))
uint64_t helper2(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t result;
    asm volatile ("imul %1, %0\n\t"
                  "add %2, %0"
                  : "=r" (result)
                  : "r" (a), "r" (b), "0" (c)
                  : "cc");
    return result;
}

__attribute__((noinline, noipa))
uint64_t helper3(uint64_t a, uint64_t b) {
    uint64_t result;
    asm volatile ("xor %1, %0\n\t"
                  "rol $7, %0"
                  : "=r" (result)
                  : "r" (a), "0" (b)
                  : "cc");
    return result;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline))
uint64_t test1(uint64_t seed) {
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
    
    uint64_t result = 0;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers many registers */
        v1 = helper1(v1, v2, v3, v4);
        
        /* Critical instruction that should be at the end of basic block */
        v5 = v6 + v7;  /* This instruction might need to be moved */
        
        /* Second call - more register clobbering */
        v2 = helper2(v5, v8, v9);
        
        /* Use results to prevent elimination */
        result += v1 + v2 + v3 + v4 + v5;
        
        /* Modify variables to create live ranges across calls */
        v3 = v4 + i;
        v4 = v5 + i;
        v6 = v7 + i;
        v7 = v8 + i;
        v8 = v9 + i;
        v9 = v10 + i;
        v10 = v1 + i;
    }
    
    return result;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline))
uint64_t test2(uint64_t seed) {
    uint64_t a = seed * 3;
    uint64_t b = seed * 5;
    uint64_t c = seed * 7;
    uint64_t d = seed * 11;
    uint64_t e = seed * 13;
    uint64_t f = seed * 17;
    uint64_t g = seed * 19;
    uint64_t h = seed * 23;
    
    uint64_t result = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Inline asm that explicitly clobbers call-used registers */
        asm volatile ("mov %1, %%r11\n\t"
                      "mov %2, %%r12\n\t"
                      "add %%r11, %%r12\n\t"
                      "mov %%r12, %0"
                      : "=r" (a)
                      : "r" (b), "r" (c)
                      : "r11", "r12", "cc");
        
        /* Call that will need save/restore around clobbered registers */
        b = helper3(c, d);
        
        /* Instruction that might be moved to end of basic block */
        c = d + e;  /* Potential candidate for movement */
        
        /* Another call */
        d = helper1(e, f, g, h);
        
        result += a + b + c + d;
        
        /* Rotate values to create different live ranges */
        uint64_t temp = a;
        a = b; b = c; c = d; d = e;
        e = f; f = g; g = h; h = temp;
    }
    
    return result;
}

/* Test function 3: Mix of pointers and scalars */
__attribute__((noinline))
uint64_t test3(uint64_t seed) {
    uint64_t array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = seed + i;
    }
    
    uint64_t *ptr1 = &array[0];
    uint64_t *ptr2 = &array[4];
    uint64_t scalar1 = seed * 2;
    uint64_t scalar2 = seed * 3;
    uint64_t scalar3 = seed * 4;
    uint64_t scalar4 = seed * 5;
    
    uint64_t result = 0;
    
    for (int i = 0; i < 3; i++) {
        /* Dereference pointer - creates complex addressing */
        scalar1 = *ptr1 + scalar2;
        
        /* Call that clobbers registers */
        scalar2 = helper2(scalar1, scalar3, scalar4);
        
        /* Instruction at potential block end */
        *ptr1 = scalar2 + i;  /* Store that might be moved */
        
        /* Another call */
        scalar3 = helper3(*ptr2, scalar1);
        
        /* Update pointers */
        ptr1++;
        if (ptr1 >= &array[8]) ptr1 = &array[0];
        
        result += scalar1 + scalar2 + scalar3 + *ptr1 + *ptr2;
    }
    
    return result;
}

/* Main function that drives all tests */
int main() {
    volatile uint64_t seed = 12345;
    uint64_t total = 0;
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 10; iteration++) {
        seed = seed * 1103515245 + 12345;
        
        total += test1(seed + iteration);
        total += test2(seed + iteration * 2);
        total += test3(seed + iteration * 3);
        
        /* Mix in some direct calls to increase pressure */
        total += helper1(seed, seed+1, seed+2, seed+3);
        total += helper2(seed, seed+4, seed+5);
        total += helper3(seed, seed+6);
    }
    
    global_accumulator = total;
    
    printf("Result: %lu\n", (unsigned long)global_accumulator);
    return 0;
}
