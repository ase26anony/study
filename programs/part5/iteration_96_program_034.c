/* Condition code coverage test for i386.cc lines 13992-14017 */
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <xmmintrin.h>

/* Force generation of specific condition codes */
#define FORCE_COND_CODE(cond, a, b) \
    do { \
        int result; \
        __asm__ volatile ( \
            "fucomi %2, %1\n\t" \
            "set%C0 %0" \
            : "=r"(result) \
            : "f"(a), "f"(b), "i"(cond) \
            : "cc" \
        ); \
        cond_acc += result; \
    } while(0)

/* Direct inline assembly with %C constraint */
#define EMIT_COND_MOVE(cond, dest, src) \
    do { \
        __asm__ volatile ( \
            "test $1, %1\n\t" \
            "cmov%C0 %2, %0" \
            : "+r"(dest) \
            : "r"(src), "i"(cond) \
            : "cc" \
        ); \
    } while(0)

/* Generate all condition codes via floating comparisons */
static void generate_all_condition_codes(void) {
    volatile double nan = __builtin_nan("");
    volatile double inf = __builtin_inf();
    volatile double normal = 3.14159;
    volatile double zero = 0.0;
    
    int cc_accumulator = 0;
    
    /* UNORDERED: NaN comparison */
    if (!(nan == nan)) cc_accumulator |= 1;
    if (isunordered(nan, normal)) cc_accumulator |= 2;
    
    /* ORDERED: normal comparison */
    if (normal < inf) cc_accumulator |= 4;
    if (isgreater(normal, zero)) cc_accumulator |= 8;
    
    /* UNEQ: unordered or equal */
    volatile double a = nan;
    volatile double b = nan;
    if (a == b || isunordered(a, b)) cc_accumulator |= 16;
    
    /* UNGE: unordered or greater-or-equal */
    if (normal >= zero || isunordered(normal, zero)) cc_accumulator |= 32;
    
    /* UNGT: unordered or greater */
    if (inf > normal || isunordered(inf, normal)) cc_accumulator |= 64;
    
    /* UNLE: unordered or less-or-equal */
    if (zero <= normal || isunordered(zero, normal)) cc_accumulator |= 128;
    
    /* UNLT: unordered or less */
    if (zero < normal || isunordered(zero, normal)) cc_accumulator |= 256;
    
    /* LTGT: less or greater (ordered and not equal) */
    if ((normal < inf) != (normal > inf)) cc_accumulator |= 512;
    
    /* Prevent optimization */
    __asm__ volatile ("" : : "r"(cc_accumulator));
}

int main(void) {
    /* Mix of NaN and normal values */
    double arr1[256];
    double arr2[256];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 256; i++) {
        arr1[i] = i * 1.5;
        arr2[i] = (i + 1) * 1.7;
        
        /* Insert NaN at specific indices */
        if (i % 7 == 0) {
            arr1[i] = __builtin_nan("");
        }
        if (i % 11 == 0) {
            arr2[i] = __builtin_nan("");
        }
        if (i % 13 == 0) {
            arr1[i] = __builtin_inf();
            arr2[i] = -__builtin_inf();
        }
    }
    
    volatile int cond_acc = 0;
    int temp_result = 0;
    
    /* Loop with all comparison types */
    for (int i = 0; i < 256; i++) {
        volatile double a = arr1[i];
        volatile double b = arr2[i];
        
        /* Generate condition codes via comparisons */
        int lt = (a < b) ? 1 : 0;
        int le = (a <= b) ? 1 : 0;
        int gt = (a > b) ? 1 : 0;
        int ge = (a >= b) ? 1 : 0;
        int eq = (a == b) ? 1 : 0;
        int ne = (a != b) ? 1 : 0;
        
        /* Force conditional move generation */
        temp_result = (a < b) ? temp_result + 1 : temp_result - 1;
        temp_result = (a <= b) ? temp_result * 2 : temp_result / 2;
        temp_result = (a > b) ? temp_result | 0xFF : temp_result & 0x0F;
        temp_result = (a >= b) ? temp_result ^ 0xAA : temp_result ^ 0x55;
        temp_result = (a == b) ? ~temp_result : temp_result;
        temp_result = (a != b) ? temp_result << 2 : temp_result >> 2;
        
        cond_acc += lt + le + gt + ge + eq + ne;
    }
    
    /* Direct inline assembly with %C constraint for each condition code */
    int x = 42, y = 100;
    
    /* UNORDERED */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(16)  /* 16 = UNORDERED */
        : "cc"
    );
    
    /* ORDERED */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(17)  /* 17 = ORDERED */
        : "cc"
    );
    
    /* UNEQ */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(18)  /* 18 = UNEQ */
        : "cc"
    );
    
    /* UNGE */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(19)  /* 19 = UNGE */
        : "cc"
    );
    
    /* UNGT */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(20)  /* 20 = UNGT */
        : "cc"
    );
    
    /* UNLE */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(21)  /* 21 = UNLE */
        : "cc"
    );
    
    /* UNLT */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(22)  /* 22 = UNLT */
        : "cc"
    );
    
    /* LTGT */
    __asm__ volatile (
        "test $1, %1\n\t"
        "cmov%C0 %2, %0"
        : "+r"(x)
        : "r"(y), "i"(23)  /* 23 = LTGT */
        : "cc"
    );
    
    /* Generate all condition codes */
    generate_all_condition_codes();
    
    /* Prevent dead code elimination */
    printf("Result: %d %d\n", cond_acc, temp_result + x);
    
    return 0;
}
