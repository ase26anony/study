/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns -fearly-remat -fno-inline -fno-tree-vectorize -fno-gcse -fno-strict-aliasing test.c -o test */
/* Also try: gcc -O3 -m32 -march=i686 -fno-dse -fearly-remat test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Non-inlineable function to create barriers */
__attribute__((noinline)) int external_func(int x) {
    volatile int dummy = x;
    return dummy + 1;
}

/* Function pointer with volatile to prevent optimization */
volatile void (*volatile_func_ptr)(void);

/* Stress function with complex control flow and register pressure */
__attribute__((noinline, optimize("no-gcse", "no-tree-vectorize")))
int stress_function(int seed) {
    /* Create many local variables to increase register pressure */
    volatile int v1 = seed;
    char c1 = seed & 0xFF;
    short s1 = seed & 0xFFFF;
    int i1 = seed;
    long long ll1 = (long long)seed * 1000;
    float f1 = seed * 0.5f;
    double d1 = seed * 0.25;
    int *ptr1 = &i1;
    
    int i2 = seed + 1;
    int i3 = seed + 2;
    int i4 = seed + 3;
    int i5 = seed + 4;
    int i6 = seed + 5;
    int i7 = seed + 6;
    int i8 = seed + 7;
    int i9 = seed + 8;
    int i10 = seed + 9;
    
    float f2 = f1 + 1.0f;
    float f3 = f1 + 2.0f;
    double d2 = d1 + 1.0;
    double d3 = d1 + 2.0;
    
    /* Key intermediate result - this is what we want to rematerialize */
    int key_result = 0;
    
    /* Complex computation with mixed types */
    key_result = v1 + c1 + s1 + i1 + (int)ll1;
    key_result += (int)f1 + (int)d1 + *ptr1;
    
    /* Store label addresses for goto */
    void* labels[] = { &&label1, &&label2, &&label3, &&label4 };
    volatile_func_ptr = labels[seed % 4];
    
    /* Deeply nested conditional blocks */
    if (v1 > 100) {
        if (c1 < 50) {
            switch (s1 % 4) {
                case 0:
                    /* Use key_result in conditional block with inline asm */
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        "mov r0, %0\n\t"
                        "mov r1, %0\n\t"
                        "mov r2, %0\n\t"
                        "mov r3, %0\n\t"
                        : "+r" (key_result)
                        :
                        : "r0", "r1", "r2", "r3", "memory"
                    );
                    
                    /* Call non-inlineable function */
                    key_result = external_func(key_result);
                    break;
                    
                case 1:
                    /* Different mode usage - short */
                    short temp_short = (short)key_result;
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        : "+r" (temp_short)
                        :
                        : "memory"
                    );
                    key_result = temp_short;
                    break;
                    
                case 2:
                    /* Different mode usage - char */
                    char temp_char = (char)key_result;
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        : "+r" (temp_char)
                        :
                        : "memory"
                    );
                    key_result = temp_char;
                    break;
                    
                case 3:
                    /* Different mode usage - long long */
                    long long temp_ll = (long long)key_result;
                    asm volatile (
                        "add %0, %0, #1\n\t"
                        : "+r" (temp_ll)
                        :
                        : "memory"
                    );
                    key_result = (int)temp_ll;
                    break;
            }
            
            /* More nested conditions */
            if (f1 > 0.0f) {
                volatile int volatile_cond = i2;
                if (volatile_cond) {
                    /* Use key_result again with different mode */
                    float temp_float = (float)key_result;
                    asm volatile (
                        "fadds %0, %0, %0\n\t"
                        : "+w" (temp_float)
                        :
                        : "memory"
                    );
                    key_result = (int)temp_float;
                }
            }
        } else {
            /* Another path using key_result */
            double temp_double = (double)key_result;
            asm volatile (
                "faddd %0, %0, %0\n\t"
                : "+w" (temp_double)
                :
                : "memory"
            );
            key_result = (int)temp_double;
        }
        
        /* Irreducible control flow using computed goto */
        goto *volatile_func_ptr;
        
    label1:
        key_result += i3;
        goto end_block;
        
    label2:
        key_result += i4;
        /* Fall through */
        
    label3:
        key_result += i5;
        goto end_block;
        
    label4:
        key_result += i6;
        /* Continue */
    }
    
end_block:
    
    /* Use key_result outside conditional blocks - this creates the live range
       that needs to be preserved across the conditional use */
    key_result = key_result * 2 + i7 - i8 + (int)f2 + (int)d2;
    
    /* More operations to keep variables live */
    i9 = key_result + i9;
    i10 = key_result - i10;
    
    /* Final computation using all variables to prevent elimination */
    int final_result = key_result + i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    final_result += (int)f1 + (int)f2 + (int)f3;
    final_result += (int)d1 + (int)d2 + (int)d3;
    final_result += c1 + s1 + (int)ll1;
    
    return final_result;
}

/* Another function to create cross-function pressure */
__attribute__((noinline))
int helper_func(int x, int y) {
    volatile int a = x;
    volatile int b = y;
    
    /* Inline asm that clobbers many registers */
    asm volatile (
        "mov r0, %0\n\t"
        "mov r1, %1\n\t"
        "add r0, r0, r1\n\t"
        "mov r2, r0\n\t"
        "mov r3, r0\n\t"
        "mov r4, r0\n\t"
        "mov r5, r0\n\t"
        "mov %0, r0\n\t"
        : "+r" (a)
        : "r" (b)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    
    return a;
}

int main() {
    int total = 0;
    
    /* Loop to create different execution paths */
    for (int i = 0; i < 100; i++) {
        /* Vary the seed to hit different conditional paths */
        int result = stress_function(i);
        
        /* Call helper to increase register pressure across calls */
        result = helper_func(result, i);
        
        total += result;
        
        /* Volatile store to prevent optimization */
        volatile int dummy = result;
    }
    
    printf("Result: %d\n", total);
    return total > 0 ? 0 : 1;
}
