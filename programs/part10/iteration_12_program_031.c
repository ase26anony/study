/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void use_value(int val);

/* Global volatile variables to prevent optimization */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char global_byte = 0;
VOLATILE_VAR int *global_ptr = NULL;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header: 4;
    unsigned int data: 12;
    unsigned int footer: 8;
    unsigned int checksum: 8;
} NOINLINE;

/* Union for bitfield access */
union packet_union {
    struct bitfield_packet fields;
    uint32_t raw;
};

NOINLINE static uint32_t test_zero_extract(void) {
    union packet_union pkt;
    VOLATILE_VAR uint32_t temp;
    
    /* Initialize */
    pkt.raw = 0xDEADBEEF;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    pkt.fields.header = 0xA;
    pkt.fields.data = 0xBCD;
    pkt.fields.footer = 0xEF;
    pkt.fields.checksum = (pkt.fields.header ^ pkt.fields.data ^ pkt.fields.footer) & 0xFF;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    temp = pkt.raw;
    
    /* Multiple extraction patterns */
    uint32_t extracted1 = (temp >> 4) & 0xFFF;      /* Extract 12-bit data field */
    uint32_t extracted2 = (temp >> 16) & 0xFF;      /* Extract footer */
    uint32_t extracted3 = (temp >> 24) & 0xFF;      /* Extract checksum */
    
    /* Complex extraction with variable shift */
    int shift = global_index & 0x7;
    uint32_t extracted4 = (temp >> shift) & ((1 << 12) - 1);
    
    /* Return checksum to ensure side effects */
    return pkt.fields.checksum + extracted1 + extracted2 + extracted3 + extracted4;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE static int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_reg = 0x12345678;
    VOLATILE_VAR uint16_t half_reg;
    VOLATILE_VAR uint8_t byte_reg;
    int result = 0;
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    *(volatile uint8_t *)&wide_reg = 0xFF;
    result += wide_reg;
    
    /* Another byte store */
    ((volatile uint8_t *)&wide_reg)[1] = 0xAA;
    result += wide_reg;
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } pun;
    pun.full = 0x87654321;
    
    /* Store low byte only */
    pun.bytes[0] = global_byte;
    result += pun.full;
    
    /* Arithmetic truncation that preserves high bits */
    uint32_t source = 0x89ABCDEF;
    uint8_t low_byte = source & 0xFF;  /* May generate STRICT_LOW_PART on some archs */
    result += low_byte;
    
    /* Inline assembly forcing low-part access (x86-specific) */
    #if defined(__i386__) || defined(__x86_64__)
    uint32_t asm_in = 0xDEADBEEF;
    uint8_t asm_out;
    asm volatile (
        "movb %%al, %0\n\t"
        : "=r" (asm_out)
        : "a" (asm_in)
        : 
    );
    result += asm_out;
    #endif
    
    return result;
}

/* ==================== SUBREG patterns ==================== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    int result = 0;
    
    /* Vector element extraction - may generate SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    result += elem0 + elem2;
    
    /* Short vector element extraction */
    short elem1 = short_vec[1];
    short elem5 = short_vec[5];
    result += elem1 + elem5;
    
    /* Type punning through casts */
    float float_val = 3.14159f;
    uint32_t int_val;
    
    /* Bitcast through union */
    union {
        float f;
        uint32_t i;
    } converter;
    converter.f = float_val;
    int_val = converter.i;
    result += int_val;
    
    /* Direct cast (may generate SUBREG) */
    double dbl = 2.71828;
    uint64_t dbl_bits = *(uint64_t *)&dbl;
    result += (int)(dbl_bits & 0xFFFFFFFF);
    
    /* Mixed size operations */
    uint16_t short_val = 0xABCD;
    uint32_t extended = (uint32_t)short_val;  /* Zero extension */
    uint32_t truncated = (uint16_t)extended;  /* Truncation */
    result += truncated;
    
    return result;
}

/* ==================== Memory operand patterns ==================== */

NOINLINE static int test_memory_operand(void) {
    /* Complex memory addressing structures */
    static VOLATILE_VAR int array1[100];
    static VOLATILE_VAR int array2[100];
    VOLATILE_VAR int ***triple_ptr;
    VOLATILE_VAR int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 100; i++) {
        array1[i] = i * 2;
        array2[i] = i * 3;
    }
    
    /* Multi-level pointer dereference */
    int **double_ptr;
    int *single_ptr;
    int value = 42;
    
    single_ptr = &value;
    double_ptr = &single_ptr;
    triple_ptr = &double_ptr;
    
    /* Complex addressing: ***triple_ptr */
    result += ***triple_ptr;
    
    /* Array indexing with volatile index */
    VOLATILE_VAR int idx = global_index % 100;
    result += array1[idx] + array2[idx * 2 % 100];
    
    /* Structure with nested arrays */
    struct nested_mem {
        int data[10][10];
        int *ptr_array[5];
    };
    
    static VOLATILE_VAR struct nested_mem nested;
    
    /* Initialize nested structure */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            nested.data[i][j] = i * 10 + j;
        }
    }
    
    /* Complex nested array access */
    idx = (global_index * 7) % 10;
    result += nested.data[idx][idx * 3 % 10];
    
    /* Pointer chasing */
    int *ptr_chain[5];
    for (int i = 0; i < 5; i++) {
        ptr_chain[i] = &array1[i * 20];
    }
    
    int chain_idx = global_index % 5;
    result += *ptr_chain[chain_idx];
    
    /* Volatile memory operations */
    VOLATILE_VAR int *volatile_ptr = array2;
    result += *(volatile_ptr + 10);
    result += *(volatile_ptr + 20);
    
    return result;
}

/* ==================== Main test driver ==================== */

int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing GCC resource patterns...\n");
    
    /* Test each pattern */
    checksum += test_zero_extract();
    printf("  Zero-extract test completed\n");
    
    checksum += test_strict_low_part();
    printf("  Strict-low-part test completed\n");
    
    checksum += test_subreg();
    printf("  Subreg test completed\n");
    
    checksum += test_memory_operand();
    printf("  Memory operand test completed\n");
    
    printf("Final checksum: %u\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Dummy external functions to prevent optimization */
int get_index(void) {
    return rand() % 100;
}

void use_value(int val) {
    /* Do nothing, just prevent optimization */
    asm volatile ("" : : "r"(val));
}
