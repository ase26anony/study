/* caller-save-test.c */
/* Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test */

#include <stdio.h>
#include <stdint.h>

volatile int global_seed = 12345;
int global_accumulator = 0;

/* Non-inline helper functions that clobber registers */
__attribute__((noinline, noipa)) int helper1(int a, int b) {
    return a + b + 1;
}

__attribute__((noinline, noipa)) int helper2(int a, int b) {
    return a * b - 1;
}

__attribute__((noinline, noipa)) int helper3(int a, int b) {
    return (a << 3) | (b & 0x7);
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a ^ b;
}

__attribute__((noinline, noipa)) long helper5(long a, long b) {
    return a + (b >> 2);
}

/* Test function 1: High integer register pressure */
void test1(int seed) {
    int var1 = seed + 1;
    int var2 = seed + 2;
    int var3 = seed + 3;
    int var4 = seed + 4;
    int var5 = seed + 5;
    int var6 = seed + 6;
    int var7 = seed + 7;
    int var8 = seed + 8;
    int var9 = seed + 9;
    int var10 = seed + 10;
    
    for (int i = 0; i < 3; i++) {
        /* Use all variables to create register pressure */
        var1 = helper1(var1, var2);
        var2 = helper2(var2, var3);
        
        /* Critical instruction that should be at the end of a basic block */
        var3 = var4 + var5;  /* This instruction might need to be moved */
        
        var4 = helper3(var4, var5);
        var5 = helper1(var5, var6);
        
        /* Another critical instruction */
        var6 = var7 * var8;  /* Potential end-of-block instruction */
        
        var7 = helper2(var7, var8);
        var8 = helper3(var8, var9);
        
        /* Instruction that uses result and might be moved */
        var9 = var10 - var1;  /* Could be last in block */
        
        var10 = helper1(var10, var1);
    }
    
    /* Use results to prevent elimination */
    global_accumulator += var1 + var2 + var3 + var4 + var5 + 
                         var6 + var7 + var8 + var9 + var10;
}

/* Test function 2: Explicit register clobbering with inline asm */
void test2(int seed) {
    long var1 = seed * 2L;
    long var2 = seed * 3L;
    long var3 = seed * 4L;
    long var4 = seed * 5L;
    long var5 = seed * 6L;
    long var6 = seed * 7L;
    
    /* Use inline asm to force specific register usage */
    register long r11 asm("r11") = var1;
    register long r12 asm("r12") = var2;
    
    for (int i = 0; i < 4; i++) {
        /* Call that clobbers registers */
        var3 = helper4(var3, var4);
        
        /* Inline asm that uses and clobbers specific registers */
        asm volatile (
            "addq %1, %0\n\t"
            : "+r" (r11)
            : "r" (r12)
            : "cc"
        );
        
        /* Instruction that might be moved to end of block */
        var4 = var5 ^ var6;
        
        /* Another call */
        var5 = helper5(var5, var6);
        
        /* Another potential end-of-block instruction */
        var6 = r11 + r12;
        
        /* Update asm registers */
        r12 = var3;
    }
    
    global_accumulator += (int)(var3 + var4 + var5 + var6 + r11 + r12);
}

/* Test function 3: Mixed pointer and integer operations */
void test3(int seed) {
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = seed + i;
    }
    
    int *ptr1 = &data[0];
    int *ptr2 = &data[5];
    int var1 = seed;
    int var2 = seed * 2;
    int var3 = seed * 3;
    int var4 = seed * 4;
    
    for (int i = 0; i < 5; i++) {
        /* Dereference pointer before call */
        var1 = *ptr1 + var2;
        
        /* Call that might require saving ptr1 */
        var2 = helper1(var2, var3);
        
        /* Pointer arithmetic that could be at block end */
        ptr1++;  /* This could be the last instruction in a basic block */
        
        /* Another call */
        var3 = helper2(var3, var4);
        
        /* Mixed operation */
        var4 = *ptr2 - var1;
        
        /* Another pointer operation at potential block end */
        ptr2--;  /* Could be last instruction */
        
        /* Use results */
        *ptr1 = var1 + var2;
    }
    
    global_accumulator += var1 + var2 + var3 + var4 + *ptr1 + *ptr2;
}

/* Test function 4: Complex control flow within loop */
void test4(int seed) {
    int var1 = seed;
    int var2 = seed + 1;
    int var3 = seed + 2;
    int var4 = seed + 3;
    int var5 = seed + 4;
    int var6 = seed + 5;
    
    for (int i = 0; i < 3; i++) {
        /* Basic block starting with computations */
        var1 = var1 * 2;
        var2 = var2 + 3;
        
        /* Call that clobbers registers */
        var3 = helper3(var3, var4);
        
        /* Multiple instructions that could be rearranged */
        var4 = var5 + var6;  /* Candidate for movement */
        
        /* Another call */
        var5 = helper1(var5, var6);
        
        /* Instruction that might end up at block end */
        var6 = var1 - var2;  /* Potential last instruction */
        
        /* Small if-else to create more basic blocks */
        if (var6 > 0) {
            /* Call in one branch */
            var1 = helper2(var1, var2);
            /* Simple instruction that could be last */
            var2 = var3 * 2;
        } else {
            /* Call in other branch */
            var3 = helper3(var3, var4);
            /* Another potential end instruction */
            var4 = var5 / 2;
        }
    }
    
    global_accumulator += var1 + var2 + var3 + var4 + var5 + var6;
}

int main() {
    volatile int seed = global_seed;
    
    /* Call test functions multiple times to increase coverage chance */
    for (int i = 0; i < 10; i++) {
        test1(seed + i);
        test2(seed + i * 2);
        test3(seed + i * 3);
        test4(seed + i * 4);
        
        /* Modify seed to prevent constant propagation */
        seed += 7;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
