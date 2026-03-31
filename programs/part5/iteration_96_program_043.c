/* Condition Code Coverage Test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of all x86 condition codes from floating-point comparisons */
#define FORCE_USED __attribute__((used))

/* Inline assembly helpers to force condition code printing */
static FORCE_USED void emit_condition_codes(void)
{
    int var = 0, src = 1;
    
    /* Direct inline assembly with %C constraint to trigger condition code printing */
    /* UNORDERED */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(16) : "cc");
    
    /* ORDERED */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(23) : "cc");
    
    /* UNEQ */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(24) : "cc");
    
    /* UNGE */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(25) : "cc");
    
    /* UNGT */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(26) : "cc");
    
    /* UNLE */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(27) : "cc");
    
    /* UNLT */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(28) : "cc");
    
    /* LTGT */
    asm volatile ("cmov%C0 %1, %0" : "+r"(var) : "r"(src), "i"(29) : "cc");
}

/* Function to create unordered floating-point scenarios */
static FORCE_USED int test_unordered_comparisons(void)
{
    volatile int cc_accumulator = 0;
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.7;
        
        /* Insert NaN at specific indices to create unordered comparisons */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_nan("0xdead");
            arr2[i] = __builtin_nan("0xbeef");
        }
    }
    
    /* Perform all floating-point comparisons to generate condition codes */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        volatile int result;
        
        /* Generate UNORDERED/ORDERED codes */
        result = !(a == a) || !(b == b);  /* Check for NaN */
        cc_accumulator += result ? 1 : 0;
        
        /* Generate UNEQ (unordered or equal) */
        result = !(a < b) && !(a > b);
        cc_accumulator += result ? 1 : 0;
        
        /* Generate UNGE (unordered or greater-or-equal) */
        result = !(a < b);
        cc_accumulator += result ? 1 : 0;
        
        /* Generate UNGT (unordered or greater) */
        result = !(a <= b);
        cc_accumulator += result ? 1 : 0;
        
        /* Generate UNLE (unordered or less-or-equal) */
        result = !(a > b);
        cc_accumulator += result ? 1 : 0;
        
        /* Generate UNLT (unordered or less) */
        result = !(a >= b);
        cc_accumulator += result ? 1 : 0;
        
        /* Generate LTGT (less or greater, but not equal and not unordered) */
        result = (a < b) || (a > b);
        cc_accumulator += result ? 1 : 0;
        
        /* Use ternary operators to force conditional move generation */
        int temp = (a < b) ? 1 : 
                   (a <= b) ? 2 : 
                   (a > b) ? 3 : 
                   (a >= b) ? 4 : 
                   (a == b) ? 5 : 
                   (a != b) ? 6 : 0;
        cc_accumulator += temp;
    }
    
    return cc_accumulator;
}

/* Additional test with mixed types */
static FORCE_USED int test_mixed_comparisons(void)
{
    volatile float f1 = __builtin_nanf("");
    volatile float f2 = 3.14f;
    volatile double d1 = __builtin_nan("");
    volatile double d2 = 2.71828;
    volatile long double ld1 = __builtin_nanl("");
    volatile long double ld2 = 1.618034L;
    
    int acc = 0;
    
    /* Mixed float/double comparisons */
    acc += (f1 < f2) ? 1 : 0;      /* UNORDERED */
    acc += (f1 <= f2) ? 1 : 0;     /* UNORDERED */
    acc += (f1 > f2) ? 1 : 0;      /* UNORDERED */
    acc += (f1 >= f2) ? 1 : 0;     /* UNORDERED */
    acc += (f1 == f2) ? 1 : 0;     /* UNORDERED */
    acc += (f1 != f2) ? 1 : 0;     /* ORDERED */
    
    acc += (d1 < d2) ? 1 : 0;      /* UNORDERED */
    acc += (d1 <= d2) ? 1 : 0;     /* UNORDERED */
    acc += (d1 > d2) ? 1 : 0;      /* UNORDERED */
    acc += (d1 >= d2) ? 1 : 0;     /* UNORDERED */
    acc += (d1 == d2) ? 1 : 0;     /* UNORDERED */
    acc += (d1 != d2) ? 1 : 0;     /* ORDERED */
    
    /* Force generation of specific condition codes via builtins */
    if (__builtin_isnan(f1)) acc++;
    if (__builtin_isnan(d1)) acc++;
    if (!__builtin_isnan(ld2)) acc++;
    
    return acc;
}

/* Main function with volatile results to prevent optimization */
int main(void)
{
    /* Call functions to ensure they're not eliminated */
    emit_condition_codes();
    
    volatile int result1 = test_unordered_comparisons();
    volatile int result2 = test_mixed_comparisons();
    
    /* Additional direct inline assembly with various condition codes */
    int x = 0, y = 42;
    
    /* UNORDERED */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(16) : "eax", "cc");
    
    /* ORDERED */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(23) : "eax", "cc");
    
    /* UNEQ */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(24) : "eax", "cc");
    
    /* UNGE */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(25) : "eax", "cc");
    
    /* UNGT */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(26) : "eax", "cc");
    
    /* UNLE */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(27) : "eax", "cc");
    
    /* UNLT */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(28) : "eax", "cc");
    
    /* LTGT */
    asm volatile (
        "test %%eax, %%eax\n\t"
        "cmov%C0 %1, %0"
        : "+r"(x) : "r"(y), "i"(29) : "eax", "cc");
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d %d %d\n", result1, result2, x);
    
    return 0;
}
