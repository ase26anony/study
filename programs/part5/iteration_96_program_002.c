/* Condition Code Test Program for i386.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <math.h>

/* Force generation of specific condition codes */
#define FORCE_CC_PRINTING(cond, a, b) do { \
    int result; \
    __asm__ volatile ( \
        "# CC Test: " #cond "\n\t" \
        "fcomip %%st(1), %%st(0)\n\t" \
        "cmov%C0 %2, %0\n\t" \
        : "=r"(result) \
        : "u"(a), "r"(1), "i"(cond) \
        : "cc", "st", "st(1)" \
    ); \
    cc_accumulator += result; \
} while(0)

/* Alternative using builtin for constant propagation */
#define FORCE_CC_CONST(cond) do { \
    if (__builtin_constant_p(cond)) { \
        int dummy; \
        __asm__ volatile ( \
            "testl $0, %%eax\n\t" \
            "cmov%C0 %%eax, %0" \
            : "=r"(dummy) \
            : "i"(cond), "a"(0) \
            : "cc" \
        ); \
    } \
} while(0)

volatile int cc_accumulator = 0;

int main(void) {
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays with mix of normal values and NaN */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.25;
        
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
    
    /* Force generation of all condition codes through comparisons */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        int cmp_result;
        
        /* Generate UNORDERED/ORDERED codes */
        if (!(a == a) || !(b == b)) {  /* Check for NaN */
            cc_accumulator += 1;
        }
        
        /* Generate UNEQ (unordered or equal) */
        cmp_result = (a == b) ? 1 : 0;
        cc_accumulator += cmp_result;
        
        /* Generate UNGE (not less than) */
        cmp_result = !(a < b) ? 1 : 0;
        cc_accumulator += cmp_result;
        
        /* Generate UNGT (not less than or equal) */
        cmp_result = !(a <= b) ? 1 : 0;
        cc_accumulator += cmp_result;
        
        /* Generate UNLE (unordered or less than or equal) */
        cmp_result = (a <= b) ? 1 : 0;
        cc_accumulator += cmp_result;
        
        /* Generate UNLT (unordered or less than) */
        cmp_result = (a < b) ? 1 : 0;
        cc_accumulator += cmp_result;
        
        /* Generate LTGT (less than or greater than) */
        cmp_result = (a < b || a > b) ? 1 : 0;
        cc_accumulator += cmp_result;
        
        /* Force conditional move generation with ternary */
        int target = 0;
        target = (a < b) ? (target + 1) : (target - 1);
        target = (a > b) ? (target * 2) : (target / 2);
        target = (a <= b) ? (target | 0xFF) : (target & 0x0F);
        target = (a >= b) ? (target ^ 0xAA) : (target ^ 0x55);
        cc_accumulator += target & 1;
    }
    
    /* Direct inline assembly to force condition code printing */
    /* These will generate the specific strings when compiled with -dp/-dP */
    
    /* UNORDERED */
    __asm__ volatile (
        "# UNORDERED test\n\t"
        "fldz\n\t"
        "fldz\n\t"
        "fcomip %%st(1), %%st(0)\n\t"
        "cmov%c0 %%eax, %%ebx"
        : 
        : "i"(UNORDERED), "a"(0), "b"(0)
        : "cc", "st", "st(1)"
    );
    
    /* ORDERED */
    __asm__ volatile (
        "# ORDERED test\n\t"
        "fld1\n\t"
        "fld1\n\t"
        "fcomip %%st(1), %%st(0)\n\t"
        "cmov%c0 %%eax, %%ebx"
        : 
        : "i"(ORDERED), "a"(1), "b"(1)
        : "cc", "st", "st(1)"
    );
    
    /* UNEQ */
    __asm__ volatile (
        "# UNEQ test\n\t"
        "cmov%c0 %%ecx, %%edx"
        : 
        : "i"(UNEQ), "c"(2), "d"(2)
        : "cc"
    );
    
    /* UNGE */
    __asm__ volatile (
        "# UNGE test\n\t"
        "cmov%c0 %%esi, %%edi"
        : 
        : "i"(UNGE), "S"(3), "D"(3)
        : "cc"
    );
    
    /* UNGT */
    __asm__ volatile (
        "# UNGT test\n\t"
        "cmov%c0 %%r8d, %%r9d"
        : 
        : "i"(UNGT), "r"(4), "r"(4)
        : "cc"
    );
    
    /* UNLE */
    __asm__ volatile (
        "# UNLE test\n\t"
        "cmov%c0 %%r10d, %%r11d"
        : 
        : "i"(UNLE), "r"(5), "r"(5)
        : "cc"
    );
    
    /* UNLT */
    __asm__ volatile (
        "# UNLT test\n\t"
        "cmov%c0 %%r12d, %%r13d"
        : 
        : "i"(UNLT), "r"(6), "r"(6)
        : "cc"
    );
    
    /* LTGT */
    __asm__ volatile (
        "# LTGT test\n\t"
        "cmov%c0 %%r14d, %%r15d"
        : 
        : "i"(LTGT), "r"(7), "r"(7)
        : "cc"
    );
    
    /* Additional floating-point comparisons to ensure RTL generation */
    volatile double nan1 = __builtin_nan("");
    volatile double nan2 = __builtin_nan("0x123");
    volatile double normal = 3.14159;
    
    /* These comparisons should generate various condition codes */
    volatile int r1 = (nan1 < normal) ? 1 : 0;
    volatile int r2 = (normal > nan2) ? 1 : 0;
    volatile int r3 = (nan1 <= nan2) ? 1 : 0;
    volatile int r4 = (normal >= nan1) ? 1 : 0;
    volatile int r5 = (nan1 == nan2) ? 1 : 0;
    volatile int r6 = (normal != nan1) ? 1 : 0;
    
    cc_accumulator += r1 + r2 + r3 + r4 + r5 + r6;
    
    printf("Condition code accumulator: %d\n", cc_accumulator);
    
    /* Prevent optimization of the entire program */
    volatile int* volatile_ptr = &cc_accumulator;
    __asm__ volatile ("" : : "r"(volatile_ptr) : "memory");
    
    return cc_accumulator != 0;
}
