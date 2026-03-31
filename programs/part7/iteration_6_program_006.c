/* test_resource_coverage.c
 * 
 * This test program is designed to trigger specific RTL patterns in GCC's
 * resource management code (resource.cc lines 282-290). When compiled with
 * optimization, it should generate RTL containing:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register accesses (x86-specific)
 * - SUBREG for type conversions and partial accesses
 * - MEM_P with complex addressing modes
 */

#include <stdint.h>
#include <assert.h>

/* Ensure optimization is enabled for RTL generation */
#ifndef __OPTIMIZE__
#error "This test requires optimization (-O1, -O2, or -O3) to generate target RTL patterns"
#endif

/* Prevent inlining to ensure separate RTL generation for each function */
#define NOINLINE __attribute__((noinline))

/* Function 1: Generate ZERO_EXTRACT patterns through bit-field operations */
NOINLINE static int bitfield_operations(void) {
    /* Use volatile to prevent constant propagation */
    volatile unsigned int value = 0xABCD1234;
    volatile unsigned int mask = 0x00000FFF;
    
    /* Multiple bit-field extractions that may generate ZERO_EXTRACT */
    unsigned int result = 0;
    
    /* Extract bits 4-11 */
    result += (value >> 4) & 0xFF;
    
    /* Extract bits 8-15 with different shift */
    result += (value >> 8) & 0xFF;
    
    /* Extract bits 12-19 */
    result += (value >> 12) & 0x7F;
    
    /* Complex extraction with variable shift */
    volatile int shift = 3;
    result += (value >> shift) & mask;
    
    return result;
}

/* Function 2: Generate STRICT_LOW_PART patterns (x86-specific) */
NOINLINE static int partial_register_ops(void) {
    int result = 0;
    
    /* x86-specific inline assembly for byte operations */
#if defined(__i386__) || defined(__x86_64__)
    unsigned char byte_val = 0;
    unsigned int dword_val = 0x12345678;
    
    /* Assembly that modifies only part of a register */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q" (byte_val)      /* =q constraint for byte-addressable register */
        : "r" ((unsigned char)dword_val)
        : "cc"
    );
    
    /* Another byte operation */
    unsigned short word_val = 0;
    asm volatile (
        "movw %1, %0\n\t"
        : "=r" (word_val)      /* Word operation that may use partial reg */
        : "r" ((unsigned short)dword_val)
        : "cc"
    );
    
    result = byte_val + word_val;
#else
    /* Fallback for non-x86: use bit-field structs which may also generate
       partial register accesses through different mechanisms */
    struct {
        unsigned int low : 8;
        unsigned int high : 8;
    } bits = {0xAB, 0xCD};
    
    result = bits.low + bits.high;
#endif
    
    return result;
}

/* Function 3: Generate SUBREG patterns through type conversions */
NOINLINE static int subreg_conversions(void) {
    volatile long long big_value = 0x123456789ABCDEF0LL;
    volatile int int_value = 0;
    volatile short short_value = 0;
    volatile char char_value = 0;
    
    /* Various type conversions that may generate SUBREG */
    int_value = (int)big_value;           /* truncation */
    short_value = (short)int_value;       /* int to short */
    char_value = (char)short_value;       /* short to char */
    
    /* Access different parts of larger types */
    int low_part = (int)(big_value & 0xFFFFFFFF);
    int high_part = (int)((big_value >> 32) & 0xFFFFFFFF);
    
    /* Use union for type punning - may generate SUBREG accesses */
    union {
        long long ll;
        int i[2];
    } converter;
    
    converter.ll = big_value;
    int_value = converter.i[0] + converter.i[1];
    
    return int_value + short_value + char_value + low_part + high_part;
}

/* Function 4: Generate complex MEM_P addressing patterns */
NOINLINE static int complex_memory_access(void) {
    /* Use arrays with variable indices for complex addressing */
    volatile int array1[256];
    volatile int array2[256];
    volatile int array3[256];
    
    /* Initialize arrays to prevent optimization */
    for (int i = 0; i < 256; i++) {
        array1[i] = i;
        array2[i] = i * 2;
        array3[i] = i * 3;
    }
    
    volatile int index1 = 10;
    volatile int index2 = 20;
    volatile int index3 = 30;
    volatile int stride = 4;
    
    int result = 0;
    
    /* Complex addressing modes with multiple variables */
    result += array1[index1];
    result += array2[index1 + index2];
    result += array3[index1 * stride + index3];
    
    /* Pointer arithmetic with variable offsets */
    int *ptr1 = (int*)array1;
    int *ptr2 = (int*)array2;
    
    result += *(ptr1 + index1);
    result += *(ptr2 + index1 * 2);
    
    /* Multi-dimensional array-like access */
    result += array1[(index1 * 3 + index2) & 0xFF];
    
    /* Structure access that may generate complex MEM */
    struct {
        int a;
        int b;
        int c[4];
    } s = {0};
    
    s.a = index1;
    s.b = index2;
    s.c[0] = index3;
    
    result += s.a + s.b + s.c[index1 & 3];
    
    return result;
}

/* Function 5: Mixed operations to increase RTL pattern diversity */
NOINLINE static int mixed_operations(int iterations) {
    int sum = 0;
    volatile int counter = iterations;
    
    /* Loop to keep variables live and generate more RTL */
    while (counter-- > 0) {
        /* Mix different operations */
        sum += bitfield_operations() & 0xFF;      /* May generate ZERO_EXTRACT */
        sum += partial_register_ops() & 0xFF;     /* May generate STRICT_LOW_PART */
        sum += subreg_conversions() & 0xFF;       /* May generate SUBREG */
        sum += complex_memory_access() & 0xFF;    /* May generate complex MEM */
        
        /* Additional bit manipulation */
        volatile int temp = sum;
        sum = (temp << 3) | (temp >> 5);  /* Rotate */
    }
    
    return sum;
}

/* Main function that drives all patterns */
int main(void) {
    int result = 0;
    
    /* Call each pattern function */
    result += bitfield_operations();
    result += partial_register_ops();
    result += subreg_conversions();
    result += complex_memory_access();
    
    /* Call mixed operations with a loop */
    result += mixed_operations(3);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple validation - just ensure we return something non-crashing */
    return (result != 0) ? 0 : 1;
}
