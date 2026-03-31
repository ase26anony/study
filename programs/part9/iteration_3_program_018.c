/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile uint64_t global_acc = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) uint64_t helper1(uint64_t a, uint64_t b) {
    /* Use inline asm to ensure register clobbering */
    uint64_t result;
    asm volatile ("addq %1, %0" : "=r"(result) : "r"(a), "0"(b) : "cc");
    return result;
}

__attribute__((noinline, noipa)) uint64_t helper2(uint64_t a, uint64_t b) {
    uint64_t result;
    asm volatile ("subq %1, %0" : "=r"(result) : "r"(a), "0"(b) : "cc");
    return result;
}

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a) {
    /* Clobber specific call-used registers on x86-64 */
    asm volatile ("" : : : "r11", "r12", "r13", "r14", "r15");
    return a + 1;
}

__attribute__((noinline, noipa)) uint64_t helper4(uint64_t a, uint64_t b, uint64_t c) {
    asm volatile ("" : : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
    return a + b + c;
}

/* Test function 1: Many local variables with consecutive calls */
__attribute__((noinline)) uint64_t test1(uint64_t seed) {
    /* Declare many local variables to create register pressure */
    register uint64_t var1 asm("r10") = seed + 1;
    register uint64_t var2 asm("r11") = seed + 2;
    register uint64_t var3 asm("r12") = seed + 3;
    register uint64_t var4 asm("r13") = seed + 4;
    uint64_t var5 = seed + 5;
    uint64_t var6 = seed + 6;
    uint64_t var7 = seed + 7;
    uint64_t var8 = seed + 8;
    uint64_t var9 = seed + 9;
    uint64_t var10 = seed + 10;
    
    uint64_t sum = 0;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers many registers */
        uint64_t tmp1 = helper1(var1, var2);
        
        /* Critical instruction that should be at the end of a basic block */
        /* This instruction uses variables that need to be preserved across calls */
        var5 = var6 + var7;  /* This could be moved by caller-save */
        
        /* Second call - clobbers different registers */
        uint64_t tmp2 = helper2(var3, var4);
        
        /* More operations to create live ranges */
        var8 = helper3(var8);
        var9 = helper4(var9, var10, tmp1);
        
        /* Use all variables to keep them live */
        sum += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10 + tmp1 + tmp2;
        
        /* Modify variables for next iteration */
        var1++;
        var2 += tmp1;
        var3 ^= tmp2;
    }
    
    return sum;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) uint64_t test2(uint64_t seed) {
    uint64_t a = seed * 2;
    uint64_t b = seed * 3;
    uint64_t c = seed * 4;
    uint64_t d = seed * 5;
    uint64_t e = seed * 6;
    uint64_t f = seed * 7;
    uint64_t g = seed * 8;
    uint64_t h = seed * 9;
    
    uint64_t result = 0;
    
    for (int i = 0; i < 4; i++) {
        /* Force specific registers to be used */
        register uint64_t r11_val asm("r11") = a + b;
        register uint64_t r12_val asm("r12") = c + d;
        
        /* Call that clobbers registers */
        uint64_t tmp = helper3(r11_val);
        
        /* Instruction that could be at block end and need moving */
        /* This uses r12_val which must be preserved across helper4 call */
        e = r12_val + f;  /* Potential candidate for movement */
        
        /* Another call with explicit clobber list */
        asm volatile ("call helper4" : "=a"(tmp) : "a"(tmp), "b"(e), "c"(g), "d"(h) : "memory", "r11", "r12", "r13", "r14", "r15");
        
        /* Use the result */
        result += tmp + e + r11_val + r12_val;
        
        /* Update variables */
        a ^= b;
        b += c;
        c = helper1(c, d);
        d = helper2(d, e);
    }
    
    return result;
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) uint64_t test3(uint64_t seed) {
    uint64_t array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = seed + i;
    }
    
    uint64_t *ptr = array;
    uint64_t sum = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Load from pointer - creates complex addressing */
        uint64_t val1 = *ptr;
        uint64_t val2 = *(ptr + 1);
        
        /* Call that clobbers registers */
        uint64_t tmp1 = helper1(val1, val2);
        
        /* Pointer arithmetic that could be at block end */
        ptr = ptr + 2;  /* This instruction might need moving */
        
        /* Another call */
        uint64_t tmp2 = helper2(tmp1, *ptr);
        
        /* More operations */
        sum += tmp1 + tmp2 + val1 + val2;
        
        /* Conditional to create basic block boundaries */
        if (ptr >= array + 6) {
            ptr = array;
        }
    }
    
    return sum;
}

/* Main function that drives all tests */
int main() {
    volatile uint64_t seed = 12345;
    uint64_t total = 0;
    
    /* Run tests multiple times to increase coverage chances */
    for (int iter = 0; iter < 10; iter++) {
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
