/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are live */
volatile uint64_t global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
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

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a, uint64_t b) {
    uint64_t result;
    asm volatile ("imulq %1, %0" : "=r"(result) : "r"(a), "0"(b) : "cc");
    return result;
}

__attribute__((noinline, noipa)) uint64_t helper4(uint64_t a, uint64_t b) {
    uint64_t result;
    asm volatile ("xorq %1, %0" : "=r"(result) : "r"(a), "0"(b) : "cc");
    return result;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(uint64_t seed) {
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
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers registers */
        uint64_t tmp1 = helper1(var1, var2);
        
        /* Critical instruction that should be at the end of a basic block */
        /* This instruction uses variables that need to be preserved across calls */
        uint64_t critical_op = var3 + var4 + i;
        
        /* Second call - more register clobbering */
        uint64_t tmp2 = helper2(var5, var6);
        
        /* Use the result to prevent elimination */
        var1 = tmp1 + critical_op;
        var2 = tmp2 + var7;
        
        /* More operations to increase pressure */
        var3 = helper3(var8, var9);
        var4 = helper4(var10, var1);
        
        /* Update other variables */
        var5 = var5 + var6 + i;
        var6 = var6 * 2;
        var7 = var7 - 1;
        
        /* This store operation might become the last instruction in BB */
        var8 = var9 + var10;
    }
    
    /* Ensure all variables are used */
    global_accumulator += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
}

/* Test function 2: Mix of pointer and scalar operations */
__attribute__((noinline)) void test2(uint64_t seed) {
    uint64_t data[8];
    uint64_t *ptr = data;
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i;
    }
    
    /* Create register pressure with many local variables */
    uint64_t a = seed * 2;
    uint64_t b = seed * 3;
    uint64_t c = seed * 4;
    uint64_t d = seed * 5;
    uint64_t e = seed * 6;
    uint64_t f = seed * 7;
    uint64_t g = seed * 8;
    uint64_t h = seed * 9;
    
    /* Loop with pointer arithmetic and calls */
    for (int i = 0; i < 4; i++) {
        /* Dereference pointer - creates complex live range */
        uint64_t val = *ptr;
        
        /* Call that clobbers registers */
        uint64_t res1 = helper1(a, b);
        
        /* Pointer update - could be moved by caller-save */
        ptr++;
        
        /* Another call */
        uint64_t res2 = helper2(c, d);
        
        /* More operations */
        a = res1 + val;
        b = res2 + e;
        
        /* Additional calls to increase pressure */
        c = helper3(f, g);
        d = helper4(h, a);
        
        /* Update variables */
        e = e + f + i;
        f = f * 3;
        g = g - 2;
        
        /* This could be the last instruction before loop back-edge */
        h = *ptr + i;
    }
    
    /* Use results */
    global_accumulator += a + b + c + d + e + f + g + h + data[0];
}

/* Test function 3: Explicit register usage with asm clobbers */
__attribute__((noinline)) void test3(uint64_t seed) {
    /* Use explicit register variables for call-clobbered registers */
    register uint64_t r11_val asm("r11") = seed;
    register uint64_t r12_val asm("r12") = seed + 1;
    register uint64_t r13_val asm("r13") = seed + 2;
    register uint64_t r14_val asm("r14") = seed + 3;
    register uint64_t r15_val asm("r15") = seed + 4;
    
    uint64_t var1 = seed + 10;
    uint64_t var2 = seed + 20;
    uint64_t var3 = seed + 30;
    uint64_t var4 = seed + 40;
    uint64_t var5 = seed + 50;
    
    /* Loop with inline asm that clobbers specific registers */
    for (int i = 0; i < 3; i++) {
        /* Inline asm that uses and clobbers registers */
        asm volatile (
            "movq %1, %%r11\n\t"
            "movq %2, %%r12\n\t"
            "addq %%r12, %%r11\n\t"
            "movq %%r11, %0"
            : "=r"(var1)
            : "r"(r11_val), "r"(r12_val)
            : "r11", "r12", "cc"
        );
        
        /* Call that will need to save/restore r11, r12 */
        uint64_t tmp = helper1(var1, var2);
        
        /* Instruction that might be moved to after spill code */
        uint64_t update = r13_val + r14_val + i;
        
        /* Another call */
        uint64_t tmp2 = helper2(var3, var4);
        
        /* Update register variables */
        r11_val = tmp + update;
        r12_val = tmp2 + var5;
        
        /* More operations */
        r13_val = helper3(r14_val, r15_val);
        r14_val = helper4(var1, var2);
        
        /* This could be last instruction in BB */
        r15_val = r11_val + r12_val + i;
    }
    
    /* Use all values */
    global_accumulator += r11_val + r12_val + r13_val + r14_val + r15_val + 
                         var1 + var2 + var3 + var4 + var5;
}

/* Main function that drives the tests */
int main() {
    volatile uint64_t seed = 12345;
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 10; iteration++) {
        uint64_t current_seed = seed + iteration * 1000;
        
        test1(current_seed);
        test2(current_seed + 100);
        test3(current_seed + 200);
        
        /* Add some branching to create more complex CFG */
        if (iteration % 2 == 0) {
            test1(current_seed + 300);
        } else {
            test2(current_seed + 400);
        }
    }
    
    printf("Global accumulator: %lu\n", (unsigned long)global_accumulator);
    return 0;
}
