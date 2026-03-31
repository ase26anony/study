/* caller-save-test.c */
#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile int64_t global_acc = 0;

/* Non-inlineable helper functions that clobber registers */
__attribute__((noinline, noipa)) int64_t helper1(int64_t a, int64_t b) {
    /* Use inline asm to ensure register clobbering */
    asm volatile("" : "+r"(a), "+r"(b) : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11");
    return a + b + 1;
}

__attribute__((noinline, noipa)) int64_t helper2(int64_t a, int64_t b) {
    asm volatile("" : "+r"(a), "+r"(b) : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12");
    return a * b - 2;
}

__attribute__((noinline, noipa)) int64_t helper3(int64_t a, int64_t b, int64_t c) {
    asm volatile("" : "+r"(a), "+r"(b), "+r"(c) : : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10");
    return (a + b) * c;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(int64_t seed) {
    /* Declare many local variables to create register pressure */
    register int64_t v1 asm("r11") = seed + 1;
    register int64_t v2 asm("r12") = seed + 2;
    int64_t v3 = seed + 3;
    int64_t v4 = seed + 4;
    int64_t v5 = seed + 5;
    int64_t v6 = seed + 6;
    int64_t v7 = seed + 7;
    int64_t v8 = seed + 8;
    int64_t v9 = seed + 9;
    int64_t v10 = seed + 10;
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers many registers */
        v1 = helper1(v1, v2);
        
        /* Critical instruction: This should be the last in basic block
           and may need to be moved by caller-save */
        v3 = v4 + v5;  /* This instruction might become BB_END */
        
        /* Second call - forces spill/restore around v3 assignment */
        v2 = helper2(v3, v6);
        
        /* Use results to prevent elimination */
        v7 = v1 + v2;
        v8 = v3 * v4;
        v9 = v5 - v6;
        v10 = v7 + v8 + v9;
        
        /* Update accumulator */
        global_acc += v10;
    }
}

/* Test function 2: Explicit register clobbering with asm */
__attribute__((noinline)) void test2(int64_t seed) {
    int64_t a = seed * 2;
    int64_t b = seed * 3;
    int64_t c = seed * 4;
    int64_t d = seed * 5;
    int64_t e = seed * 6;
    int64_t f = seed * 7;
    int64_t g = seed * 8;
    int64_t h = seed * 9;
    
    for (int i = 0; i < 4; i++) {
        /* Inline asm that clobbers specific call-used registers */
        asm volatile(
            "mov %1, %%r11\n\t"
            "mov %2, %%r12\n\t"
            "add %%r11, %%r12\n\t"
            : "=r"(a)
            : "r"(b), "r"(c)
            : "r11", "r12", "cc"
        );
        
        /* Critical instruction - may become BB_END */
        d = e + f;
        
        /* Call that forces save/restore */
        g = helper3(a, d, h);
        
        /* Complex live range spanning the call */
        b = c + d;
        c = e + f;
        e = g * h;
        
        global_acc += a + b + c + d + e + f + g + h;
    }
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) void test3(int64_t seed) {
    int64_t arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = seed + i;
    }
    
    int64_t *ptr = arr;
    int64_t sum = 0;
    int64_t tmp1 = seed;
    int64_t tmp2 = seed * 2;
    int64_t tmp3 = seed * 3;
    int64_t tmp4 = seed * 4;
    
    for (int i = 0; i < 5; i++) {
        /* Dereference pointer - creates complex addressing */
        tmp1 = *ptr + *(ptr + 1);
        
        /* Call that clobbers registers */
        tmp2 = helper1(tmp1, tmp3);
        
        /* Critical instruction - pointer update that might be BB_END */
        ptr = arr + (i % 7);
        
        /* Another call */
        tmp3 = helper2(tmp2, tmp4);
        
        /* Use results */
        tmp4 = tmp1 + tmp2 + tmp3;
        sum += tmp4;
        
        /* Ensure pointer is live across calls */
        asm volatile("" : "+r"(ptr));
    }
    
    global_acc += sum;
}

/* Test function 4: Nested loops with varying pressure */
__attribute__((noinline)) void test4(int64_t seed) {
    int64_t a = seed, b = seed + 1, c = seed + 2;
    int64_t d = seed + 3, e = seed + 4, f = seed + 5;
    int64_t g = seed + 6, h = seed + 7, j = seed + 8;
    
    for (int outer = 0; outer < 2; outer++) {
        /* Basic block with multiple instructions before call */
        a = b + c;
        b = c + d;
        
        /* Call that forces spills */
        c = helper1(a, b);
        
        /* Critical instruction sequence */
        d = e + f;  /* This might be moved by caller-save */
        
        /* Conditional to create basic block boundaries */
        if (d > 100) {
            /* Another call in a different block */
            e = helper2(c, d);
            
            /* Instruction that might need moving */
            f = g + h;
        } else {
            e = helper3(c, d, j);
            f = h + j;
        }
        
        /* Use all variables */
        g = a + b + c + d + e + f;
        h = g * j;
        j = h - seed;
        
        global_acc += g + h + j;
    }
}

int main() {
    volatile int64_t seed = 12345;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test1(seed + i * 100);
        test2(seed + i * 200);
        test3(seed + i * 300);
        test4(seed + i * 400);
        
        /* Modify seed to prevent constant propagation */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %ld\n", (long)global_acc);
    return 0;
}
