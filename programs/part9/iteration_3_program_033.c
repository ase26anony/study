/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller_save_test.c -o caller_save_test */

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

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a, uint64_t b) {
    uint64_t result;
    asm volatile ("imulq %1, %0" : "=r"(result) : "r"(a), "0"(b) : "cc");
    return result;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(uint64_t seed) {
    /* Declare many local variables to create register pressure */
    register uint64_t v1 asm("r10") = seed + 1;
    register uint64_t v2 asm("r11") = seed + 2;
    register uint64_t v3 asm("r12") = seed + 3;
    register uint64_t v4 asm("r13") = seed + 4;
    uint64_t v5 = seed + 5;
    uint64_t v6 = seed + 6;
    uint64_t v7 = seed + 7;
    uint64_t v8 = seed + 8;
    uint64_t v9 = seed + 9;
    uint64_t v10 = seed + 10;
    
    /* Loop to create basic blocks with calls */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        v1 = helper1(v1, v2);
        
        /* Critical instruction that should be at the end of basic block */
        /* This is the instruction that might need to be moved */
        v3 = v4 + v5;  /* Simple arithmetic that could be last in BB */
        
        /* Second call - more register clobbering */
        v2 = helper2(v2, v3);
        
        /* Use the result to prevent elimination */
        global_acc += v1 + v2 + v3;
        
        /* Modify variables to create live ranges across calls */
        v4 = v5 + i;
        v5 = v6 - i;
        v6 = v7 * (i + 1);
    }
    
    /* Force use of all variables */
    global_acc += v4 + v5 + v6 + v7 + v8 + v9 + v10;
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) void test2(uint64_t seed) {
    uint64_t a = seed * 2;
    uint64_t b = seed * 3;
    uint64_t c = seed * 4;
    uint64_t d = seed * 5;
    uint64_t e = seed * 6;
    uint64_t f = seed * 7;
    uint64_t g = seed * 8;
    uint64_t h = seed * 9;
    
    for (int i = 0; i < 4; i++) {
        /* Call that clobbers specific registers */
        a = helper1(a, b);
        
        /* Instruction that might become BB_END */
        c = d + e;  /* This could be the last instruction before BB_END update */
        
        /* Inline asm that explicitly clobbers call-used registers */
        asm volatile ("" : "+r"(a), "+r"(b), "+r"(c) : : "r11", "r12", "r13", "cc");
        
        /* Another call */
        d = helper2(d, c);
        
        /* Critical: instruction after call that might need moving */
        e = f + g;  /* Potential candidate for BB_END movement */
        
        /* Third call */
        f = helper3(f, e);
        
        global_acc += a + b + c + d + e + f + g + h + i;
        
        /* Create complex live ranges */
        g = h + a;
        h = a * b;
    }
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) void test3(uint64_t seed) {
    uint64_t arr[8] = {seed, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7};
    uint64_t *ptr = arr;
    uint64_t sum = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Load from pointer - creates register pressure */
        uint64_t val1 = *ptr;
        ptr++;
        
        /* First call */
        val1 = helper1(val1, seed);
        
        /* Instruction that could be at BB end */
        uint64_t val2 = *(ptr + 1);  /* Load that might be moved */
        
        /* Second call */
        val2 = helper2(val2, i);
        
        /* Store that might need repositioning */
        *(ptr - 1) = val1 + val2;  /* This store could be BB_END candidate */
        
        /* Third call */
        sum = helper3(sum, val1);
        
        global_acc += sum + val2;
        
        /* Pointer arithmetic that creates live ranges */
        if (i & 1) {
            ptr = arr + (i % 4);
        }
    }
}

/* Test function 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(uint64_t seed) {
    uint64_t x1 = seed, x2 = seed * 2, x3 = seed * 3;
    uint64_t y1 = seed * 4, y2 = seed * 5, y3 = seed * 6;
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 3; j++) {
            /* Call in inner loop */
            x1 = helper1(x1, x2);
            
            /* Critical instruction - simple assignment that could be BB_END */
            x3 = y1 + y2;  /* This might be the last instruction in its BB */
            
            /* Conditional to create basic block boundaries */
            if (j & 1) {
                /* Another call in a different BB */
                y1 = helper2(y1, x3);
                
                /* Another potential BB_END instruction */
                y2 = x1 + x3;  /* Could be moved by caller-save */
            } else {
                y1 = helper3(y1, seed);
                y2 = x2 + x3;
            }
            
            global_acc += x1 + x2 + x3 + y1 + y2;
            
            /* Update variables for next iteration */
            x2 = x3 + j;
        }
        
        /* Call in outer loop */
        x1 = helper2(x1, x2);
        x3 = y1 + y2;  /* Another candidate for movement */
    }
}

int main() {
    volatile uint64_t seed = 12345;
    
    /* Call test functions multiple times with varying seeds */
    for (int iter = 0; iter < 10; iter++) {
        uint64_t current_seed = seed + iter * 1000;
        
        test1(current_seed);
        test2(current_seed + 1);
        test3(current_seed + 2);
        test4(current_seed + 3);
        
        /* Mix in some direct calls to increase pressure */
        for (int i = 0; i < 5; i++) {
            uint64_t tmp = helper1(current_seed, i);
            tmp = helper2(tmp, i + 1);
            tmp = helper3(tmp, i + 2);
            global_acc += tmp;
        }
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
