/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
 * Compile with: gcc -O2 -fno-optimize-sibling-calls -fno-ipa-ra caller-save-test.c -o caller-save-test
 */

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

__attribute__((noinline, noipa)) int helper3(int a, int b, int c) {
    return (a ^ b) | c;
}

__attribute__((noinline, noipa)) long helper4(long a, long b) {
    return a + (b << 2);
}

__attribute__((noinline, noipa)) int helper5(int a) {
    /* Use inline asm to explicitly clobber registers */
    int result;
    asm volatile ("movl %1, %0\n\t"
                  "addl $5, %0"
                  : "=r" (result)
                  : "r" (a)
                  : "cc");
    return result;
}

/* Test 1: High register pressure with int variables */
__attribute__((noinline)) void test1(int seed) {
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
    
    /* Loop to create basic blocks */
    for (int i = 0; i < 3; i++) {
        /* Multiple calls that clobber registers */
        int tmp1 = helper1(var1, var2);
        int tmp2 = helper2(var3, var4);
        
        /* Critical instruction: This should be at the end of a basic block
         * and might need to be moved by caller-save */
        var1 = var2 + var3;  /* This could be the last instruction before BB end */
        
        /* More calls to increase pressure */
        int tmp3 = helper3(var5, var6, var7);
        int tmp4 = helper1(var8, var9);
        
        /* Another candidate for movement */
        var2 = var3 * var4;
        
        /* Use results to prevent elimination */
        global_accumulator += tmp1 + tmp2 + tmp3 + tmp4;
        
        /* Shuffle variables to create complex live ranges */
        var3 = var4 ^ var5;
        var4 = var5 | var6;
        var5 = var6 & var7;
    }
    
    /* Final use */
    global_accumulator += var1 + var2;
}

/* Test 2: Long variables with explicit register clobbering */
__attribute__((noinline)) void test2(long seed) {
    register long r11_val asm ("r11") = seed;
    register long r12_val asm ("r12") = seed + 1;
    register long r13_val asm ("r13") = seed + 2;
    long var1 = seed + 3;
    long var2 = seed + 4;
    long var3 = seed + 5;
    long var4 = seed + 6;
    
    for (int i = 0; i < 4; i++) {
        /* Call that uses and clobbers registers */
        long tmp1 = helper4(r11_val, r12_val);
        
        /* Inline asm that explicitly clobbers call-used registers */
        asm volatile ("addq %1, %0\n\t"
                      "subq %2, %0"
                      : "+r" (r13_val)
                      : "r" (var1), "r" (var2)
                      : "cc", "r11", "r12");
        
        /* Critical instruction at potential BB end */
        var1 = var2 + r11_val;
        
        /* Another call */
        long tmp2 = helper4(var3, var4);
        
        /* Instruction that might need moving */
        var2 = r12_val * 3;
        
        global_accumulator += tmp1 + tmp2 + r13_val;
        
        /* Update for next iteration */
        r11_val += 1;
        r12_val += 2;
        var3 = var4 ^ seed;
        var4 = tmp1 + i;
    }
}

/* Test 3: Mixed scalar and pointer operations */
__attribute__((noinline)) void test3(int seed) {
    int data[8];
    for (int i = 0; i < 8; i++) {
        data[i] = seed + i;
    }
    
    int *ptr = data;
    int idx = 0;
    int sum = 0;
    
    /* Complex loop with multiple basic blocks */
    for (int i = 0; i < 5; i++) {
        /* First call */
        int tmp1 = helper5(ptr[0]);
        
        /* Pointer arithmetic that might be at BB end */
        ptr = &data[idx];  /* Could be last instruction in BB */
        
        /* Second call */
        int tmp2 = helper5(ptr[1]);
        
        /* Another candidate for movement */
        idx = (idx + 1) & 7;
        
        /* Third call */
        int tmp3 = helper5(ptr[2]);
        
        /* Store that might need moving relative to calls */
        data[idx] = tmp1 + tmp2;  /* Potential BB end instruction */
        
        sum += tmp3;
        
        /* More operations to create register pressure */
        asm volatile ("" : "+r" (ptr), "+r" (idx) : : "memory");
    }
    
    global_accumulator += sum;
}

/* Test 4: Nested loops with varying pressure */
__attribute__((noinline)) void test4(int seed) {
    int a = seed, b = seed + 1, c = seed + 2, d = seed + 3;
    int e = seed + 4, f = seed + 5, g = seed + 6, h = seed + 7;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 3; inner++) {
            /* Sequence of calls creating pressure */
            int t1 = helper1(a, b);
            int t2 = helper2(c, d);
            
            /* Instruction that could be at BB end before jump to loop header */
            a = b + c;  /* This might be last in BB before loop backedge */
            
            int t3 = helper3(e, f, g);
            int t4 = helper1(h, a);
            
            /* Another potential BB-end instruction */
            b = c * d;
            
            global_accumulator += t1 + t2 + t3 + t4;
            
            /* Complex updates */
            c = d ^ e;
            d = e | f;
            e = f & g;
            f = g + h;
            g = h * 2;
            h = t1 + inner;
        }
        
        /* Basic block ending with call + instruction */
        int t5 = helper2(a, b);
        c = d + e;  /* Could be moved after call by caller-save */
    }
}

int main() {
    volatile int seed = global_seed;
    
    /* Run tests multiple times to increase coverage chance */
    for (int iteration = 0; iteration < 10; iteration++) {
        test1(seed + iteration);
        test2(seed + iteration);
        test3(seed + iteration);
        test4(seed + iteration * 2);
        
        /* Modify seed to create different execution paths */
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
