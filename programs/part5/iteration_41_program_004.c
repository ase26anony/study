/* test_resource_patterns.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer test_resource_patterns.c -o test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all test_resource_patterns.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding */
volatile int g_seed = 42;

/* ========== ZERO_EXTRACT Patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* Multiple bit-field extractions to increase chances */
    unsigned int mask1 = (1 << 5) - 1;  /* 5-bit mask */
    unsigned int mask2 = (1 << 3) - 1;  /* 3-bit mask */
    unsigned int mask3 = 0xFF00;        /* 8-bit mask in middle */
    
    /* Force compiler to generate ZERO_EXTRACT for bit-field operations */
    unsigned int result = 0;
    result += (x >> shift) & mask1;           /* Simple extract */
    result += (x >> (shift + 1)) & mask2;     /* Another extract */
    result += (x & mask3) >> 8;               /* Mask then shift */
    
    /* Complex expression with multiple extracts */
    result += ((x >> 2) & 0x1F) + ((x >> 10) & 0x3F);
    
    return result;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 3;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    /* Bit-field assignments and comparisons */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x07;
    s->field3 = (val >> 8) & 0xFF;
    
    /* Bit-field comparisons that may generate ZERO_EXTRACT */
    unsigned int result = 0;
    if (s->field1 == 10) result += 1;
    if (s->field2 > 3) result += 2;
    if (s->field3 != 0) result += 4;
    
    /* Arithmetic with bit-fields */
    result += s->field1 * s->field2 + s->field3;
    
    return result;
}

/* ========== STRICT_LOW_PART Patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* These assignments may generate STRICT_LOW_PART for partial reg updates */
        char c = chars[i] + g_seed;
        short s = shorts[i] * 2;
        
        /* Mix operations to prevent optimization */
        sum += (unsigned int)c + (unsigned int)s;
        
        /* Write back partial results */
        chars[i] = c ^ 0x55;          /* Byte write */
        shorts[i] = s + i;           /* Short write */
    }
    
    return sum;
}

/* Volatile pointer writes for strict low part */
unsigned int test_strict_low_part_volatile(volatile short *ptr, int iterations) {
    unsigned int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile write of short - may generate STRICT_LOW_PART */
        *ptr = (short)(i + g_seed);
        
        /* Read back and accumulate */
        result += *ptr;
        ptr++;  /* Move pointer to avoid simple optimization */
    }
    
    return result;
}

/* Inline assembly for byte register operations */
unsigned int test_strict_low_part_asm(void) {
    unsigned int result = 0;
    
    /* Force byte register operations */
    for (int i = 0; i < 10; i++) {
        unsigned char byte_val = (i * 7) & 0xFF;
        unsigned int temp;
        
        /* Inline assembly that operates on byte register */
        __asm__ volatile (
            "movb %1, %%al\n\t"
            "addb $1, %%al\n\t"
            "movb %%al, %0"
            : "=r" (temp)
            : "r" (byte_val)
            : "%al"
        );
        
        result += temp;
    }
    
    return result;
}

/* ========== SUBREG Patterns ========== */

/* Union for type-punning (generates SUBREG) */
unsigned int test_subreg_union(int value) {
    union {
        int i;
        short s[2];
        char c[4];
    } u;
    
    u.i = value + g_seed;
    
    /* Access sub-parts through union - generates SUBREG */
    unsigned int result = 0;
    result += u.s[0];      /* Access low 16 bits */
    result += u.s[1] << 8; /* Access high 16 bits */
    result += u.c[2] * 3;  /* Access specific byte */
    
    return result;
}

/* Casting between different integer sizes */
unsigned int test_subreg_casts(int *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Multiple casts that may generate SUBREG */
        short s = (short)(array[i] & 0xFFFF);
        char c = (char)(array[i] >> 16);
        
        /* Mix operations to keep all values alive */
        sum += (unsigned int)s + ((unsigned int)c << 8);
        
        /* Another SUBREG pattern: 64-bit operation on 32-bit arch */
        long long big_val = (long long)array[i] * 3;
        sum += (unsigned int)(big_val & 0xFFFFFFFF);
    }
    
    return sum;
}

/* SIMD-like operations using manual packing */
unsigned int test_subreg_simd(unsigned int packed) {
    /* Extract 4 bytes from packed word */
    unsigned char b0 = (packed >> 0) & 0xFF;
    unsigned char b1 = (packed >> 8) & 0xFF;
    unsigned char b2 = (packed >> 16) & 0xFF;
    unsigned char b3 = (packed >> 24) & 0xFF;
    
    /* Process each byte separately */
    b0 = b0 * 2;
    b1 = b1 + 1;
    b2 = b2 ^ 0xAA;
    b3 = b3 - 5;
    
    /* Pack back together */
    unsigned int result = (b3 << 24) | (b2 << 16) | (b1 << 8) | b0;
    
    return result;
}

/* ========== Memory References with Complex Addresses ========== */

/* Complex addressing modes with bit-field operations */
unsigned int test_complex_memory(struct bitfield_struct *structs, int count) {
    unsigned int total = 0;
    
    for (int i = 0; i < count; i++) {
        /* Array access with index calculation */
        struct bitfield_struct *s = &structs[i];
        
        /* Bit-field operations on memory location */
        s->field1 = (i * 3) & 0x1F;
        s->field3 = (s->field1 + g_seed) & 0xFF;
        
        /* Complex memory reference in calculation */
        total += s->field1 + (s->field3 << 8);
        
        /* Pointer arithmetic with different types */
        short *short_ptr = (short *)s;
        total += short_ptr[1] + i;  /* May generate SUBREG + MEM */
    }
    
    return total;
}

/* ========== Main Test Driver ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some randomness */
    int data_size = 100;
    int *int_array = malloc(data_size * sizeof(int));
    char *char_array = malloc(data_size * sizeof(char));
    short *short_array = malloc(data_size * sizeof(short));
    struct bitfield_struct *structs = malloc(data_size * sizeof(struct bitfield_struct));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < data_size; i++) {
        int_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
        char_array[i] = (char)(int_array[i] & 0xFF);
        short_array[i] = (short)(int_array[i] & 0xFFFF);
        structs[i].field1 = i & 0x1F;
        structs[i].field2 = (i >> 2) & 0x07;
        structs[i].field3 = (i * 7) & 0xFF;
        structs[i].field4 = i * 100;
    }
    
    /* Test ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    for (int i = 0; i < 10; i++) {
        final_result ^= test_zero_extract_int(int_array[i], i % 16);
        final_result += test_zero_extract_struct(&structs[i], int_array[i]);
    }
    
    /* Test STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    final_result += test_strict_low_part_chars(char_array, short_array, data_size / 2);
    
    volatile short volatile_short = 0;
    final_result += test_strict_low_part_volatile(&volatile_short, 20);
    
    final_result += test_strict_low_part_asm();
    
    /* Test SUBREG patterns */
    printf("Testing SUBREG patterns...\n");
    for (int i = 0; i < 20; i++) {
        final_result += test_subreg_union(int_array[i]);
    }
    
    final_result += test_subreg_casts(int_array, data_size / 4);
    
    for (int i = 0; i < 10; i++) {
        final_result ^= test_subreg_simd(int_array[i]);
    }
    
    /* Test complex memory patterns */
    printf("Testing complex memory patterns...\n");
    final_result += test_complex_memory(structs, data_size / 2);
    
    /* Clean up */
    free(int_array);
    free(char_array);
    free(short_array);
    free(structs);
    
    printf("Final result: %u\n", final_result);
    
    /* Return non-zero to indicate success */
    return (final_result != 0) ? 0 : 1;
}
