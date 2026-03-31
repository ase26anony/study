/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory references
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ========== ZERO_EXTRACT PATTERNS ========== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 3;
    unsigned int field4 : 17;
};

/* Explicit bit-field extraction */
unsigned int test_zero_extract_bitfield(struct bitfield_struct *bf, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* These operations should generate ZERO_EXTRACT RTL */
        unsigned int val1 = bf->field1;  /* Bit-field read */
        unsigned int val2 = bf->field2;
        unsigned int val3 = bf->field3;
        
        /* Bit-field assignment (write) */
        bf->field1 = (val1 + i) & 0x1F;  /* Mask to 5 bits */
        bf->field2 = (val2 * 2) & 0x7F;  /* Mask to 7 bits */
        bf->field3 = (val3 ^ i) & 0x07;  /* Mask to 3 bits */
        
        /* Bit-field comparison */
        if (bf->field1 == (unsigned int)(i & 0x1F)) {
            sum += bf->field2;
        }
        
        /* Explicit mask and shift (another ZERO_EXTRACT pattern) */
        unsigned int combined = (bf->field4 & 0x1FFFF) >> bf->field3;
        sum += combined;
    }
    
    return sum;
}

/* Manual bit-field operations */
unsigned int test_zero_extract_manual(unsigned int *array, int size) {
    unsigned int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Various bit extraction patterns */
        unsigned int val = array[i] + g_volatile_seed;
        
        /* Pattern 1: Extract 8-bit field from position 8 */
        unsigned int field1 = (val >> 8) & 0xFF;
        
        /* Pattern 2: Extract 6-bit field from position 2 */
        unsigned int field2 = (val >> 2) & 0x3F;
        
        /* Pattern 3: Extract variable width field */
        unsigned int width = (val & 0x7) + 1;  /* 1-8 bits */
        unsigned int mask = (1u << width) - 1;
        unsigned int field3 = (val >> 16) & mask;
        
        /* Pattern 4: Nested extractions */
        unsigned int temp = (field1 & 0x0F) | ((field2 & 0x0F) << 4);
        unsigned int field4 = (temp >> 2) & 0x3F;
        
        sum += field1 + field2 + field3 + field4;
        
        /* Write back extracted bits to different positions */
        array[i] = (field1 << 24) | (field2 << 16) | (field3 << 8) | field4;
    }
    
    return sum;
}

/* ========== STRICT_LOW_PART PATTERNS ========== */

/* Partial register updates */
unsigned int test_strict_low_part(int iterations) {
    volatile short vs;  /* volatile to prevent optimization */
    volatile char vc;
    unsigned int sum = 0;
    
    /* Use function parameters to force register allocation */
    int param1 = g_volatile_seed;
    short param2 = (short)(g_volatile_seed * 2);
    char param3 = (char)(g_volatile_seed + 1);
    
    for (int i = 0; i < iterations; i++) {
        /* These should generate STRICT_LOW_PART for partial register writes */
        
        /* char assignment in register context */
        char c = (char)(param1 + i);
        vc = c;  /* Partial write to char */
        sum += (unsigned int)vc;
        
        /* short assignment in register context */
        short s = (short)(param2 - i);
        vs = s;  /* Partial write to short */
        sum += (unsigned int)vs;
        
        /* Mixed-size operations */
        int temp = param1 * i;
        short partial = (short)(temp & 0xFFFF);
        vs = partial;  /* Writing only low 16 bits */
        sum += partial;
        
        /* Pointer to volatile short (might generate strict low part store) */
        volatile short *ps = &vs;
        *ps = (short)(*ps + 1);  /* Increment through pointer */
        
        /* Function call with small types to force register partial updates */
        param3 = (char)(param3 + c);
        param2 = (short)(param2 + s);
    }
    
    return sum;
}

/* Inline assembly for explicit partial register access */
unsigned int test_strict_low_part_asm(int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        unsigned int val = g_volatile_seed + i;
        
        /* Inline assembly that operates on partial registers */
        unsigned char byte_val;
        unsigned short word_val;
        
        /* Byte operation - may generate STRICT_LOW_PART */
        asm volatile (
            "movb %1, %0\n\t"
            : "=q"(byte_val)    /* q = a, b, c, or d register (byte-addressable) */
            : "r"((unsigned char)val)
            : "cc"
        );
        
        /* Word operation */
        asm volatile (
            "movw %1, %0\n\t"
            : "=r"(word_val)    /* word-sized register */
            : "r"((unsigned short)val)
            : "cc"
        );
        
        sum += byte_val + word_val;
    }
    
    return sum;
}

/* ========== SUBREG PATTERNS ========== */

/* Union for type-punning (SUBREG patterns) */
union type_punner {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

unsigned int test_subreg_union(int iterations) {
    union type_punner u;
    unsigned int sum = 0;
    
    u.full = g_volatile_seed;
    
    for (int i = 0; i < iterations; i++) {
        /* These accesses should generate SUBREG RTL */
        
        /* Access 16-bit halves */
        u.halves[0] = (uint16_t)(u.halves[0] + i);
        u.halves[1] = (uint16_t)(u.halves[1] - i);
        
        /* Access individual bytes */
        u.bytes[0] ^= (uint8_t)i;
        u.bytes[2] += (uint8_t)(i >> 1);
        
        /* Access through struct view */
        u.parts.low = u.parts.low * 3;
        u.parts.high = u.parts.high / 2;
        
        /* Cast between different sizes */
        uint16_t temp16 = (uint16_t)u.full;  /* Low 16 bits */
        uint8_t temp8 = (uint8_t)(u.full >> 16);  /* High byte of high word */
        
        sum += temp16 + temp8 + u.bytes[1] + u.parts.low;
        
        /* Update full value */
        u.full = (u.full * 1103515245u + 12345u) & 0x7FFFFFFFu;
    }
    
    return sum;
}

/* SIMD-like operations using unions */
unsigned int test_subreg_simd(int iterations) {
    /* Packed data in registers */
    union {
        uint64_t dword;
        uint32_t words[2];
        uint16_t shorts[4];
    } data;
    
    unsigned int sum = 0;
    data.dword = (uint64_t)g_volatile_seed << 32 | g_volatile_seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Extract and manipulate sub-parts */
        
        /* Access 32-bit parts */
        data.words[0] += i;
        data.words[1] ^= i;
        
        /* Access 16-bit parts */
        for (int j = 0; j < 4; j++) {
            data.shorts[j] = (uint16_t)(data.shorts[j] * (j + 1));
            sum += data.shorts[j];
        }
        
        /* Cross-part operations */
        uint32_t mixed = (uint32_t)data.shorts[0] | ((uint32_t)data.shorts[1] << 16);
        data.words[0] = mixed ^ data.words[1];
        
        /* Partial updates with casting */
        uint16_t temp = (uint16_t)data.words[0];  /* Low 16 bits */
        data.shorts[2] = temp;
        data.shorts[3] = (uint16_t)(data.words[0] >> 16);  /* High 16 bits */
    }
    
    return sum;
}

/* ========== COMPLEX MEMORY REFERENCES ========== */

/* Structure with mixed types for complex addressing */
struct complex_mem {
    int data[16];
    short shorts[32];
    char bytes[64];
    struct bitfield_struct bitfields[4];
};

unsigned int test_complex_memory(struct complex_mem *cm, int iterations) {
    unsigned int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int idx = (i * g_volatile_seed) & 0xF;
        int idx2 = (i * 3) & 0x1F;
        int idx3 = (i * 5) & 0x3F;
        
        /* Complex addressing modes */
        cm->data[idx] += cm->shorts[idx2] * cm->bytes[idx3];
        
        /* Bit-field in array */
        cm->bitfields[idx & 3].field1 = (cm->bitfields[idx & 3].field1 + i) & 0x1F;
        cm->bitfields[idx & 3].field2 = (cm->bytes[idx3] & 0x7F);
        
        /* Pointer arithmetic with different types */
        short *sptr = &cm->shorts[idx2];
        *sptr = (short)(*sptr + cm->data[idx]);
        
        char *cptr = &cm->bytes[idx3];
        *cptr = (char)(*cptr ^ (i & 0xFF));
        
        /* Multi-dimensional indexing */
        sum += cm->data[idx] + *sptr + *cptr + cm->bitfields[idx & 3].field1;
        
        /* Update volatile to prevent optimization */
        g_volatile_seed = (g_volatile_seed * 1664525u + 1013904223u) & 0x7FFFFFFFu;
    }
    
    return sum;
}

/* ========== MAIN TEST DRIVER ========== */

int main(int argc, char **argv) {
    /* Use command line or default iterations */
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    if (iterations < 10) iterations = 1000;
    
    unsigned int final_sum = 0;
    
    /* Initialize test data */
    struct bitfield_struct bf = {1, 2, 3, 1000};
    unsigned int array[100];
    struct complex_mem cm;
    
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3 + g_volatile_seed;
    }
    
    for (int i = 0; i < 16; i++) cm.data[i] = i;
    for (int i = 0; i < 32; i++) cm.shorts[i] = (short)i;
    for (int i = 0; i < 64; i++) cm.bytes[i] = (char)i;
    for (int i = 0; i < 4; i++) {
        cm.bitfields[i].field1 = i;
        cm.bitfields[i].field2 = i * 2;
        cm.bitfields[i].field3 = i & 0x07;
        cm.bitfields[i].field4 = i * 100;
    }
    
    /* Run all tests */
    final_sum += test_zero_extract_bitfield(&bf, iterations);
    final_sum += test_zero_extract_manual(array, 100);
    final_sum += test_strict_low_part(iterations / 10);
    final_sum += test_strict_low_part_asm(iterations / 20);
    final_sum += test_subreg_union(iterations);
    final_sum += test_subreg_simd(iterations / 2);
    final_sum += test_complex_memory(&cm, iterations);
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %u\n", final_sum);
    
    /* Return non-zero result for scripting */
    return (final_sum != 0) ? 0 : 1;
}
