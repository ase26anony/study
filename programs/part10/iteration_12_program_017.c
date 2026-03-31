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
extern void* get_ptr(void);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int payload:12;
    unsigned int flags:8;
    unsigned int checksum:8;
} NOINLINE;

/* Another bitfield with different layout */
struct control_reg {
    unsigned int enable:1;
    unsigned int mode:3;
    unsigned int data:20;
    unsigned int status:8;
};

NOINLINE unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packet packet;
    VOLATILE_VAR struct control_reg ctrl;
    unsigned int result = 0;
    
    /* Initialize */
    packet.header = 0xA;
    packet.payload = 0xABC;
    packet.flags = 0x3F;
    packet.checksum = 0x55;
    
    ctrl.enable = 1;
    ctrl.mode = 5;
    ctrl.data = 0x12345;
    ctrl.status = 0xAA;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    result = packet.payload;          /* Extract bitfield */
    packet.flags = result & 0x0F;     /* Store to bitfield */
    
    /* Explicit bit extraction that may use ZERO_EXTRACT */
    unsigned int raw = 0xDEADBEEF;
    unsigned int extracted = (raw >> 8) & 0xFFFF;  /* Extract middle 16 bits */
    
    /* Complex bitfield manipulation */
    ctrl.data = (extracted << 4) | (packet.header & 0x7);
    
    /* Multiple extractions */
    unsigned int mask = (1 << 12) - 1;
    unsigned int masked = ctrl.data & mask;
    
    return result + extracted + masked + ctrl.status;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0x12345678;
    VOLATILE_VAR unsigned int another = 0x9ABCDEF0;
    unsigned int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0xFF;  /* Store byte to low part */
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = another;
    pun.bytes[1] = 0xAA;  /* Modify middle byte */
    
    /* Truncation preserving high bits in source */
    unsigned char low_byte = wide_reg & 0xFF;
    unsigned char high_byte = (wide_reg >> 24) & 0xFF;
    
    /* Arithmetic that truncates to byte */
    unsigned int temp = wide_reg + another;
    unsigned char truncated = temp;  /* Implicit truncation */
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        pun.bytes[i] = (pun.bytes[i] + low_byte) & 0xFF;
    }
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %0\n\t"          /* Move low byte */
        : "=r" (asm_out)
        : "r" (asm_in)
        : "cc"
    );
    
    result = pun.full + truncated + asm_out;
    return result;
}

/* ==================== SUBREG patterns ==================== */

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    unsigned int result = 0;
    
    /* Type punning between different sizes */
    unsigned int int_val = 0xDEADBEEF;
    unsigned short short_val = int_val;  /* Implicit truncation */
    unsigned char char_val = int_val;
    
    /* Cast between types - may generate SUBREG */
    float float_val = 3.14159f;
    unsigned int int_from_float;
    memcpy(&int_from_float, &float_val, sizeof(float_val));
    
    /* Vector element extraction */
    int elem0 = vec[0];      /* May use SUBREG to extract element */
    short elem3 = short_vec[3];
    
    /* Mixed type operations */
    result = int_val + short_val + char_val;
    
    /* More complex type mixing */
    union {
        double d;
        unsigned long long ll;
        unsigned int i[2];
    } u;
    u.d = 2.71828;
    result += u.i[0] + u.i[1];
    
    /* Pointer casting for subregister access */
    unsigned long long big = 0x123456789ABCDEF0ULL;
    unsigned int* half = (unsigned int*)&big;
    result += half[0] + half[1];
    
    return result + elem0 + elem3;
}

/* ==================== Memory operand patterns ==================== */

NOINLINE unsigned int test_memory_operand(void) {
    /* Complex memory buffer */
    VOLATILE_VAR unsigned char buffer[256];
    VOLATILE_VAR unsigned int* int_buffer = (unsigned int*)buffer;
    VOLATILE_VAR unsigned int** ptr_array;
    
    /* Initialize buffer with pattern */
    for (int i = 0; i < 256; i++) {
        buffer[i] = i & 0xFF;
    }
    
    /* Multi-level pointer dereferencing */
    unsigned int*** triple_ptr;
    unsigned int** double_ptr;
    unsigned int* single_ptr;
    
    /* Build pointer chain */
    unsigned int target = 0xCAFEBABE;
    single_ptr = &target;
    double_ptr = &single_ptr;
    triple_ptr = &double_ptr;
    
    /* Complex addressing modes */
    unsigned int idx = global_index;
    unsigned int val1 = int_buffer[idx];           /* Indexed load */
    unsigned int val2 = int_buffer[idx + 1];       /* Offset load */
    unsigned int val3 = int_buffer[idx * 2];       /* Scaled index */
    
    /* Structure with nested arrays */
    struct nested {
        int data[4][4];
        int* ptrs[4];
    } nested_struct;
    
    /* Initialize nested structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            nested_struct.data[i][j] = i * 10 + j;
        }
        nested_struct.ptrs[i] = &nested_struct.data[i][0];
    }
    
    /* Complex structure access */
    int struct_val = nested_struct.data[1][2];
    struct_val += *nested_struct.ptrs[3];
    
    /* Pointer chasing */
    unsigned int chase = ***triple_ptr;
    
    /* Volatile memory operations */
    VOLATILE_VAR unsigned int* volatile volatile_ptr = int_buffer;
    unsigned int volatile_val = *(volatile_ptr + idx);
    
    /* Array with volatile index */
    VOLATILE_VAR int volatile_idx = get_index() & 0x3F;
    unsigned int volatile_indexed = buffer[volatile_idx];
    
    return val1 + val2 + val3 + struct_val + chase + volatile_val + volatile_indexed;
}

/* ==================== Main function ==================== */

int main(void) {
    unsigned int total = 0;
    
    printf("Testing GCC resource patterns...\n");
    
    /* Run all tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    printf("Result checksum: 0x%08X\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total != 0) {
        printf("All tests executed.\n");
    }
    
    return 0;
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ & 0xFF;
}

void* get_ptr(void) {
    static char buffer[100];
    return buffer;
}
