/* test_resource_patterns.c */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimizations */
volatile int external_seed = 42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x) {
    unsigned int sum = 0;
    /* Multiple bit-field extractions with different widths */
    sum += (x >> 0) & 0x1F;      /* Extract bits 0-4 */
    sum += (x >> 5) & 0x3F;      /* Extract bits 5-10 (6 bits) */
    sum += (x >> 11) & 0x7FF;    /* Extract bits 11-21 (11 bits) */
    sum += (x >> 22) & 0x3FF;    /* Extract bits 22-31 (10 bits) */
    
    /* Combined mask and shift (common ZERO_EXTRACT pattern) */
    sum += (x & 0xFF00) >> 8;    /* Extract middle byte */
    sum += (x & 0xF0F0F0F0) >> 4; /* Complex scattered extraction */
    
    return sum;
}

/* Structure with bit-fields */
struct BitFieldStruct {
    unsigned int a : 3;
    unsigned int b : 5;
    unsigned int c : 8;
    unsigned int d : 16;
};

unsigned int test_zero_extract_struct(struct BitFieldStruct *s, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Bit-field reads generate ZERO_EXTRACT */
        sum += s[i].a;
        sum += s[i].b;
        sum += s[i].c;
        sum += s[i].d;
        
        /* Bit-field comparisons */
        if (s[i].a == 3) sum += 1;
        if (s[i].b > 10) sum += 2;
        if (s[i].c != 0) sum += s[i].c;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Partial register updates with char/short */
unsigned int test_strict_low_part_chars(char *data, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* char assignment in loop - may use partial register */
        char temp = data[i] + external_seed;
        sum += temp;
        
        /* volatile pointer to force strict low part store */
        volatile char *vptr = &data[i];
        *vptr = temp ^ 0x55;
    }
    
    return sum;
}

unsigned int test_strict_low_part_shorts(short *array, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* short operations that may use partial registers */
        short val = array[i];
        val = (val * 3 + 1) & 0x7FFF;
        
        /* Store through volatile short pointer */
        volatile short *vs = &array[i];
        *vs = val;
        
        sum += val;
    }
    
    return sum;
}

/* Inline assembly for byte register operations */
unsigned int test_strict_low_part_asm(void) {
    unsigned int result = 0;
    
    /* Force byte register operations */
    for (int i = 0; i < 4; i++) {
        unsigned char byte_val = (external_seed + i) & 0xFF;
        
        /* Inline assembly that operates on byte register */
        __asm__ volatile (
            "addb %1, %0"
            : "+q"(byte_val)  /* q constraint = byte register */
            : "r"(i)
            : "cc"
        );
        
        result += byte_val;
    }
    
    return result;
}

/* ========== SUBREG patterns ========== */

/* Union for type-punning */
union TypePun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
};

unsigned int test_subreg_union(union TypePun *u, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Access different views of same data - generates SUBREG */
        sum += u[i].halves[0];      /* Low 16 bits */
        sum += u[i].halves[1];      /* High 16 bits */
        sum += u[i].bytes[2];       /* Third byte */
        
        /* Modify through one view, read through another */
        u[i].halves[0] = (u[i].halves[0] + sum) & 0xFFFF;
        sum += u[i].full;           /* Read full 32-bit */
    }
    
    return sum;
}

/* Casting between different integer sizes */
unsigned int test_subreg_casts(int *ints, short *shorts, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Cast int to short - may generate SUBREG */
        short s = (short)(ints[i] & 0xFFFF);
        sum += s;
        
        /* Cast short to int with sign extension */
        int extended = (int)shorts[i];
        sum += extended & 0xFF;
        
        /* Store back through different type pointer */
        *(volatile short*)&shorts[i] = s ^ 0xAA;
    }
    
    return sum;
}

/* ========== Complex memory references ========== */

/* Memory references with addressing modes */
unsigned int test_complex_memory(int *base, int *offsets, int count) {
    unsigned int sum = 0;
    
    for (int i = 0; i < count; i++) {
        /* Complex address calculation */
        int *ptr = base + offsets[i % 8];
        
        /* Access with offset - creates MEM with complex address */
        int val = ptr[external_seed & 3];
        
        /* Bit-field extract from memory value */
        sum += (val >> (i & 0xF)) & 0xFF;
        
        /* Partial write to memory location */
        *(volatile short*)ptr = (sum & 0xFFFF);
    }
    
    return sum;
}

/* ========== Main test driver ========== */

int main(void) {
    unsigned int total_sum = 0;
    const int TEST_SIZE = 100;
    
    /* Initialize test data */
    struct BitFieldStruct bf_array[TEST_SIZE];
    union TypePun unions[TEST_SIZE];
    int int_array[TEST_SIZE * 2];
    short short_array[TEST_SIZE * 2];
    char char_array[TEST_SIZE];
    int offsets[8] = {0, 1, 3, 7, 15, 31, 63, 127};
    
    /* Seed with pseudo-random but deterministic values */
    for (int i = 0; i < TEST_SIZE; i++) {
        /* Bit-field struct */
        bf_array[i].a = (i * 3) & 0x7;
        bf_array[i].b = (i * 5) & 0x1F;
        bf_array[i].c = (i * 7) & 0xFF;
        bf_array[i].d = (i * 11) & 0xFFFF;
        
        /* Union */
        unions[i].full = i * 0x1234567;
        
        /* Arrays */
        int_array[i] = i * 0xABCD;
        short_array[i] = i * 0xEF;
        char_array[i] = (i * 13) & 0xFF;
    }
    
    /* Run all tests */
    total_sum += test_zero_extract_int(external_seed * 0x987654);
    total_sum += test_zero_extract_struct(bf_array, TEST_SIZE / 2);
    total_sum += test_strict_low_part_chars(char_array, TEST_SIZE);
    total_sum += test_strict_low_part_shorts(short_array, TEST_SIZE);
    total_sum += test_strict_low_part_asm();
    total_sum += test_subreg_union(unions, TEST_SIZE / 4);
    total_sum += test_subreg_casts(int_array, short_array, TEST_SIZE / 2);
    total_sum += test_complex_memory(int_array, offsets, TEST_SIZE / 8);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %u\n", total_sum);
    
    /* Return non-zero result for scripting */
    return (total_sum & 0xFF) != 0 ? 0 : 1;
}
