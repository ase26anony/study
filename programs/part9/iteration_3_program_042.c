/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile uint64_t global_acc = 0;

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

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a) {
    uint64_t result;
    asm volatile ("imulq $37, %1, %0" : "=r"(result) : "r"(a) : "cc");
    return result;
}

__attribute__((noinline, noipa)) uint64_t helper4(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t result;
    asm volatile ("xorq %1, %0\n\t"
                  "addq %2, %0" 
                  : "=r"(result) : "r"(a), "r"(b), "0"(c) : "cc");
    return result;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(uint64_t seed) {
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
        v1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last instruction in 
           its basic block before potential movement */
        v3 = v4 + v5;  /* This instruction may need to be moved */
        
        /* Second call - more register clobbering */
        v2 = helper2(v2, v3);
        
        /* Use results to prevent elimination */
        global_acc += v1 + v2 + v3;
        
        /* Modify variables to create live ranges across calls */
        v4 = helper3(v4);
        v5 = helper1(v5, v6);
        v6 = helper2(v6, v7);
        v7 = helper3(v7);
        
        /* More operations to increase pressure */
        v8 = v8 + v9 + v10;
        v9 = helper4(v9, v10, seed);
        v10 = v10 * 3;
    }
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) void test2(uint64_t seed) {
    uint64_t a = seed * 2;
    uint64_t b = seed * 3;
    uint64_t c = seed * 5;
    uint64_t d = seed * 7;
    uint64_t e = seed * 11;
    uint64_t f = seed * 13;
    uint64_t g = seed * 17;
    uint64_t h = seed * 19;
    
    /* Use inline asm to explicitly clobber call-used registers */
    for (int i = 0; i < 4; i++) {
        /* Call that clobbers specific registers */
        asm volatile ("movq %1, %%rax\n\t"
                      "movq %2, %%rcx\n\t"
                      "addq %%rcx, %%rax\n\t"
                      "movq %%rax, %0"
                      : "=r"(a) : "r"(b), "r"(c) : "rax", "rcx", "cc");
        
        /* Instruction that might be moved - placed right before 
           another call in the same basic block */
        d = e + f;  /* Potential candidate for movement */
        
        /* Another call */
        g = helper1(g, h);
        
        /* Instruction after call - creates need for save/restore */
        h = helper2(h, a);
        
        /* Complex expression that uses many variables */
        c = a + b + d + e + f + g + h;
        
        /* Update variables to maintain liveness */
        b = helper3(b);
        e = helper4(e, f, g);
        f = f * 2;
        
        global_acc += c;
    }
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) void test3(uint64_t seed) {
    uint64_t arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = seed + i;
    }
    
    uint64_t *ptr1 = &arr[0];
    uint64_t *ptr2 = &arr[4];
    uint64_t sum = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Dereference pointer before call */
        uint64_t val1 = *ptr1;
        uint64_t val2 = *ptr2;
        
        /* Call that clobbers registers */
        val1 = helper1(val1, val2);
        
        /* Critical store instruction - might be last in basic block */
        *ptr1 = val1 + 1;  /* This store could be moved */
        
        /* Another call */
        val2 = helper2(val2, *ptr2);
        
        /* Store result */
        *ptr2 = val2;
        
        /* Update pointers - creates complex addressing */
        ptr1++;
        if (ptr1 >= &arr[8]) ptr1 = &arr[0];
        
        /* Instruction that uses result */
        sum += *ptr1 + *ptr2;
        
        /* More calls to increase pressure */
        arr[2] = helper3(arr[2]);
        arr[3] = helper4(arr[3], arr[5], arr[7]);
    }
    
    global_acc += sum;
}

/* Test function 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(uint64_t seed) {
    uint64_t x1 = seed, x2 = seed + 1, x3 = seed + 2;
    uint64_t y1 = seed + 3, y2 = seed + 4, y3 = seed + 5;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            /* First call in inner loop */
            x1 = helper1(x1, x2);
            
            /* Arithmetic that might be moved */
            x3 = y1 + y2;  /* Potential movement candidate */
            
            /* Second call - creates basic block boundary */
            y1 = helper2(y1, x3);
            
            /* More operations */
            x2 = helper3(x2);
            y2 = helper4(y2, x1, x3);
            
            /* Update for next iteration */
            y3 = y3 * 2;
            
            global_acc += x1 + x2 + x3 + y1 + y2 + y3;
        }
        
        /* Additional calls in outer loop to create different patterns */
        x1 = helper1(x1, y3);
        y1 = helper2(y1, x2);
    }
}

int main() {
    volatile uint64_t seed = 12345;
    
    /* Call test functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        uint64_t current_seed = seed + i * 1000;
        
        test1(current_seed);
        test2(current_seed + 1);
        test3(current_seed + 2);
        test4(current_seed + 3);
        
        /* Modify seed to create varying behavior */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
