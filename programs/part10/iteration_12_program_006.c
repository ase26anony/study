/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void escape(void*);

/* Global volatile variables to prevent optimization */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char* global_ptr = NULL;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int payload:12;
    unsigned int checksum:8;
    unsigned int footer:8;
} NOINLINE;

/* Union for bitfield access */
union bitfield_union {
    struct bitfield_packet bits;
    uint32_t raw;
};

NOINLINE uint32_t test_zero_extract(void) {
    VOLATILE_VAR union bitfield_union u;
    u.raw = 0xDEADBEEF;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    u.bits.header = 0xA;
    u.bits.payload = 0xBCD;
    u.bits.checksum = 0xEF;
    u.bits.footer = 0x12;
    
    /* Explicit bit extraction that may generate ZERO_EXTRACT */
    VOLATILE_VAR uint32_t val = u.raw;
    VOLATILE_VAR uint32_t extracted;
    
    /* Multiple extraction patterns */
    extracted = (val >> 4) & 0xFFF;        /* Extract payload */
    extracted |= ((val >> 16) & 0xFF) << 12; /* Extract checksum */
    extracted |= (val & 0xF) << 20;        /* Extract header */
    extracted |= ((val >> 24) & 0xFF) << 24; /* Extract footer */
    
    /* Complex extraction with variable shift */
    VOLATILE_VAR int shift = get_index() & 0x1F;
    extracted = (val >> shift) & ((1 << 8) - 1);
    
    return extracted + u.bits.payload;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE uint32_t test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_reg = 0x12345678;
    VOLATILE_VAR uint32_t result = 0;
    
    /* Union for byte access */
    union {
        uint32_t dword;
        uint8_t bytes[4];
    } converter;
    
    converter.dword = wide_reg;
    
    /* Byte-sized stores that may generate STRICT_LOW_PART */
    converter.bytes[0] = 0xFF;  /* Low byte store */
    converter.bytes[1] = 0xEE;  /* Second byte store */
    
    /* Pointer casting for byte access */
    *(volatile uint8_t*)&wide_reg = 0xAA;
    *((volatile uint8_t*)&wide_reg + 1) = 0xBB;
    
    /* Arithmetic truncation preserving high bits */
    VOLATILE_VAR uint32_t temp = wide_reg;
    temp = (temp & ~0xFF) | 0xCC;  /* Replace low byte only */
    
    /* Multiple operations that might need low-part handling */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        converter.bytes[i] = (converter.bytes[i] + i) & 0xFF;
    }
    
    /* Inline assembly for explicit low-byte access */
    uint32_t asm_out;
    uint32_t asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %b0\n\t"
        "movb %b1, %h0"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    result = converter.dword + temp + asm_out;
    return result;
}

/* ==================== SUBREG patterns ==================== */

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE uint32_t test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR uint32_t result = 0;
    
    /* Type punning between different sizes */
    VOLATILE_VAR uint32_t int_val = 0xDEADBEEF;
    VOLATILE_VAR uint16_t short_val;
    VOLATILE_VAR uint8_t byte_val;
    
    /* Explicit casts that may generate SUBREG */
    short_val = (uint16_t)int_val;
    byte_val = (uint8_t)(int_val >> 16);
    
    /* Vector element extraction */
    result += vec[0];  /* May generate SUBREG */
    result += vec[get_index() & 3];
    
    /* Mixed vector operations */
    short_vec[0] = (short)vec[0];
    short_vec[1] = (short)(vec[1] & 0xFFFF);
    
    /* Float/int bitcasting */
    VOLATILE_VAR float f = 3.14159f;
    VOLATILE_VAR uint32_t int_bits;
    memcpy(&int_bits, &f, sizeof(int_bits));
    
    /* Union for type punning */
    union {
        float f;
        uint32_t i;
        uint16_t s[2];
    } pun;
    
    pun.f = f;
    result += pun.s[0];  /* Access half of the float */
    result += pun.i;
    
    return result + short_val + byte_val;
}

/* ==================== Memory operand patterns ==================== */

/* Complex structure for memory addressing */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE uint32_t test_memory_operand(void) {
    /* Allocate memory with volatile pointers */
    VOLATILE_VAR struct nested* node1 = malloc(sizeof(struct nested));
    VOLATILE_VAR struct nested* node2 = malloc(sizeof(struct nested));
    VOLATILE_VAR struct nested* node3 = malloc(sizeof(struct nested));
    
    if (!node1 || !node2 || !node3) {
        return 0;
    }
    
    /* Initialize */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        node1->data[i] = i * 10;
        node2->data[i] = i * 20;
        node3->data[i] = i * 30;
    }
    
    node1->next = node2;
    node2->next = node3;
    node3->next = NULL;
    
    VOLATILE_VAR int result = 0;
    VOLATILE_VAR struct nested* current = node1;
    
    /* Complex pointer chasing with volatile index */
    VOLATILE_VAR int idx = global_index;
    
    /* Multi-level dereferencing */
    result += current->data[idx & 3];
    result += current->next->data[(idx + 1) & 3];
    result += current->next->next->data[(idx + 2) & 3];
    
    /* Array with volatile index */
    VOLATILE_VAR int* volatile array = node1->data;
    VOLATILE_VAR int volatile_index = get_index() & 3;
    result += array[volatile_index];
    result += array[volatile_index + 1];
    
    /* Triple pointer indirection */
    VOLATILE_VAR int*** triple_ptr = malloc(sizeof(int**));
    VOLATILE_VAR int** double_ptr = malloc(sizeof(int*));
    VOLATILE_VAR int* single_ptr = node1->data;
    
    if (triple_ptr && double_ptr) {
        *double_ptr = single_ptr;
        *triple_ptr = double_ptr;
        
        /* Complex memory access */
        result += ***triple_ptr;
        result += (*(*triple_ptr))[volatile_index];
        
        free(triple_ptr);
        free(double_ptr);
    }
    
    /* Cleanup */
    free(node1);
    free(node2);
    free(node3);
    
    return result;
}

/* ==================== Main function ==================== */

int main(void) {
    VOLATILE_VAR uint32_t checksum = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all tests */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_memory_operand();
    
    printf("Final checksum: %u (0x%08X)\n", checksum, checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* External function implementations */
int get_index(void) {
    static VOLATILE_VAR int counter = 0;
    return counter++ & 0x3;
}

void escape(void* ptr) {
    asm volatile ("" : : "r"(ptr) : "memory");
}
