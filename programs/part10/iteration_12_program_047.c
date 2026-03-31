/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

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

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_packed {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int status:2;
} __attribute__((packed));

struct bitfield_nested {
    struct {
        unsigned int low:8;
        unsigned int high:8;
    } bytes;
    unsigned int combined:16;
};

NOINLINE static unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packed bf1 = {0};
    VOLATILE_VAR struct bitfield_nested bf2 = {0};
    VOLATILE_VAR unsigned int raw_value = 0xABCD1234;
    unsigned int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = 0x5;
    bf1.value = 0x1F;
    bf1.mode = 0x9;
    bf1.status = 0x3;
    
    /* Extract bitfields using masking operations */
    result |= (bf1.flag & 0x7);
    result |= ((bf1.value & 0x1F) << 3);
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    unsigned int extracted = (raw_value >> 8) & 0xFF;  /* Extract byte 1 */
    result ^= extracted;
    
    /* Nested bitfield access */
    bf2.bytes.low = 0xAA;
    bf2.bytes.high = 0xBB;
    bf2.combined = (bf2.bytes.high << 8) | bf2.bytes.low;
    
    /* Complex bitfield expression */
    unsigned int mask = 0x0F0F;
    unsigned int masked = (raw_value & mask) | ((raw_value >> 4) & mask);
    result += masked;
    
    /* Multiple extractions */
    for (int i = 0; i < 4; i++) {
        unsigned int shift = i * 8;
        unsigned int byte = (raw_value >> shift) & 0xFF;
        result += byte;
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned char byte_store;
    unsigned int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    unsigned char* byte_ptr = (unsigned char*)&wide_reg;
    byte_ptr[0] = 0x11;  /* Store byte into low part */
    byte_ptr[1] = 0x22;  /* Store into next byte */
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x12345678;
    pun.bytes[0] = 0xAA;  /* Modify low byte only */
    result = pun.full;
    
    /* Explicit truncation preserving high bits in source */
    unsigned int temp = wide_reg;
    unsigned char low_byte = temp & 0xFF;  /* Extract low byte */
    result ^= (low_byte << 8);
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %0\n\t"  /* %b1 accesses low byte of register */
        : "=r" (asm_out)
        : "r" (asm_in)
        : "cc"
    );
    result += asm_out;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        unsigned char* ptr = ((unsigned char*)&wide_reg) + i;
        *ptr = (*ptr + 1) & 0xFF;  /* Modify each byte separately */
    }
    result ^= wide_reg;
    
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE static unsigned int test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    VOLATILE_VAR v4si vec_int = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    unsigned int result = 0;
    
    /* Extract vector elements - generates SUBREG */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    result = elem0 + elem2;
    
    /* Type punning between different sizes */
    unsigned int int_val = 0x12345678;
    unsigned short short_val = (unsigned short)int_val;  /* Truncation */
    unsigned char char_val = (unsigned char)int_val;     /* Further truncation */
    result ^= (short_val << 8) | char_val;
    
    /* Float/int reinterpretation */
    float float_val = 3.14159f;
    unsigned int int_bits;
    memcpy(&int_bits, &float_val, sizeof(float_val));  /* Type punning */
    result += int_bits;
    
    /* Mixed-size operations */
    unsigned long long big_val = 0x1122334455667788ULL;
    unsigned int low_part = (unsigned int)big_val;      /* Low 32 bits */
    unsigned int high_part = (unsigned int)(big_val >> 32); /* High 32 bits */
    result ^= low_part ^ high_part;
    
    /* Vector element manipulation */
    vec_short[0] = 100;  /* Modify single element */
    vec_short[3] = 200;
    result += vec_short[0] + vec_short[3];
    
    /* Complex subregister access pattern */
    for (int i = 0; i < 4; i++) {
        vec_int[i] = vec_int[i] * 2 + vec_short[i*2];
    }
    result += vec_int[0] + vec_int[3];
    
    return result;
}

/* ========== Memory operand patterns ========== */
NOINLINE static unsigned int test_memory_operand(void) {
    /* Complex memory addressing structures */
    struct node {
        int value;
        struct node* next;
        struct node* prev;
    };
    
    /* Allocate and initialize test structures */
    struct node nodes[4];
    for (int i = 0; i < 4; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = (i < 3) ? &nodes[i+1] : NULL;
        nodes[i].prev = (i > 0) ? &nodes[i-1] : NULL;
    }
    
    VOLATILE_VAR int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    unsigned int result = 0;
    VOLATILE_VAR int* volatile ptr1 = &array[0];
    VOLATILE_VAR int** volatile ptr2 = &ptr1;
    VOLATILE_VAR int*** volatile ptr3 = &ptr2;
    
    /* Multi-level pointer dereferencing */
    result += ***ptr3;  /* Triple dereference */
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index % 256;
    result += array[idx];
    result += array[idx + 1];
    result += array[idx + 2];
    
    /* Structure pointer chasing */
    struct node* current = &nodes[0];
    for (int i = 0; i < 3 && current != NULL; i++) {
        result += current->value;
        current = current->next;  /* Pointer chasing */
    }
    
    /* Volatile memory operations */
    VOLATILE_VAR int* mem_ptr = (int*)global_ptr;
    if (mem_ptr != NULL) {
        result += *mem_ptr;
    }
    
    /* Complex address calculation */
    int offset = get_index() % 128;
    result += *(array + offset);
    result += *(array + offset * 2);
    
    /* Memory operation with side effect */
    VOLATILE_VAR int counter = 0;
    for (int i = 0; i < 16; i++) {
        array[i] = array[i] + counter++;
    }
    result += array[15];
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Initialize globals */
    global_index = 42;
    global_ptr = malloc(sizeof(int));
    if (global_ptr) {
        *(int*)global_ptr = 0xCAFEBABE;
    }
    
    /* Run all pattern tests */
    total += test_zero_extract();
    printf("  Zero-extract test completed\n");
    
    total += test_strict_low_part();
    printf("  Strict-low-part test completed\n");
    
    total += test_subreg();
    printf("  Subreg test completed\n");
    
    total += test_memory_operand();
    printf("  Memory operand test completed\n");
    
    /* Cleanup */
    if (global_ptr) {
        free(global_ptr);
    }
    
    printf("All tests completed. Checksum: 0x%08X\n", total);
    return (int)(total & 0x7FFFFFFF);
}

/* External functions to prevent optimization */
int get_index(void) {
    static int counter = 0;
    return counter++ % 256;
}

void* get_ptr(void) {
    static char buffer[64];
    return buffer;
}
