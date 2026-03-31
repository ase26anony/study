/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) 
uint64_t helper1(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    /* Use inline asm to ensure register clobbering */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d) : : "memory");
    return a + b + c + d + 1;
}

__attribute__((noinline, noipa))
uint64_t helper2(uint64_t a, uint64_t b, uint64_t c, uint64_t d, uint64_t e) {
    /* Clobber additional call-used registers */
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c), "+r"(d), "+r"(e) 
                 : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
    return a + b + c + d + e + 2;
}

__attribute__((noinline, noipa))
uint64_t helper3(uint64_t* ptr, uint64_t val) {
    /* Memory operation that might require register spilling */
    uint64_t old = *ptr;
    *ptr = val;
    asm volatile("" : : "r"(old), "r"(val) : "memory");
    return old + val;
}

/* Test 1: High register pressure with consecutive calls */
__attribute__((noinline, noipa))
void test1(uint64_t seed) {
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
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers many registers */
        uint64_t t1 = helper1(v1, v2, v3, v4);
        
        /* Critical instruction: This should be the last in basic block
           and may need to be moved by caller-save */
        v5 = v6 + v7 + i;  /* This instruction might become BB_END */
        
        /* Second call - more register pressure */
        uint64_t t2 = helper2(v5, v8, v9, v10, t1);
        
        /* Use results to prevent elimination */
        global_acc += t1 + t2 + v5;
        
        /* Modify variables to create live ranges across calls */
        v1 += t1;
        v2 += t2;
        v3 += i;
    }
}

/* Test 2: Explicit register clobbering with asm */
__attribute__((noinline, noipa))
void test2(uint64_t seed) {
    uint64_t a = seed * 3;
    uint64_t b = seed * 5;
    uint64_t c = seed * 7;
    uint64_t d = seed * 11;
    uint64_t e = seed * 13;
    uint64_t f = seed * 17;
    uint64_t g = seed * 19;
    
    /* Loop with multiple basic blocks */
    for (int j = 0; j < 4; j++) {
        /* First basic block ends with a call */
        uint64_t r1 = helper1(a, b, c, d);
        
        /* This store operation should be at the end of a basic block */
        uint64_t temp = e + f + g + j;
        
        /* Inline asm that explicitly uses/clobbers registers */
        asm volatile(
            "addq %%r10, %%r11\n\t"
            "movq %%r12, %%r13\n\t"
            : 
            : "r"(temp), "r"(r1)
            : "r10", "r11", "r12", "r13", "memory"
        );
        
        /* Another call that requires saving registers */
        uint64_t r2 = helper2(temp, r1, a, b, c);
        
        /* Critical: instruction at potential BB_END */
        g = f + e + j;  /* This might be moved by caller-save */
        
        global_acc += r1 + r2 + g;
        
        /* Rotate values to create different live ranges */
        uint64_t tmp = a;
        a = b; b = c; c = d; d = e; e = f; f = g; g = tmp + j;
    }
}

/* Test 3: Pointer operations creating complex live ranges */
__attribute__((noinline, noipa))
void test3(uint64_t seed) {
    uint64_t array[8];
    for (int i = 0; i < 8; i++) {
        array[i] = seed + i * 100;
    }
    
    uint64_t* ptr = &array[0];
    uint64_t sum = 0;
    
    /* Multiple iterations to create pressure */
    for (int k = 0; k < 5; k++) {
        /* Dereference and use pointer - creates register pressure */
        uint64_t val1 = *ptr;
        uint64_t val2 = *(ptr + 1);
        
        /* Call that might require spilling pointer register */
        uint64_t r1 = helper3(ptr, val1 + val2);
        
        /* Critical instruction: pointer arithmetic at BB_END */
        ptr = ptr + 1;  /* This could be the last instruction before BB boundary */
        
        if (ptr >= &array[7]) {
            ptr = &array[0];
        }
        
        /* Another call */
        uint64_t r2 = helper1(val1, val2, r1, k);
        
        global_acc += r1 + r2 + (uint64_t)ptr;
        
        /* Complex computation spanning calls */
        for (int m = 0; m < 2; m++) {
            array[m] = array[m] + r1 + r2 + m;
        }
    }
}

/* Test 4: Mixed types and nested loops */
__attribute__((noinline, noipa))
void test4(uint64_t seed) {
    int32_t a = seed;
    int64_t b = seed * 2;
    uint32_t c = seed * 3;
    uint64_t d = seed * 4;
    
    /* Nested loops create more basic blocks */
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            /* Multiple calls in sequence */
            uint64_t r1 = helper1(a, b, c, d);
            
            /* Instruction that should be at BB_END */
            int32_t temp = a + b + i + j;
            
            uint64_t r2 = helper2(temp, r1, c, d, j);
            
            /* Another potential BB_END instruction */
            d = c + b + i;
            
            global_acc += r1 + r2 + temp + d;
            
            /* Update variables for next iteration */
            a += r1;
            b += r2;
            c += i;
        }
        
        /* Additional basic block with different structure */
        if (i == 0) {
            uint64_t r3 = helper3(&d, a + b);
            global_acc += r3;
        }
    }
}

int main() {
    volatile uint64_t seed = 12345;
    
    /* Call test functions multiple times with different seeds */
    for (int iter = 0; iter < 10; iter++) {
        uint64_t s = seed + iter * 1000;
        
        test1(s);
        test2(s + 1);
        test3(s + 2);
        test4(s + 3);
        
        /* Modify seed to prevent constant propagation */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
