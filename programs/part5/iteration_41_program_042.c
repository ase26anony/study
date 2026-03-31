/* test_resource_coverage.c
 * 
 * This program is designed to trigger specific RTL patterns in GCC's
 * resource management subsystem to cover lines 282-290 in resource.cc:
 * - ZERO_EXTRACT expressions (bit-field operations)
 * - STRICT_LOW_PART expressions (partial register updates)
 * - SUBREG expressions (register sub-parts)
 * - Memory references with complex addressing
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
    unsigned int field2 : 8;
    unsigned int field3 : 3;
    unsigned int field4 : 16;
};

/* Function 1: Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* This should generate ZERO_EXTRACT RTL */
    unsigned int mask = (1 << 9) - 1;  /* 9-bit mask, not full word */
    return (x >> shift) & mask;
}

/* Function 2: Bit-field structure operations */
unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignment - should generate ZERO_EXTRACT */
    s->field1 = val & 0x1F;
    
    /* Bit-field comparison - should generate ZERO_EXTRACT */
    if (s->field2 == 0x55) {
        result |= 0x100;
    }
    
    /* Bit-field arithmetic */
    result += s->field3;
    
    /* Complex bit-field expression */
    result += ((s->field4 & 0xFF) << 3) | ((s->field4 >> 8) & 0x7);
    
    return result;
}

/* Function 3: Explicit bit-field extraction */
unsigned int test_explicit_bitfields(unsigned int x) {
    /* Multiple bit-field extractions */
    unsigned int part1 = (x & 0x000000FF) >> 0;   /* Low byte */
    unsigned int part2 = (x & 0x0000FF00) >> 8;   /* Next byte */
    unsigned int part3 = (x & 0x00FF0000) >> 16;  /* High byte */
    unsigned int part4 = (x & 0xFF000000) >> 24;  /* Top byte */
    
    /* Combine with shifting to ensure ZERO_EXTRACT usage */
    return (part1 << 24) | (part2 << 16) | (part3 << 8) | part4;
}

/* ==================== STRICT_LOW_PART PATTERNS ==================== */

/* Function 4: Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *chars, short *shorts, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* These assignments should generate STRICT_LOW_PART RTL
         * as we're writing partial registers */
        chars[i] = (chars[i] + g_volatile_seed) & 0xFF;
        shorts[i] = (shorts[i] * 3) & 0xFFFF;
        
        /* Use the results to prevent elimination */
        sum += chars[i] + shorts[i];
    }
    
    return sum;
}

/* Function 5: Volatile pointer writes for STRICT_LOW_PART */
unsigned int test_volatile_partial_writes(volatile short *vptr, int iterations) {
    unsigned int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Volatile write to short - should generate STRICT_LOW_PART */
        *vptr = (short)(i * 7);
        
        /* Read back through different type to force register usage */
        result += *(volatile short*)vptr;
    }
    
    return result;
}

/* Function 6: Inline assembly for byte register operations */
unsigned int test_asm_strict_low_part(unsigned int x) {
    unsigned char byte1, byte2;
    
    /* Inline assembly that operates on byte registers */
    asm volatile (
        "movb %[in1], %[out1]\n\t"
        "addb $5, %[out1]\n\t"
        : [out1] "=q" (byte1)
        : [in1] "q" ((unsigned char)(x & 0xFF))
        : "cc"
    );
    
    asm volatile (
        "movb %[in2], %[out2]\n\t"
        "xorb $0xAA, %[out2]\n\t"
        : [out2] "=q" (byte2)
        : [in2] "q" ((unsigned char)((x >> 8) & 0xFF))
        : "cc"
    );
    
    return (byte2 << 8) | byte1;
}

/* ==================== SUBREG PATTERNS ==================== */

/* Union for type-punning SUBREG generation */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

/* Function 7: Union-based SUBREG operations */
unsigned int test_subreg_union(union type_pun *u, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* These accesses should generate SUBREG RTL */
        u[i].full = i * 0x01010101;
        
        /* Access sub-parts through different views */
        sum += u[i].halves[0] + u[i].halves[1];
        sum += u[i].bytes[1] * u[i].bytes[3];
        
        /* Type punning through pointer casting */
        uint16_t *half_ptr = (uint16_t*)&u[i].full;
        sum += half_ptr[0] ^ half_ptr[1];
    }
    
    return sum;
}

/* Function 8: Casting between integer sizes */
unsigned int test_subreg_casting(unsigned int *data, int len) {
    unsigned int checksum = 0;
    
    for (int i = 0; i < len; i++) {
        /* Casting to smaller types should generate SUBREG */
        uint16_t low16 = (uint16_t)(data[i] & 0xFFFF);
        uint16_t high16 = (uint16_t)((data[i] >> 16) & 0xFFFF);
        
        /* Cast back with operations */
        checksum += ((unsigned int)low16 << 16) | high16;
        
        /* Byte extraction through casting */
        unsigned char bytes[4];
        *(unsigned int*)bytes = data[i];
        checksum += bytes[0] + (bytes[1] << 8) + (bytes[2] << 16) + (bytes[3] << 24);
    }
    
    return checksum;
}

/* ==================== COMPLEX MEMORY PATTERNS ==================== */

/* Function 9: Complex addressing with bit-fields */
unsigned int test_complex_memory(struct bitfield_struct *array, int size, int *indices) {
    unsigned int result = 0;
    
    for (int i = 0; i < size; i++) {
        /* Complex addressing: array + index calculation */
        int idx = indices[i] % size;
        
        /* Memory access with bit-field operation */
        result += array[idx].field2;
        
        /* Modify through pointer with offset */
        struct bitfield_struct *ptr = &array[idx];
        ptr->field3 = (ptr->field3 + 1) & 0x7;
        
        /* Additional complex address calculation */
        result += ((struct bitfield_struct*)((char*)array + idx * sizeof(struct bitfield_struct)))->field4;
    }
    
    return result;
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    
    /* Initialize test data with some randomness */
    int data_size = 100;
    
    /* Allocate and initialize arrays */
    struct bitfield_struct *bitfields = 
        (struct bitfield_struct*)calloc(data_size, sizeof(struct bitfield_struct));
    
    char *chars = (char*)malloc(data_size * sizeof(char));
    short *shorts = (short*)malloc(data_size * sizeof(short));
    
    union type_pun *unions = (union type_pun*)malloc(data_size * sizeof(union type_pun));
    
    unsigned int *int_data = (unsigned int*)malloc(data_size * sizeof(unsigned int));
    int *indices = (int*)malloc(data_size * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < data_size; i++) {
        /* Initialize bitfields */
        bitfields[i].field1 = i & 0x1F;
        bitfields[i].field2 = (i * 3) & 0xFF;
        bitfields[i].field3 = (i >> 2) & 0x7;
        bitfields[i].field4 = i * 0x0101;
        
        /* Initialize char/short arrays */
        chars[i] = (char)(i & 0xFF);
        shorts[i] = (short)(i * 7);
        
        /* Initialize union data */
        unions[i].full = i * 0x11111111;
        
        /* Initialize integer data */
        int_data[i] = i * 0x12345678;
        
        /* Initialize indices */
        indices[i] = (i * 13) % data_size;
    }
    
    /* Test 1: ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    for (int i = 0; i < data_size; i++) {
        final_result ^= test_zero_extract_int(int_data[i], i & 0x1F);
        final_result += test_zero_extract_struct(&bitfields[i], i);
        final_result ^= test_explicit_bitfields(int_data[i]);
    }
    
    /* Test 2: STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    final_result += test_strict_low_part_chars(chars, shorts, data_size);
    
    volatile short volatile_short = 0;
    final_result += test_volatile_partial_writes(&volatile_short, 50);
    
    for (int i = 0; i < data_size; i++) {
        final_result ^= test_asm_strict_low_part(int_data[i]);
    }
    
    /* Test 3: SUBREG patterns */
    printf("Testing SUBREG patterns...\n");
    final_result += test_subreg_union(unions, data_size);
    final_result ^= test_subreg_casting(int_data, data_size);
    
    /* Test 4: Complex memory patterns */
    printf("Testing complex memory patterns...\n");
    final_result += test_complex_memory(bitfields, data_size, indices);
    
    /* Clean up */
    free(bitfields);
    free(chars);
    free(shorts);
    free(unions);
    free(int_data);
    free(indices);
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %u\n", final_result);
    
    /* Return non-zero to indicate success */
    return (final_result != 0) ? 0 : 1;
}
