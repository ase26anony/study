/* test_caller_save.c - Program to trigger specific RTL list surgery in GCC's caller-save pass */

#include <stdio.h>
#include <stdint.h>

/* Global volatile flag to create unpredictable conditional */
volatile int global_volatile_flag = 1;

/* Callee function that clobbers caller-saved registers */
__attribute__((noipa, noinline, no_caller_saved_registers))
void callee_function(void)
{
    /* Use inline assembly to clobber caller-saved registers */
    /* This forces the caller to save/restore these registers */
#if defined(__x86_64__)
    asm volatile (
        "nop\n\t"
        : 
        : 
        : "rax", "rcx", "rdx", "rsi", "rdi", 
          "r8", "r9", "r10", "r11", 
          "xmm0", "xmm1", "xmm2", "xmm3",
          "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
#elif defined(__aarch64__)
    asm volatile (
        "nop\n\t"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
          "memory"
    );
#else
    /* Generic memory clobber for other architectures */
    asm volatile("" : : : "memory");
#endif
}

/* Caller function with high register pressure */
__attribute__((noipa, noinline, optimize("no-ipa-cp", "no-ipa-sra")))
int64_t caller_function(int64_t seed)
{
    /* Declare many local variables to create register pressure */
    register int64_t a asm("") = seed + 1;
    register int64_t b asm("") = seed * 2;
    register int64_t c asm("") = seed / 3;
    register int64_t d asm("") = seed - 4;
    register int64_t e asm("") = seed + 5;
    register int64_t f asm("") = seed * 6;
    register int64_t g asm("") = seed / 7;
    register int64_t h asm("") = seed - 8;
    register int64_t i asm("") = seed + 9;
    register int64_t j asm("") = seed * 10;
    register int64_t k asm("") = seed / 11;
    register int64_t l asm("") = seed - 12;
    
    /* Create data dependencies between variables */
    a = b * c + d;
    b = c * d - e;
    c = d * e + f;
    d = e * f - g;
    
    /* Memory barrier to ensure values are in registers */
    asm volatile("" : : : "memory");
    
    /* Volatile read to prevent optimization */
    volatile int local_flag = global_volatile_flag;
    
    /* Conditional call - creates basic block boundaries */
    if (local_flag) {
        /* Additional computation before call to create more register pressure */
        e = f * g + h;
        f = g * h - i;
        
        /* Call that clobbers caller-saved registers */
        callee_function();
        
        /* More computations after call, using same variables */
        g = h * i + j;
        h = i * j - k;
    } else {
        /* Alternative path with different computations */
        e = f * g - h;
        f = g * h + i;
        g = h * i - j;
        h = i * j + k;
    }
    
    /* Complex computation using all variables */
    /* This ensures they remain live across the call */
    i = j * k + l;
    j = k * l - a;
    k = l * a + b;
    l = a * b - c;
    
    /* Create a complex return value using all variables */
    /* This prevents dead code elimination */
    int64_t result = (a + b) * (c - d) + (e * f) - (g / h) + 
                     (i ^ j) | (k & l) + (a << 2) - (b >> 3);
    
    /* Another memory barrier */
    asm volatile("" : : : "memory");
    
    return result;
}

/* Helper function to create additional call context */
__attribute__((noinline))
int64_t helper_function(int64_t x, int64_t y)
{
    register int64_t t1 = x * y;
    register int64_t t2 = x + y;
    register int64_t t3 = x - y;
    register int64_t t4 = x ^ y;
    
    asm volatile("" : : : "memory");
    
    return t1 + t2 + t3 + t4;
}

int main(void)
{
    int64_t total = 0;
    
    /* Loop to create multiple call sites with different contexts */
    for (int iter = 0; iter < 100; iter++) {
        /* Vary the seed to create different register usage patterns */
        int64_t seed = iter * 12345 + 6789;
        
        /* Call helper to create additional register pressure context */
        int64_t helper_result = helper_function(seed, seed + 1);
        
        /* Main call that should trigger caller-save optimizations */
        int64_t result = caller_function(seed + helper_result);
        
        /* Use result to prevent elimination */
        total += result;
        
        /* Toggle flag occasionally to exercise both paths */
        if (iter % 7 == 0) {
            global_volatile_flag = !global_volatile_flag;
        }
    }
    
    printf("Total: %ld\n", (long)total);
    
    return 0;
}
