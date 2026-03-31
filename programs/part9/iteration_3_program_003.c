/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile uint64_t global_acc = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) uint64_t helper1(uint64_t a, uint64_t b) {
    /* Use inline asm to ensure register clobbering */
    uint64_t res;
    asm volatile ("addq %1, %0" : "=r"(res) : "r"(a), "0"(b) : "cc");
    return res;
}

__attribute__((noinline, noipa)) uint64_t helper2(uint64_t a, uint64_t b, uint64_t c) {
    uint64_t res;
    asm volatile ("imulq %1, %0" : "=r"(res) : "r"(a), "0"(b) : "cc");
    res += c;
    return res;
}

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a) {
    /* Clobber multiple call-used registers */
    uint64_t res;
    asm volatile ("movq %1, %%r11\n\t"
                  "addq $1, %%r11\n\t"
                  "movq %%r11, %0"
                  : "=r"(res) : "r"(a) : "r11", "cc");
    return res;
}

__attribute__((noinline, noipa)) uint64_t helper4(uint64_t a, uint64_t b) {
    uint64_t res;
    asm volatile ("xorq %1, %0" : "=r"(res) : "r"(a), "0"(b) : "cc");
    return res;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(uint64_t seed) {
    /* Declare many local variables to create register pressure */
    register uint64_t v1 asm("rax") = seed + 1;
    register uint64_t v2 asm("rbx") = seed + 2;
    register uint64_t v3 asm("rcx") = seed + 3;
    uint64_t v4 = seed + 4;
    uint64_t v5 = seed + 5;
    uint64_t v6 = seed + 6;
    uint64_t v7 = seed + 7;
    uint64_t v8 = seed + 8;
    uint64_t v9 = seed + 9;
    uint64_t v10 = seed + 10;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        v1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last in basic block
           and may need to be moved by caller-save */
        v3 = v4 + v5;  /* This instruction might be moved */
        
        /* Second call - more register clobbering */
        v6 = helper2(v6, v7, v8);
        
        /* Use results to prevent elimination */
        global_acc += v1 + v3 + v6;
        
        /* Modify variables to create live ranges across calls */
        v4 = v9 + v10;
        v5 = helper3(v5);
    }
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
    
    /* Use inline asm to suggest specific registers */
    register uint64_t r11_val asm("r11") = a;
    register uint64_t r12_val asm("r12") = b;
    
    for (int i = 0; i < 4; i++) {
        /* Call that clobbers specific registers */
        asm volatile ("" : "+r"(r11_val), "+r"(r12_val) : : "r11", "r12", "cc");
        
        /* Instruction that might be moved to end of block */
        c = d + e;  /* Potential candidate for movement */
        
        /* Another call */
        f = helper4(f, g);
        
        /* Complex expression creating register pressure */
        h = (r11_val * r12_val) + (c * f) - (d ^ e);
        
        global_acc += h;
        
        /* Update variables for next iteration */
        d = helper1(d, 1);
        e = helper2(e, 2, 3);
    }
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) void test3(uint64_t seed) {
    uint64_t array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = seed + i;
    }
    
    uint64_t *ptr = array;
    uint64_t sum = 0;
    uint64_t tmp1 = seed;
    uint64_t tmp2 = seed * 2;
    uint64_t tmp3 = seed * 3;
    
    for (int i = 0; i < 5; i++) {
        /* Dereference pointer - creates memory pressure */
        uint64_t val = *ptr;
        
        /* Call that might require saving registers */
        tmp1 = helper3(tmp1);
        
        /* Critical instruction at potential block end */
        tmp2 = tmp3 + val;  /* This could be moved */
        
        /* Another call */
        tmp3 = helper4(tmp2, tmp1);
        
        /* Pointer arithmetic that must stay in register */
        ptr++;
        
        /* Use results */
        sum += tmp1 + tmp2 + tmp3;
        
        /* Conditional to create basic block boundaries */
        if (ptr >= &array[7]) {
            ptr = array;
        }
    }
    
    global_acc += sum;
}

/* Test function 4: Nested loops with calls */
__attribute__((noinline)) void test4(uint64_t seed) {
    uint64_t a = seed;
    uint64_t b = seed + 1;
    uint64_t c = seed + 2;
    uint64_t d = seed + 3;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 3; inner++) {
            /* First call in basic block */
            a = helper1(a, b);
            
            /* Instruction that should be at block end */
            uint64_t critical = c + d;  /* May be moved by caller-save */
            
            /* Second call */
            b = helper2(b, critical, a);
            
            /* Third call - high register pressure */
            c = helper3(c);
            
            /* Use results */
            d = helper4(d, critical);
            
            global_acc += a + b + c + d;
            
            /* Update for next iteration */
            c = a ^ b;
        }
        
        /* Reset some values */
        a = seed + outer;
        d = helper1(d, 1);
    }
}

int main() {
    volatile uint64_t seed = 12345;
    
    /* Call test functions multiple times with different seeds */
    for (int i = 0; i < 10; i++) {
        test1(seed + i * 100);
        test2(seed + i * 200);
        test3(seed + i * 300);
        test4(seed + i * 400);
        
        /* Modify seed to prevent constant propagation */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
