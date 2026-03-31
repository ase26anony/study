/* test_resource_patterns.c
 * Designed to trigger specific RTL patterns in GCC's resource.cc:
 * - ZERO_EXTRACT for bit-field operations
 * - STRICT_LOW_PART for partial register updates
 * - SUBREG for register sub-parts
 * - Complex memory references
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent constant folding and dead code elimination */
volatile int g_volatile_seed = 42;

/* ==================== ZERO_EXTRACT PATTERNS ==================== */

/* Bit-field extraction from integer */
unsigned int test_zero_extract_int(unsigned int x, unsigned int shift) {
    /* Multiple bit-field extractions that should generate ZERO_EXTRACT */
    unsigned int result = 0;
    
    /* Pattern 1: Explicit mask and shift */
    result += (x & 0xFF00) >> 8;           /* Extract bits 8-15 */
    
    /* Pattern 2: Variable shift with mask */
    unsigned int mask = (1 << 5) - 1;      /* 5-bit mask */
    result += (x >> shift) & mask;         /* Extract variable bit-field */
    
    /* Pattern 3: Nested extractions */
    result += ((x >> 3) & 0x1F) + ((x >> 10) & 0x3F);
    
    /* Pattern 4: Conditional extraction */
    if (x & 0x80000000) {
        result += (x >> 16) & 0xFFFF;
    } else {
        result += (x >> 8) & 0xFF;
    }
    
    return result;
}

/* Bit-field structure operations */
struct bitfield_struct {
    unsigned int field1 : 5;
    unsigned int field2 : 7;
    unsigned int field3 : 10;
    unsigned int field4 : 3;
    unsigned int field5 : 7;
};

unsigned int test_zero_extract_struct(struct bitfield_struct *s, unsigned int val) {
    unsigned int result = 0;
    
    /* Bit-field assignments */
    s->field1 = val & 0x1F;
    s->field2 = (val >> 5) & 0x7F;
    s->field3 = (val >> 12) & 0x3FF;
    
    /* Bit-field comparisons and arithmetic */
    if (s->field1 == 10) {
        result += s->field2;
    }
    
    result += s->field3 * 2;
    result += s->field4 << 1;
    
    /* Complex bit-field expression */
    result += ((s->field1 << 3) | s->field2) & 0xFF;
    
    return result;
}

/* ==================== STRICT_LOW_PART PATTERNS ==================== */

/* Partial register updates with small integer types */
unsigned int test_strict_low_part(volatile short *mem, int count) {
    unsigned int result = 0;
    int i;
    
    /* Pattern 1: char/short operations in loops */
    for (i = 0; i < count; i++) {
        short temp = mem[i];
        temp = (temp + g_volatile_seed) & 0x7FFF;  /* Keep in short range */
        result += temp;
        
        /* Partial write to memory */
        mem[i] = temp & 0xFF;  /* Only write low byte */
    }
    
    /* Pattern 2: Type demotion with arithmetic */
    int large_val = result * 3;
    short demoted = (short)(large_val & 0xFFFF);  /* Explicit truncation */
    result += demoted;
    
    /* Pattern 3: Pointer to small type */
    volatile char *byte_ptr = (volatile char *)mem;
    for (i = 0; i < 8; i++) {
        byte_ptr[i] = (result >> (i * 4)) & 0xF;  /* Write nibbles */
    }
    
    return result;
}

/* Inline assembly for partial register access (x86-specific) */
#ifdef __x86_64__
unsigned int test_strict_low_part_asm(unsigned int x) {
    unsigned short low_part;
    
    /* Assembly that operates on partial register */
    __asm__ volatile (
        "movw %1, %%ax\n\t"          /* Load low 16 bits */
        "addw $0x1234, %%ax\n\t"     /* Operate on low part */
        "movw %%ax, %0\n\t"          /* Store back */
        : "=r" (low_part)
        : "r" (x)
        : "%ax"
    );
    
    return low_part + (x >> 16);
}
#else
unsigned int test_strict_low_part_asm(unsigned int x) {
    /* Fallback for non-x86 */
    return (x & 0xFFFF) + 0x1234 + (x >> 16);
}
#endif

/* ==================== SUBREG PATTERNS ==================== */

/* Union for type-punning */
union type_pun {
    uint32_t full;
    uint16_t halves[2];
    uint8_t bytes[4];
    struct {
        uint16_t low;
        uint16_t high;
    } parts;
};

unsigned int test_subreg_union(union type_pun *u, unsigned int val) {
    unsigned int result = 0;
    
    u->full = val;
    
    /* Access sub-parts through different views */
    result += u->halves[0];          /* Low 16 bits */
    result += u->halves[1] << 8;     /* High 16 bits */
    
    /* Byte-wise access */
    for (int i = 0; i < 4; i++) {
        result += u->bytes[i] * (i + 1);
    }
    
    /* Structure member access */
    result += u->parts.low * 3;
    result += u->parts.high / 2;
    
    /* Cast between different sizes */
    uint16_t truncated = (uint16_t)(result & 0xFFFF);
    result = truncated + (result >> 16);
    
    return result;
}

/* SIMD-like operations using subregs */
unsigned int test_subreg_simd(unsigned int a, unsigned int b) {
    /* Treat 32-bit as packed 16-bit values */
    unsigned int result = 0;
    
    /* Extract and operate on 16-bit parts */
    unsigned short a_low = a & 0xFFFF;
    unsigned short a_high = (a >> 16) & 0xFFFF;
    unsigned short b_low = b & 0xFFFF;
    unsigned short b_high = (b >> 16) & 0xFFFF;
    
    result = (a_low * b_low) + (a_high * b_high);
    
    /* Re-pack results */
    result = (result & 0xFFFF) | ((result & 0xFFFF0000) << 16);
    
    return result;
}

/* ==================== COMPLEX MEMORY REFERENCES ==================== */

/* Array operations with complex addressing */
unsigned int test_complex_memory(int *array, int size, int stride) {
    unsigned int result = 0;
    volatile int *volatile_ptr = array;
    
    /* Complex addressing modes */
    for (int i = 0; i < size; i += stride) {
        /* Multiple memory references with indexing */
        result += array[i];
        result += array[i + 1] * 2;
        result += array[(i * 3) % size] / 3;
        
        /* Pointer arithmetic */
        int *ptr = array + i;
        result += ptr[0] + ptr[stride];
        
        /* Volatile access prevents optimization */
        result += *volatile_ptr;
        volatile_ptr += stride;
    }
    
    /* Structure with bit-fields in array */
    struct bitfield_struct bf_array[4];
    for (int i = 0; i < 4; i++) {
        bf_array[i].field1 = (result >> (i * 4)) & 0x1F;
        bf_array[i].field2 = (array[i % size] >> 3) & 0x7F;
        result += bf_array[i].field1 + bf_array[i].field2;
    }
    
    return result;
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(int argc, char **argv) {
    unsigned int final_result = 0;
    int i;
    
    /* Initialize test data */
    int data_array[64];
    short short_array[32];
    union type_pun pun_union;
    struct bitfield_struct bf_struct = {0};
    
    /* Initialize with pseudo-random data */
    for (i = 0; i < 64; i++) {
        data_array[i] = (i * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    for (i = 0; i < 32; i++) {
        short_array[i] = (short)((i * 214013 + 2531011) & 0x7FFF);
    }
    
    /* Test 1: ZERO_EXTRACT patterns */
    printf("Testing ZERO_EXTRACT patterns...\n");
    for (i = 0; i < 16; i++) {
        final_result ^= test_zero_extract_int(data_array[i], i & 7);
        final_result += test_zero_extract_struct(&bf_struct, data_array[i]);
    }
    
    /* Test 2: STRICT_LOW_PART patterns */
    printf("Testing STRICT_LOW_PART patterns...\n");
    final_result += test_strict_low_part(short_array, 16);
    final_result += test_strict_low_part_asm(final_result);
    
    /* Test 3: SUBREG patterns */
    printf("Testing SUBREG patterns...\n");
    for (i = 0; i < 8; i++) {
        final_result += test_subreg_union(&pun_union, data_array[i]);
        final_result += test_subreg_simd(data_array[i], data_array[i + 1]);
    }
    
    /* Test 4: Complex memory references */
    printf("Testing complex memory references...\n");
    final_result += test_complex_memory(data_array, 64, 3);
    
    /* Use result to affect control flow */
    if (final_result & 1) {
        printf("Odd result: %u\n", final_result);
    } else {
        printf("Even result: %u\n", final_result);
    }
    
    /* Return result to prevent dead code elimination */
    return (int)(final_result & 0x7FFFFFFF);
}
