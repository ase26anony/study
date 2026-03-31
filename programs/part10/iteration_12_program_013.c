/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int opaque(int x);
extern void* opaque_ptr(void* p);

/* Global volatile variables to prevent optimization */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR int global_mask = 0xFF;
VOLATILE_VAR char global_byte = 0x42;

/* ========== ZERO_EXTRACT patterns ========== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int data:12;
    unsigned int flags:8;
    unsigned int checksum:8;
} NOINLINE;

/* Another bitfield with different layout */
struct control_reg {
    unsigned int enable:1;
    unsigned int mode:3;
    unsigned int reserved:4;
    unsigned int value:8;
    unsigned int status:16;
} NOINLINE;

NOINLINE int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packet packet;
    VOLATILE_VAR struct control_reg ctrl;
    VOLATILE_VAR unsigned int raw_value;
    int result = 0;
    
    /* Initialize with opaque values */
    raw_value = opaque(0x12345678);
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    packet.header = (raw_value >> 28) & 0xF;  /* Extract bits 28-31 */
    packet.data = (raw_value >> 16) & 0xFFF;  /* Extract bits 16-27 */
    packet.flags = (raw_value >> 8) & 0xFF;   /* Extract bits 8-15 */
    packet.checksum = raw_value & 0xFF;       /* Extract bits 0-7 */
    
    /* More complex extraction with variable shift */
    ctrl.enable = (raw_value >> global_index) & 0x1;
    ctrl.mode = (raw_value >> (global_index + 1)) & 0x7;
    ctrl.value = (raw_value >> 4) & ((1 << 8) - 1);
    
    /* Extract and combine multiple bitfields */
    result = (packet.data << 4) | packet.header;
    result |= (ctrl.mode << 12);
    
    /* Manual bit extraction that may also generate ZERO_EXTRACT */
    unsigned int extracted = (raw_value >> 5) & 0x3FF;  /* 10-bit extract */
    result ^= extracted;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Union for type punning */
union reg_access {
    uint32_t full;
    uint8_t bytes[4];
    uint16_t halves[2];
} NOINLINE;

NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t reg = 0xDEADBEEF;
    VOLATILE_VAR union reg_access ra;
    VOLATILE_VAR uint8_t *byte_ptr;
    int result = 0;
    
    /* Initialize union */
    ra.full = opaque(0x12345678);
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&reg = global_byte;  /* Store byte, preserve high bits */
    
    /* Another byte store with pointer arithmetic */
    byte_ptr = (volatile uint8_t*)&reg + 1;
    *byte_ptr = 0xAB;
    
    /* Access low byte via union */
    ra.bytes[0] = 0xCD;  /* Modify only low byte */
    
    /* Truncation operation that preserves high bits in source */
    uint32_t temp = opaque(0x87654321);
    uint8_t low_byte = temp & 0xFF;  /* Extract low byte */
    ra.bytes[1] = low_byte;
    
    /* Half-word access */
    ra.halves[0] = 0x1234;  /* Modify low 16 bits */
    
    /* Inline assembly forcing low-part register access */
    uint32_t in_val = 0x89ABCDEF;
    uint32_t out_val;
    
    /* x86 assembly accessing low byte of register */
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (out_val)
        : "r" (in_val)
        : "cc"
    );
    
    result = reg ^ ra.full ^ out_val;
    return result;
}

/* ========== SUBREG patterns ========== */

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR int scalar = 0;
    VOLATILE_VAR float f = 3.14159f;
    int result = 0;
    
    /* Type punning between int and float - may generate SUBREG */
    uint32_t int_bits;
    memcpy(&int_bits, &f, sizeof(int_bits));
    result = int_bits;
    
    /* Cast between different integer sizes */
    int32_t i32 = opaque(0x12345678);
    int16_t i16 = (int16_t)i32;  /* Truncation to 16-bit */
    result ^= i16;
    
    /* Vector element extraction - often uses SUBREG */
    scalar = vec[global_index % 4];  /* Extract element */
    result += scalar;
    
    /* Another vector extraction with different size */
    short s = short_vec[global_index % 8];
    result += s;
    
    /* Structure with mixed types */
    struct mixed {
        int a;
        short b;
        char c;
    } m;
    
    m.a = opaque(100);
    m.b = (short)m.a;  /* SUBREG for size change */
    m.c = (char)m.b;   /* Another SUBREG */
    
    result += m.c;
    
    return result;
}

/* ========== Memory operand patterns ========== */

/* Complex structure for memory addressing */
struct node {
    int data;
    struct node* next;
    struct node* prev;
    int array[8];
} NOINLINE;

NOINLINE int test_memory_operand(void) {
    /* Create a small linked structure */
    VOLATILE_VAR struct node nodes[4];
    VOLATILE_VAR int*** triple_ptr;
    VOLATILE_VAR int** double_ptr;
    VOLATILE_VAR int* single_ptr;
    VOLATILE_VAR int buffer[64];
    int result = 0;
    
    /* Initialize nodes */
    for (int i = 0; i < 4; i++) {
        nodes[i].data = opaque(i * 100);
        nodes[i].next = (i < 3) ? &nodes[i + 1] : NULL;
        nodes[i].prev = (i > 0) ? &nodes[i - 1] : NULL;
        for (int j = 0; j < 8; j++) {
            nodes[i].array[j] = opaque(i * 10 + j);
        }
    }
    
    /* Complex pointer chasing - forces walking memory addresses */
    struct node* current = &nodes[0];
    for (int i = 0; i < 3; i++) {
        if (current && current->next) {
            /* Access through multiple levels */
            result += current->data;
            result += current->next->data;  /* Multi-level access */
            current = current->next;
        }
    }
    
    /* Triple pointer indirection */
    int x = 42;
    int* p1 = &x;
    int** p2 = &p1;
    triple_ptr = &p2;
    
    /* Volatile access through multiple indirections */
    result += ***triple_ptr;
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index;
    for (int i = 0; i < 16; i++) {
        buffer[(idx + i) % 64] = opaque(i);  /* Non-constant offset */
    }
    
    /* Access with multiple array dimensions */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = opaque(i * 8 + j);
            result += matrix[i][j];  /* Complex addressing: base + i*stride + j */
        }
    }
    
    /* Structure field with offset */
    result += nodes[2].array[global_index % 8];
    
    return result;
}

/* ========== Main test driver ========== */

/* Opaque function definitions to prevent optimization */
int opaque(int x) {
    static VOLATILE_VAR int counter = 0;
    return x ^ (counter++);
}

void* opaque_ptr(void* p) {
    static VOLATILE_VAR intptr_t offset = 0;
    return (void*)((intptr_t)p + (offset++ & 0xFF));
}

int main(void) {
    int total = 0;
    
    printf("Testing resource pattern generation...\n");
    
    /* Run all tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    /* Mix in some global variable usage */
    total += global_index;
    total += global_mask;
    total += global_byte;
    
    printf("Result checksum: %d\n", total);
    printf("(Non-zero indicates all tests executed)\n");
    
    return total != 0 ? 0 : 1;
}
