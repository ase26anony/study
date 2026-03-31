/* test_resource_coverage.c
 * 
 * This program generates RTL patterns to exercise uncovered code in GCC's
 * resource.cc file, specifically targeting:
 * - ZERO_EXTRACT expressions (bit-field operations)
 * - STRICT_LOW_PART expressions (partial register updates)
 * - SUBREG expressions (register sub-parts)
 * - Complex memory references
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ==================== ZERO_EXTRACT PATTERNS ==================== */

/* Bit-field structure for ZERO_EXTRACT operations */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Function 1: Explicit bit-field extraction using shift and mask */
unsigned int test_zero_extract_shift_mask(unsigned int x, unsigned int shift) {
    unsigned int result = 0;
    
    /* Multiple ZERO_EXTRACT patterns */
    result += (x >> shift) & 0x1F;           /* Extract 5 bits */
    result += (x >> (shift + 1)) & 0x7F;     /* Extract 7 bits */
    result += (x >> (shift + 2)) & 0x7;      /* Extract 3 bits */
    
    /* Combine extractions with arithmetic */
    result = ((result & 0xFF) << 8) | ((result >> 8) & 0xFF);
    
    return result;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments (generate ZERO_EXTRACT in SET_DEST) */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0x7;
    
    /* Bit-field comparisons */
    if (s->field1 == 0x10) result += 1;
    if (s->field2 > 0x20) result += 2;
    if (s->field3 != 0) result += 4;
    
    /* Complex bit-field expression */
    result += ((s->field1 << s->field3) | s->field2) & 0xFF;
    
    return result;
}

/* Function 3: Array processing with bit-field extraction */
unsigned int test_zero_extract_array(unsigned int *arr, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Various bit-field extractions from array elements */
        sum += (arr[i] & 0xFF00) >> 8;          /* Extract byte */
        sum += (arr[i] >> 4) & 0xF;             /* Extract nibble */
        sum += (arr[i] >> 16) & 0x7FFF;         /* Extract 15 bits */
        
        /* Nested extractions */
        unsigned int temp = arr[i];
        temp = ((temp & 0xF) << 4) | ((temp >> 4) & 0xF);
        sum += temp & 0x3F;
    }
    
    return sum;
}

/* ==================== STRICT_LOW_PART PATTERNS ==================== */

/* Function 4: Partial register updates with small integer types */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* char operations that may use partial registers */
        char c = chars[i];
        c = c + g_volatile_seed;      /* Prevent constant folding */
        chars[i] = c;                 /* Potential STRICT_LOW_PART store */
        result += (unsigned char)c;
        
        /* short operations */
        short s = shorts[i];
        s = s * 2 + i;                /* Arithmetic in register */
        shorts[i] = s;                /* Partial register write */
        result += (unsigned short)s;
    }
    
    return result;
}

/* Function 5: Volatile pointer writes for STRICT_LOW_PART */
unsigned int test_strict_low_part_volatile(volatile short *vptr, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile write of short - likely generates STRICT_LOW_PART */
        *vptr = (short)(i + g_volatile_seed);
        sum += *vptr;  /* Read back to prevent elimination */
        
        /* Multiple volatile writes */
        vptr[1] = (short)(sum & 0xFFFF);
        sum += vptr[1];
    }
    
    return sum;
}

/* Function 6: Inline assembly for explicit partial register access */
unsigned int test_strict_low_part_asm(int x) {
    unsigned int result = 0;
    short s_val;
    char c_val;
    
    /* Byte register operation */
    asm volatile (
        "movb %1, %0\n\t"
        : "=q"(c_val)        /* 'q' constraint for byte register */
        : "r"((char)x)
        : "cc"
    );
    result += c_val;
    
    /* Short register operation */
    asm volatile (
        "movw %1, %0\n\t"
        : "=r"(s_val)        /* May use partial register */
        : "r"((short)(x >> 8))
        : "cc"
    );
    result += s_val;
    
    return result;
}

/* ==================== SUBREG PATTERNS ==================== */

/* Union for type-punning (generates SUBREG) */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function 7: Union-based SUBREG generation */
unsigned int test_subreg_union(union type_pun *data, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of the same data (generates SUBREG) */
        sum += data[i].halves[0];      /* Access low 16 bits */
        sum += data[i].halves[1];      /* Access high 16 bits */
        sum += data[i].bytes[1];       /* Access single byte */
        
        /* Modify through one view, read through another */
        data[i].parts.low = (uint16_t)(sum & 0xFFFF);
        sum += data[i].full;           /* Read full 32-bit */
    }
    
    return sum;
}

/* Function 8: Casting between integer sizes */
unsigned int test_subreg_casts(int *ints, short *shorts, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Casts that may generate SUBREG */
        short s = (short)(ints[i] & 0xFFFF);
        char c = (char)(ints[i] >> 16);
        
        /* Store back through different types */
        shorts[i] = s;
        result += (unsigned short)s;
        result += (unsigned char)c;
        
        /* More complex casting chain */
        int temp = ints[i];
        temp = (temp << 8) | (temp >> 24);  /* Rotate */
        s = (short)temp;
        result += s;
    }
    
    return result;
}

/* Function 9: Packed structure with mixed types */
struct packed_data {
    char a;
    short b;
    char c;
    int d;
} __attribute__((packed));

unsigned int test_subreg_packed(struct packed_data *pd, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access packed fields (may use SUBREG for misaligned access) */
        sum += pd[i].a;
        sum += pd[i].b;
        sum += pd[i].c;
        sum += pd[i].d;
        
        /* Modify and read back */
        pd[i].b = (short)(sum & 0xFFFF);
        sum += pd[i].b;
    }
    
    return sum;
}

/* ==================== COMPLEX MEMORY REFERENCES ==================== */

/* Function 10: Complex addressing modes */
unsigned int test_complex_memory(int *base, int *offsets, int count) {
    unsigned int result = 0;
    
    for (int i = 0; i < count; i++) {
        /* Array indexing with variable offset */
        int idx = offsets[i] & 0xF;
        result += base[idx * 2];          /* Complex address calculation */
        
        /* Pointer arithmetic */
        int *ptr = base + idx;
        result += ptr[1];                 /* Offset from computed pointer */
        
        /* Multiple dimensions */
        result += *(base + idx + i);      /* Combined offset */
    }
    
    return result;
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with volatile to prevent optimization */
    int data_size = 100;
    unsigned int *array = malloc(data_size * sizeof(unsigned int));
    struct bitfield_struct bf_struct;
    char *char_array = malloc(data_size * sizeof(char));
    short *short_array = malloc(data_size * sizeof(short));
    union type_pun *unions = malloc(data_size * sizeof(union type_pun));
    struct packed_data *packed = malloc(data_size * sizeof(struct packed_data));
    int *offsets = malloc(data_size * sizeof(int));
    
    /* Initialize with pseudo-random data using volatile seed */
    for (int i = 0; i < data_size; i++) {
        array[i] = i + g_volatile_seed;
        char_array[i] = (char)(i * 3);
        short_array[i] = (short)(i * 5);
        unions[i].full = i * 7;
        packed[i].a = (char)i;
        packed[i].b = (short)(i * 2);
        packed[i].c = (char)(i * 3);
        packed[i].d = i * 11;
        offsets[i] = i % 16;
    }
    
    /* Test ZERO_EXTRACT patterns */
    final_result += test_zero_extract_shift_mask(g_volatile_seed, 3);
    final_result += test_zero_extract_struct(&bf_struct, g_volatile_seed);
    final_result += test_zero_extract_array(array, data_size / 10);
    
    /* Test STRICT_LOW_PART patterns */
    final_result += test_strict_low_part_chars(char_array, short_array, data_size / 5);
    volatile short volatile_short;
    final_result += test_strict_low_part_volatile(&volatile_short, 50);
    final_result += test_strict_low_part_asm(g_volatile_seed);
    
    /* Test SUBREG patterns */
    final_result += test_subreg_union(unions, data_size / 10);
    final_result += test_subreg_casts(array, short_array, data_size / 10);
    final_result += test_subreg_packed(packed, data_size / 10);
    
    /* Test complex memory references */
    final_result += test_complex_memory(array, offsets, data_size / 10);
    
    /* Cleanup */
    free(array);
    free(char_array);
    free(short_array);
    free(unions);
    free(packed);
    free(offsets);
    
    /* Return result to prevent dead code elimination */
    printf("Final result: %u\n", final_result);
    return (int)(final_result & 0x7FFFFFFF);
}
