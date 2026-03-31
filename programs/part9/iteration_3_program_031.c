/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller_save_test.c -o caller_save_test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to prevent dead code elimination */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) uint64_t helper1(uint64_t a, uint64_t b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) uint64_t helper2(uint64_t a, uint64_t b) {
    return a * b + 2;
}

__attribute__((noinline, noipa)) uint64_t helper3(uint64_t a, uint64_t b) {
    return (a ^ b) + 3;
}

__attribute__((noinline, noipa)) uint64_t helper4(uint64_t a, uint64_t b) {
    return (a << 2) | (b >> 2) + 4;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline)) void test1(uint64_t seed) {
    /* Many local variables to pressure registers */
    register uint64_t v1 asm("r11") = seed + 1;
    register uint64_t v2 asm("r12") = seed + 2;
    uint64_t v3 = seed + 3;
    uint64_t v4 = seed + 4;
    uint64_t v5 = seed + 5;
    uint64_t v6 = seed + 6;
    uint64_t v7 = seed + 7;
    uint64_t v8 = seed + 8;
    uint64_t v9 = seed + 9;
    uint64_t v10 = seed + 10;
    
    for (int i = 0; i < 3; i++) {
        /* First call - clobbers call-used registers */
        v1 = helper1(v1, v2);
        
        /* Instruction that might need to be moved - uses register that needs spilling */
        v3 = v4 + v5;  /* This could be the last instruction in basic block */
        
        /* Second call - forces save/restore around it */
        v2 = helper2(v3, v6);
        
        /* More computations to create live ranges across calls */
        v4 = v7 + v8;
        v5 = helper3(v9, v10);
        
        /* Update loop variable in a way that creates basic block boundaries */
        if (i < 2) {
            v6 = v1 + v2;  /* Another candidate for movement */
            v7 = helper4(v3, v4);
        }
        
        /* Accumulate to global to prevent elimination */
        global_acc += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    }
}

/* Test function 2: Explicit register clobbering with inline asm */
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
        /* Use inline asm to explicitly clobber specific registers */
        asm volatile("" : : : "r11", "r12", "r13", "r14", "r15");
        
        /* Call that forces register saves */
        a = helper1(b, c);
        
        /* Critical instruction - potentially the last in basic block */
        d = e + f;
        
        /* Another call */
        b = helper2(c, d);
        
        /* More asm clobbering */
        asm volatile("" : : : "rax", "rcx", "rdx", "rsi", "rdi");
        
        /* Instruction that might get moved */
        g = h + a;
        
        /* Final call in sequence */
        c = helper3(d, e);
        
        /* Update variables to maintain live ranges */
        e = f + g;
        f = helper4(g, h);
        
        global_acc += a + b + c + d + e + f + g + h;
    }
}

/* Test function 3: Mixed pointer and scalar operations */
__attribute__((noinline)) void test3(uint64_t seed) {
    uint64_t arr[8] = {seed, seed+1, seed+2, seed+3, seed+4, seed+5, seed+6, seed+7};
    uint64_t *ptr = arr;
    uint64_t sum = 0;
    
    for (int i = 0; i < 5; i++) {
        /* Load from pointer - creates register pressure */
        uint64_t x1 = *ptr++;
        uint64_t x2 = *ptr++;
        
        /* Call that clobbers registers */
        uint64_t t1 = helper1(x1, x2);
        
        /* Store operation - candidate for movement */
        arr[i % 8] = t1 + i;
        
        /* Another call */
        uint64_t t2 = helper2(t1, x2);
        
        /* Pointer arithmetic that might be last in block */
        ptr = &arr[(i + 1) % 8];
        
        /* More calls */
        uint64_t t3 = helper3(t2, x1);
        uint64_t t4 = helper4(t3, t2);
        
        /* Update and accumulate */
        sum += t1 + t2 + t3 + t4;
        
        /* Conditional to create basic block boundaries */
        if (i % 2 == 0) {
            /* Instruction that could be moved to end of block */
            arr[(i + 2) % 8] = sum;
        }
    }
    
    global_acc += sum;
}

/* Test function 4: Nested loops with calls at different levels */
__attribute__((noinline)) void test4(uint64_t seed) {
    uint64_t a = seed, b = seed + 100, c = seed + 200;
    
    for (int i = 0; i < 3; i++) {
        uint64_t local1 = a + i;
        uint64_t local2 = b + i * 2;
        
        for (int j = 0; j < 2; j++) {
            /* Call in inner loop */
            uint64_t tmp = helper1(local1, local2);
            
            /* Instruction that might be last in basic block when j == 1 */
            local1 = local2 + tmp;
            
            /* Another call */
            local2 = helper2(tmp, local1);
            
            /* Conditional that creates block end */
            if (j == 0) {
                /* This creates a basic block ending with an arithmetic instruction */
                c = local1 + local2;
            } else {
                /* Different block ending with different instruction */
                c = helper3(local1, local2);
            }
        }
        
        /* Final computation that could be moved */
        a = b + c;
        
        /* Call that forces spills */
        b = helper4(a, c);
        
        global_acc += a + b + c + local1 + local2;
    }
}

int main() {
    volatile uint64_t seed = 12345;
    
    /* Run tests multiple times to increase coverage chances */
    for (int iter = 0; iter < 10; iter++) {
        test1(seed + iter * 100);
        test2(seed + iter * 200);
        test3(seed + iter * 300);
        test4(seed + iter * 400);
        
        /* Modify seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
