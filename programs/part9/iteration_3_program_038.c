/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

/* Global accumulator to ensure all computations are used */
volatile uint64_t global_acc = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) 
uint64_t helper1(uint64_t a, uint64_t b, uint64_t c) {
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

__attribute__((noinline, noipa))
uint64_t helper2(uint64_t a, uint64_t b) {
    /* Clobber multiple call-used registers */
    uint64_t result;
    asm volatile ("imul %1, %2\n\t"
                  "lea 1(%2), %0"
                  : "=r" (result)
                  : "r" (a), "r" (b)
                  : "rax", "rdx", "cc");
    return result;
}

__attribute__((noinline, noipa))
uint64_t helper3(uint64_t a, uint64_t b, uint64_t c, uint64_t d) {
    /* Complex operation clobbering many registers */
    uint64_t result;
    asm volatile ("mov %1, %%r10\n\t"
                  "mov %2, %%r11\n\t"
                  "xor %%r10, %%r11\n\t"
                  "add %3, %%r11\n\t"
                  "add %4, %%r11\n\t"
                  "mov %%r11, %0"
                  : "=r" (result)
                  : "r" (a), "r" (b), "r" (c), "r" (d)
                  : "r10", "r11", "cc");
    return result;
}

/* Test function 1: High register pressure with consecutive calls */
__attribute__((noinline))
void test1(uint64_t seed) {
    /* Declare many local variables to create register pressure */
    register uint64_t var1 asm("r12") = seed + 1;
    register uint64_t var2 asm("r13") = seed + 2;
    uint64_t var3 = seed + 3;
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
        
        /* Critical instruction that should be at the end of basic block */
        /* This instruction uses registers that might need spilling */
        uint64_t critical_op = var4 + var5 + i;
        
        /* Second call - clobbers more registers */
        uint64_t tmp2 = helper2(var6, var7);
        
        /* Use the results to prevent elimination */
        var8 = tmp1 + tmp2 + critical_op;
        var9 = helper3(var8, var9, var10, seed);
        
        /* Update variables to create live ranges across calls */
        var1 += var2;
        var2 += var3;
        var3 += var4;
        var4 = var5 + critical_op;  /* This creates a dependency */
        
        /* Force spill/restore around this call */
        var10 = helper1(var1, var2, var3);
    }
    
    /* Accumulate results */
    global_acc += var1 + var2 + var3 + var4 + var5 + var6 + var7 + var8 + var9 + var10;
}

/* Test function 2: Mix of pointer and scalar operations */
__attribute__((noinline))
void test2(uint64_t seed) {
    uint64_t data[8];
    uint64_t *ptr = data;
    uint64_t scalar1 = seed * 2;
    uint64_t scalar2 = seed * 3;
    uint64_t scalar3 = seed * 4;
    uint64_t scalar4 = seed * 5;
    
    /* Initialize array */
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i;
    }
    
    /* Loop with pointer arithmetic and calls */
    for (int j = 0; j < 4; j++) {
        /* Dereference pointer - creates memory operand */
        uint64_t val = *ptr;
        
        /* Call that clobbers registers */
        uint64_t res1 = helper2(val, scalar1);
        
        /* Pointer update that should be at block end */
        ptr = ptr + 1;
        
        /* Another call */
        uint64_t res2 = helper3(scalar2, scalar3, scalar4, res1);
        
        /* Update scalars */
        scalar1 = res1 + res2;
        scalar2 = helper1(scalar1, scalar2, scalar3);
        
        /* Store result back */
        if (j < 3) {
            *ptr = scalar1 + scalar2;
        }
    }
    
    global_acc += scalar1 + scalar2 + scalar3 + scalar4 + data[0] + data[7];
}

/* Test function 3: Complex control flow within basic block */
__attribute__((noinline))
void test3(uint64_t seed) {
    register uint64_t a asm("r14") = seed;
    register uint64_t b asm("r15") = seed * 2;
    uint64_t c = seed * 3;
    uint64_t d = seed * 4;
    uint64_t e = seed * 5;
    uint64_t f = seed * 6;
    
    /* Unrolled loop to create specific instruction sequences */
    for (int i = 0; i < 2; i++) {
        /* First part of basic block */
        uint64_t t1 = helper1(a, b, c);
        
        /* Middle computation - candidate for movement */
        uint64_t mid = d + e + f;
        
        /* Second call */
        uint64_t t2 = helper2(t1, mid);
        
        /* Conditional that creates block end */
        if (t2 > seed) {
            /* This assignment should be at block end */
            a = b + c;
            
            /* Call after block end adjustment */
            uint64_t t3 = helper3(a, b, c, d);
            b = t3 + mid;
        } else {
            d = e + f;
        }
        
        /* Update other variables */
        c = helper1(d, e, f);
        e = helper2(a, b);
        f = c + d + i;
    }
    
    global_acc += a + b + c + d + e + f;
}

/* Main driver */
int main() {
    volatile uint64_t seed = 12345;
    
    /* Call test functions multiple times with varying seeds */
    for (int iter = 0; iter < 10; iter++) {
        uint64_t current_seed = seed + iter * 1000;
        
        test1(current_seed);
        test2(current_seed + 1);
        test3(current_seed + 2);
        
        /* Modify seed to prevent constant propagation */
        seed = seed * 1103515245 + 12345;
    }
    
    printf("Result: %lu\n", (unsigned long)global_acc);
    return 0;
}
