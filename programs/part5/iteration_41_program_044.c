/* test_resource_patterns.c
 * Compile with: gcc -O2 -fschedule-insns -fno-omit-frame-pointer test_resource_patterns.c -o test
 * For RTL dumps: gcc -O1 -da -fdump-rtl-all test_resource_patterns.c
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant propagation */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* Multiple bit-field extractions to increase chances */
    unsigned int mask1 = (1u << 5) - 1;  /* 5-bit mask */
    unsigned int mask2 = (1u << 3) - 1;  /* 3-bit mask */
    
    /* Force compiler to generate ZERO_EXTRACT for bit-field operations */
    unsigned int result = 0;
    result += (x >> shift) & mask1;      /* Should generate ZERO_EXTRACT */
    result += (x >> (shift + 1)) & mask2; /* Another extraction */
    
    /* More complex pattern with variable shift */
    unsigned int temp = x;
    for (int i = 0; i < 4; i++) {
        result += (temp >> (i * 3)) & 0x7;  /* Extract 3-bit fields */
    }
    
    return result;
}

/* Structure with bit-fields */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 4;
    unsigned int field4 : 16;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments and comparisons */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0xF;
    
    /* Bit-field comparisons (may generate ZERO_EXTRACT) */
    if (s->field1 == 10) result += 1;
    if (s->field2 > 50) result += 2;
    if (s->field3 != 0) result += 4;
    
    /* Combine bit-fields */
    result += (s->field1 << 16) | (s->field2 << 8) | s->field3;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* char operations that may use partial registers */
        char c = chars[i];
        c = c + g_volatile_seed;  /* Prevent optimization */
        chars[i] = c;             /* Store back only low 8 bits */
        result += (unsigned char)c;
        
        /* short operations */
        short s = shorts[i];
        s = s * 2 - 1;            /* Arithmetic on 16-bit value */
        shorts[i] = s;            /* Store back only low 16 bits */
        result += (unsigned short)s;
    }
    
    return result;
}

/* Volatile pointer writes */
unsigned int test_strict_low_part_volatile(volatile short *ptr, int iterations) {
    unsigned int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile write of short - may generate STRICT_LOW_PART */
        *ptr = (short)(i + g_volatile_seed);
        result += *ptr;  /* Read back */
        ptr++;           /* Move pointer */
    }
    
    return result;
}

/* Inline assembly for byte register operations */
unsigned int test_strict_low_part_asm(void) {
    unsigned int result = 0;
    unsigned char byte_val = g_volatile_seed & 0xFF;
    
    /* Assembly that operates on byte register */
    asm volatile (
        "movb %1, %%al\n\t"
        "addb $1, %%al\n\t"
        "movb %%al, %0"
        : "=r"(byte_val)
        : "r"(byte_val)
        : "%al"
    );
    
    result = byte_val;
    
    /* Another asm with explicit byte constraint */
    unsigned short word_val = g_volatile_seed & 0xFFFF;
    asm volatile (
        "movw %1, %%ax\n\t"
        "incw %%ax\n\t"
        "movw %%ax, %0"
        : "=r"(word_val)
        : "r"(word_val)
        : "%ax"
    );
    
    result += word_val;
    return result;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

unsigned int test_subreg_union(union type_pun *u, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same data */
        u[i].full = i * 0x01010101;
        
        /* SUBREG accesses to parts */
        result += u[i].halves[0];  /* Low 16 bits */
        result += u[i].halves[1];  /* High 16 bits */
        result += u[i].bytes[2];   /* Third byte */
    }
    
    return result;
}

/* Casting between types */
unsigned int test_subreg_casts(int *ints, short *shorts, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Cast int to short - may generate SUBREG */
        short s = (short)(ints[i] & 0xFFFF);
        shorts[i] = s;
        result += s;
        
        /* Cast short to int with sign extension */
        int extended = (int)shorts[i];
        result += extended & 0xFF;
    }
    
    return result;
}

/* Packed structure */
struct __attribute__((packed)) packed_struct {
    char a;
    short b;
    char c;
    int d;
};

unsigned int test_subreg_packed(struct packed_struct *ps, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Accesses to misaligned fields may use SUBREG */
        ps[i].a = i & 0xFF;
        ps[i].b = (i * 2) & 0xFFFF;
        ps[i].c = (i + 1) & 0xFF;
        ps[i].d = i * 1000;
        
        result += ps[i].a + ps[i].b + ps[i].c + ps[i].d;
    }
    
    return result;
}

/* ========== Combined patterns with memory references ========== */

/* Complex pattern combining multiple RTL types */
unsigned int test_combined_patterns(int *array, int size) {
    unsigned int result = 0;
    
    /* Create a union for type-punning */
    union type_pun u;
    
    for (int i = 0; i < size; i++) {
        /* Memory reference with index */
        int val = array[i] + g_volatile_seed;
        
        /* ZERO_EXTRACT: Extract bit-fields */
        unsigned int bits = (val >> 3) & 0x1F;  /* 5-bit field */
        bits += (val >> 8) & 0x7;               /* 3-bit field */
        
        /* SUBREG: Type punning */
        u.full = val;
        short low_half = u.halves[0];  /* Access low 16 bits */
        
        /* STRICT_LOW_PART: Partial write */
        char byte_val = (char)(low_half & 0xFF);
        u.bytes[1] = byte_val;  /* Modify only one byte */
        
        /* Complex memory address */
        array[(i + 1) % size] = u.full + bits;
        
        result += bits + low_half + byte_val;
    }
    
    return result;
}

/* ========== Main test driver ========== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data */
    int data_size = 100;
    int *int_array = (int*)malloc(data_size * sizeof(int));
    short *short_array = (short*)malloc(data_size * sizeof(short));
    char *char_array = (char*)malloc(data_size * sizeof(char));
    union type_pun *unions = (union type_pun*)malloc(data_size * sizeof(union type_pun));
    struct bitfield_struct bf_struct;
    struct packed_struct *packed = (struct packed_struct*)malloc(data_size * sizeof(struct packed_struct));
    
    /* Seed with pseudo-random but volatile data */
    for (int i = 0; i < data_size; i++) {
        int_array[i] = (i * 13 + g_volatile_seed) & 0x7FFFFFFF;
        short_array[i] = (short)(i * 17 + g_volatile_seed);
        char_array[i] = (char)(i * 19 + g_volatile_seed);
    }
    
    /* Test ZERO_EXTRACT patterns */
    final_result += test_zero_extract_int(int_array[0], 3);
    final_result += test_zero_extract_struct(&bf_struct, int_array[1]);
    
    /* Test STRICT_LOW_PART patterns */
    final_result += test_strict_low_part_chars(char_array, short_array, data_size / 2);
    volatile short *volatile_ptr = (volatile short*)short_array;
    final_result += test_strict_low_part_volatile(volatile_ptr, data_size / 4);
    final_result += test_strict_low_part_asm();
    
    /* Test SUBREG patterns */
    final_result += test_subreg_union(unions, data_size / 2);
    final_result += test_subreg_casts(int_array, short_array, data_size / 2);
    final_result += test_subreg_packed(packed, data_size / 4);
    
    /* Test combined patterns */
    final_result += test_combined_patterns(int_array, data_size);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_result);
    
    /* Cleanup */
    free(int_array);
    free(short_array);
    free(char_array);
    free(unions);
    free(packed);
    
    return (final_result > 0) ? 0 : 1;
}
