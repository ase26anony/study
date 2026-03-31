/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource.cc file to achieve coverage of lines 282-290:
 * - ZERO_EXTRACT expressions (bit-field operations)
 * - STRICT_LOW_PART expressions (partial register updates)
 * - SUBREG expressions (register sub-parts)
 * - Memory references with complex addressing
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT operations */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 10;
};

/* Function 1: Explicit bit-field extraction */
unsigned int test_zero_extract_explicit(unsigned int x, unsigned int shift) {
    /* This should generate ZERO_EXTRACT RTL */
    unsigned int mask = (1 << 8) - 1;  /* 8-bit mask */
    return (x >> shift) & mask;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Multiple bit-field assignments and comparisons */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0x3FF;
    
    /* Bit-field comparisons */
    if (s->field1 == 0x10) {
        result |= 1;
    }
    if (s->field2 > 0x20) {
        result |= 2;
    }
    if (s->field3 != 0) {
        result |= 4;
    }
    
    /* Combined bit-field arithmetic */
    s->field4 = (s->field1 + s->field2) & 0x3FF;
    
    return result + s->field4;
}

/* Function 3: Complex bit-field extraction with memory */
unsigned int test_zero_extract_memory(unsigned int *array, int index) {
    /* Extract bits 8-15 from array element */
    unsigned int val = array[index];
    return (val & 0xFF00) >> 8;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Function 4: Partial register updates with char/short */
unsigned int test_strict_low_part_charshort(char *cptr, short *sptr, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* These assignments should generate STRICT_LOW_PART */
        char c = (char)(g_volatile_seed + i);
        short s = (short)(g_volatile_seed * i);
        
        *cptr = c;      /* Partial register write (byte) */
        *sptr = s;      /* Partial register write (word) */
        
        /* Use volatile to prevent optimization */
        sum += (unsigned int)(*cptr) + (unsigned int)(*sptr);
        
        /* Pointer arithmetic to create different addresses */
        cptr++;
        sptr++;
    }
    
    return sum;
}

/* Function 5: Volatile pointer writes */
unsigned int test_strict_low_part_volatile(volatile short *vsptr, int val) {
    /* Volatile write to short - should preserve high bits if in register */
    *vsptr = (short)val;
    return *vsptr;  /* Read back to ensure write happens */
}

/* Function 6: Inline assembly for partial register (x86-specific) */
#ifdef __x86_64__
unsigned int test_strict_low_part_asm(unsigned int x) {
    unsigned char result;
    
    /* Inline assembly that writes to byte register */
    __asm__ volatile (
        "movb %%al, %0\n\t"
        : "=q" (result)   /* q = a, b, c, or d register (byte) */
        : "a" (x)         /* input in eax */
        : "cc"
    );
    
    return result;
}
#else
/* Fallback for non-x86 */
unsigned int test_strict_low_part_asm(unsigned int x) {
    return (unsigned char)x;  /* Still generates partial register use */
}
#endif

/* ========== SUBREG PATTERNS ========== */

/* Function 7: Union for type-punning (SUBREG generation) */
unsigned int test_subreg_union(unsigned int value) {
    union {
        uint32_t i;
        uint16_t s[2];
        uint8_t  b[4];
    } u;
    
    u.i = value;
    
    /* Access sub-parts - should generate SUBREG */
    unsigned int sum = u.s[0] + u.s[1];
    sum += u.b[0] + u.b[1] + u.b[2] + u.b[3];
    
    /* Modify through sub-parts */
    u.s[1] = (uint16_t)(sum & 0xFFFF);
    
    return u.i;
}

/* Function 8: Casting between integer sizes */
unsigned int test_subreg_casting(unsigned int x, unsigned int y) {
    /* Multiple casts generating SUBREG operations */
    short s1 = (short)(x & 0xFFFF);
    short s2 = (short)(y >> 16);
    char c1 = (char)(x & 0xFF);
    char c2 = (char)(y & 0xFF);
    
    /* Operations on sub-register values */
    unsigned int result = (unsigned int)s1 * (unsigned int)s2;
    result += (unsigned int)c1 * (unsigned int)c2;
    
    /* Cast back and forth */
    result = (unsigned int)(short)(result & 0xFFFF);
    
    return result;
}

/* Function 9: SIMD-like operations (manual vectorization) */
unsigned int test_subreg_simd(unsigned int a, unsigned int b) {
    /* Treat 32-bit integers as packed 16-bit values */
    unsigned short a_lo = (unsigned short)(a & 0xFFFF);
    unsigned short a_hi = (unsigned short)(a >> 16);
    unsigned short b_lo = (unsigned short)(b & 0xFFFF);
    unsigned short b_hi = (unsigned short)(b >> 16);
    
    /* SIMD-like operations */
    unsigned int sum_lo = a_lo + b_lo;
    unsigned int sum_hi = a_hi + b_hi;
    
    /* Recombine with potential SUBREG */
    return (sum_hi << 16) | (sum_lo & 0xFFFF);
}

/* ========== COMPLEX MEMORY ADDRESSING ========== */

/* Function 10: Complex addressing with bit-fields */
unsigned int test_complex_addressing(struct bitfield_struct *structs, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Array access with index calculation */
        struct bitfield_struct *s = &structs[i];
        
        /* Multiple bit-field operations with memory */
        s->field1 = (i * 3) & 0x1F;
        s->field2 = (i * 5) & 0x7F;
        
        /* Complex addressing: struct pointer + field offset */
        sum += s->field1 + s->field2;
        
        /* Pointer arithmetic */
        if (i % 2 == 0) {
            s->field3 = sum & 0x3FF;
        }
    }
    
    return sum;
}

/* Function 11: Mixed patterns in loop */
unsigned int test_mixed_patterns(unsigned int *data, int size) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < size; i++) {
        /* ZERO_EXTRACT pattern */
        unsigned int extracted = (data[i] >> 4) & 0x0F0F0F0F;
        
        /* SUBREG pattern through union */
        union { unsigned int i; unsigned short s[2]; } u;
        u.i = extracted;
        checksum += u.s[0] + u.s[1];
        
        /* Partial write pattern */
        if (i % 3 == 0) {
            volatile short *vs = (volatile short *)&data[i];
            *vs = (short)checksum;  /* STRICT_LOW_PART candidate */
        }
        
        /* Complex addressing */
        data[(i + 1) % size] ^= checksum;
    }
    
    return checksum;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some randomness */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations <= 0) iterations = 100;
    
    /* Allocate test arrays */
    unsigned int *int_array = (unsigned int*)malloc(iterations * sizeof(unsigned int));
    struct bitfield_struct *bf_array = (struct bitfield_struct*)malloc(iterations * sizeof(struct bitfield_struct));
    char *char_array = (char*)malloc(iterations * sizeof(char));
    short *short_array = (short*)malloc(iterations * sizeof(short));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < iterations; i++) {
        int_array[i] = g_volatile_seed * i + 12345;
        bf_array[i].field1 = i & 0x1F;
        bf_array[i].field2 = (i * 3) & 0x7F;
        bf_array[i].field3 = (i * 7) & 0x3FF;
        bf_array[i].field4 = (i * 11) & 0x3FF;
        char_array[i] = (char)(i * 13);
        short_array[i] = (short)(i * 17);
    }
    
    /* Test 1: ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    for (int i = 0; i < iterations; i++) {
        final_result ^= test_zero_extract_explicit(int_array[i], i % 32);
        final_result += test_zero_extract_struct(&bf_array[i], int_array[i]);
        final_result += test_zero_extract_memory(int_array, i % iterations);
    }
    
    /* Test 2: STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    final_result += test_strict_low_part_charshort(char_array, short_array, iterations);
    final_result += test_strict_low_part_volatile((volatile short*)short_array, g_volatile_seed);
    final_result += test_strict_low_part_asm(final_result);
    
    /* Test 3: SUBREG patterns */
    printf("Testing SUBREG patterns...\n");
    for (int i = 0; i < iterations; i++) {
        final_result ^= test_subreg_union(int_array[i]);
        final_result += test_subreg_casting(int_array[i], int_array[(i + 1) % iterations]);
        final_result += test_subreg_simd(int_array[i], int_array[(i + 2) % iterations]);
    }
    
    /* Test 4: Complex addressing */
    printf("Testing complex addressing patterns...\n");
    final_result += test_complex_addressing(bf_array, iterations);
    final_result += test_mixed_patterns(int_array, iterations);
    
    /* Cleanup */
    free(int_array);
    free(bf_array);
    free(char_array);
    free(short_array);
    
    /* Return final result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_result);
    return (int)(final_result & 0x7FFFFFFF);  /* Return non-negative */
}
