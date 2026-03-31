/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -mtune=generic -fomit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
volatile uint64_t global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) uint64_t helper1(uint64_t a, uint64_t b, uint64_t c) {
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

__attribute__((noinline, noipa)) uint64_t helper2(uint64_t a, uint64_t b) {
    /* Another helper that clobbers different registers */
    uint64_t result;
    asm volatile ("imul %1, %2\n\t"
                  "lea 1(%2), %0"
                  : "=r" (result)
                  : "r" (a), "r" (b)
                  : "cc");
    return result;
}

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    /* Helper with more arguments to increase register pressure */
    return (a ^ b) + (c | d);
}

/* Test function 1: Many local variables with consecutive calls */
__attribute__((noinline)) uint64_t test1(uint64_t seed) {
    /* Declare many local variables to create register pressure */
    register uint64_t var1 asm("r10") = seed + 1;
    register uint64_t var2 asm("r11") = seed + 2;
    register uint64_t var3 asm("r12") = seed + 3;
    uint64_t var4 = seed + 4;
    uint64_t var5 = seed + 5;
    uint64_t var6 = seed + 6;
    uint64_t var7 = seed + 7;
    uint64_t var8 = seed + 8;
    uint64_t var9 = seed + 9;
    uint64_t var10 = seed + 10;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers registers */
        uint64_t tmp1 = helper1(var1, var2, var3);
        
        /* Critical instruction: This should be the last in basic block
         * and may need to be moved by caller-save */
        var4 = var5 + var6 + i;  /* This instruction might become BB_END */
        
        /* Second call - more register clobbering */
        uint64_t tmp2 = helper2(var7, var8);
        
        /* Use results to prevent elimination */
        var9 += tmp1;
        var10 += tmp2;
        
        /* Modify variables to create live ranges across calls */
        var1 += var2;
        var2 += var3;
        var3 += var4;
    }
    
    /* Return a combination to ensure all variables are used */
    return var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
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
    
    /* Loop with multiple basic blocks */
    for (int j = 0; j < 4; j++) {
        /* Call that uses many registers */
        uint64_t res1 = helper3(a, b, c, d);
        
        /* Instruction that should be at end of basic block */
        e = f + g + j;  /* Potential BB_END candidate */
        
        /* Inline asm that explicitly clobbers call-used registers */
        asm volatile ("mov %1, %%r10\n\t"
                      "mov %2, %%r11\n\t"
                      "add %%r10, %%r11\n\t"
                      "mov %%r11, %0"
                      : "=r" (h)
                      : "r" (res1), "r" (e)
                      : "r10", "r11", "cc");
        
        /* Another call */
        uint64_t res2 = helper2(h, a);
        
        /* Update variables */
        a += res1;
        b += res2;
        c += h;
    }
    
    return a + b + c + d + e + f + g + h;
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) uint64_t test3(uint64_t seed) {
    uint64_t buffer[8];
    for (int i = 0; i < 8; i++) {
        buffer[i] = seed + i;
    }
    
    uint64_t *ptr = buffer;
    uint64_t sum = 0;
    uint64_t tmp1 = seed;
    uint64_t tmp2 = seed * 2;
    uint64_t tmp3 = seed * 3;
    uint64_t tmp4 = seed * 4;
    
    /* Loop with pointer arithmetic and calls */
    for (int i = 0; i < 4; i++) {
        /* First helper call */
        uint64_t res1 = helper1(*ptr, tmp1, tmp2);
        
        /* Pointer update - could be last in basic block */
        ptr++;  /* Potential BB_END candidate */
        
        /* Second helper call */
        uint64_t res2 = helper2(tmp3, tmp4);
        
        /* Use results */
        sum += res1 + res2;
        
        /* Update variables to keep them live */
        tmp1 = res1;
        tmp2 = res2;
        tmp3 += *ptr;
        tmp4 += i;
    }
    
    return sum + tmp1 + tmp2 + tmp3 + tmp4;
}

/* Main function that drives all tests */
int main() {
    volatile uint64_t seed = 12345;
    uint64_t result = 0;
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 10; iteration++) {
        uint64_t s = seed + iteration;
        
        /* Call test functions with different patterns */
        result += test1(s);
        result += test2(s * 7919);  /* Use prime multiplier for variation */
        result += test3(s * 104729);
        
        /* Modify seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Add to global accumulator and print */
    global_accumulator += result;
    printf("Result: %lu\n", (unsigned long)global_accumulator);
    
    return 0;
}
