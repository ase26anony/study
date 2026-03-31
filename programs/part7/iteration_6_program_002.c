/* test_resource_coverage.c
 * 
 * This program is designed to generate RTL patterns that will exercise
 * the uncovered lines in GCC's resource.cc (lines 282-290) during
 * compilation with optimization enabled.
 * 
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fschedule-insns -c test_resource_coverage.c
 * For coverage: gcc -O2 -fprofile-arcs -ftest-coverage test_resource_coverage.c -o test
 */

#include <stdint.h>
#include <stdlib.h>

/* Ensure optimization is enabled at compile time */
#ifdef __OPTIMIZE__
#define OPTIMIZATION_ENABLED 1
#else
#define OPTIMIZATION_ENABLED 0
#endif

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT patterns using bit-field operations */
NOINLINE static int test_zero_extract(void) {
    /* Use volatile to prevent complete optimization */
    volatile unsigned int source = 0xABCD1234;
    volatile unsigned int shift = 8;
    volatile unsigned int mask = 0xFF;
    
    /* Bit-field extraction that may generate ZERO_EXTRACT in RTL */
    unsigned int result = (source >> shift) & mask;
    
    /* Additional bit-field operations */
    struct bitfield {
        unsigned int field1 : 4;
        unsigned int field2 : 8;
        unsigned int field3 : 4;
    } bf;
    
    bf.field1 = (source >> 0) & 0xF;
    bf.field2 = (source >> 4) & 0xFF;
    bf.field3 = (source >> 12) & 0xF;
    
    return result + bf.field1 + bf.field2 + bf.field3;
}

/* Function 2: Generate STRICT_LOW_PART patterns using inline assembly (x86) */
NOINLINE static int test_strict_low_part(void) {
    int result = 0;
    
    /* x86-specific inline assembly that may generate STRICT_LOW_PART */
#if defined(__i386__) || defined(__x86_64__)
    unsigned short val16 = 0x1234;
    unsigned char val8 = 0xAB;
    
    /* Byte operation that may use STRICT_LOW_PART */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(val8)
        : "r"((unsigned char)0xCD)
        : "cc"
    );
    
    /* Half-word operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(val16)
        : "r"((unsigned short)0x5678)
        : "cc"
    );
    
    result = val8 + val16;
#else
    /* Fallback for non-x86: use type punning that might generate similar patterns */
    unsigned int val = 0x12345678;
    unsigned short *ptr = (unsigned short*)&val;
    *ptr = 0xABCD;  /* Partial register write */
    result = val;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns using type conversions */
NOINLINE static int test_subreg(void) {
    int total = 0;
    
    /* Various type conversions that may generate SUBREG */
    long long big_val = 0x123456789ABCDEF0LL;
    int small_val = (int)big_val;  /* Truncation */
    short shorter = (short)small_val;
    char tiny = (char)shorter;
    
    /* Access different parts of larger types */
    union {
        long long ll;
        int i[2];
        short s[4];
        char c[8];
    } u;
    
    u.ll = big_val;
    total += u.i[0];  /* May involve SUBREG */
    total += u.s[2];  /* May involve SUBREG */
    total += u.c[5];  /* May involve SUBREG */
    
    /* Pointer casting between different sized types */
    int *int_ptr = &small_val;
    short *short_ptr = (short*)int_ptr;
    total += *short_ptr;
    
    return total + tiny;
}

/* Function 4: Generate complex MEM_P patterns with addressing modes */
NOINLINE static int test_mem_operands(void) {
    volatile int array[64];
    volatile int *ptr = array;
    volatile int indices[4] = {1, 3, 5, 7};
    
    /* Initialize array to prevent optimization */
    for (int i = 0; i < 64; i++) {
        array[i] = i * 3;
    }
    
    int sum = 0;
    
    /* Complex memory addressing patterns */
    for (int i = 0; i < 4; i++) {
        /* Variable index array access */
        sum += array[indices[i]];
        
        /* Pointer arithmetic with variable offset */
        sum += *(ptr + indices[i] * 2);
        
        /* Multi-dimensional like access */
        sum += array[i * 8 + indices[i % 2]];
    }
    
    /* Structure with multiple fields */
    struct data {
        int a;
        int b;
        int c[4];
        int d;
    } d;
    
    d.a = 10;
    d.b = 20;
    for (int i = 0; i < 4; i++) {
        d.c[i] = i * 5;
    }
    d.d = 30;
    
    /* Structure field accesses */
    sum += d.a + d.b + d.d;
    sum += d.c[indices[0] % 4];
    
    return sum;
}

/* Function 5: Combined test with loops to increase RTL generation */
NOINLINE static int test_combined(void) {
    int result = 0;
    
    /* Loop to increase chance of RTL generation and resource marking */
    for (int i = 0; i < 100; i++) {
        /* Mix different operations */
        if (i & 1) {
            result += test_zero_extract();
        }
        if (i & 2) {
            result += test_strict_low_part();
        }
        if (i & 4) {
            result += test_subreg();
        }
        if (i & 8) {
            result += test_mem_operands();
        }
        
        /* Prevent loop elimination */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    return result;
}

/* Main function that calls all test patterns */
int main(void) {
    /* Compile-time check for optimization */
    _Static_assert(OPTIMIZATION_ENABLED, 
                   "Compile with optimization (-O2 or -O3) for coverage");
    
    int total = 0;
    
    /* Call individual test functions */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_mem_operands();
    
    /* Call combined test */
    total += test_combined();
    
    /* Return predictable but non-constant result */
    return total % 256;
}
