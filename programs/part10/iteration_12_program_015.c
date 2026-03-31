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
extern void escape(void*);

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_pack {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int reserved:20;
} NOINLINE;

struct bitfield_large {
    unsigned long long high:32;
    unsigned long long low:32;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_pack bf = {0};
    VOLATILE_VAR struct bitfield_large bfl = {0};
    VOLATILE_VAR unsigned int result = 0;
    
    /* Direct bitfield assignments that may generate ZERO_EXTRACT */
    bf.flag = get_index() & 0x7;
    bf.value = (get_index() >> 3) & 0x1F;
    bf.mode = 0xA;
    
    /* Bitfield extraction via masking */
    unsigned int raw = *(unsigned int*)&bf;
    result = (raw >> 3) & 0x1F;  /* Extract 'value' field */
    
    /* Multi-word bitfield operations */
    bfl.high = 0xDEADBEEF;
    bfl.low = 0xCAFEBABE;
    
    /* Complex extraction spanning multiple operations */
    unsigned long long combined = *(unsigned long long*)&bfl;
    result ^= (unsigned int)((combined >> 32) & 0xFFFFFFFF);
    result ^= (unsigned int)(combined & 0xFFFFFFFF);
    
    /* Bitfield in conditional context */
    if (bf.flag & 0x4) {
        result |= 0x80000000;
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_val = 0x12345678;
    VOLATILE_VAR unsigned char* byte_ptr;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    byte_ptr = (unsigned char*)&wide_val;
    byte_ptr[0] = 0xFF;  /* Store to low byte */
    byte_ptr[1] = get_index() & 0xFF;
    
    /* Union for type punning with byte access */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x87654321;
    pun.bytes[2] = 0xAA;  /* Modify middle byte */
    
    /* Truncation preserving high bits in source */
    unsigned int temp = wide_val;
    unsigned char low_byte = temp & 0xFF;  /* Explicit truncation */
    result = low_byte;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        pun.bytes[i] = (pun.bytes[i] + 1) & 0xFF;
    }
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_in = 0x89ABCDEF;
    unsigned int asm_out;
    asm volatile (
        "movb %b1, %b0\n\t"
        "andb $0xF0, %b0"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    result ^= asm_out;
    result ^= pun.full;
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR unsigned int result = 0;
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    result = elem0 + elem2;
    
    /* Type punning between different sizes */
    unsigned int int_val = 0xDEADBEEF;
    unsigned short short_val = (unsigned short)int_val;  /* Truncation */
    unsigned char char_val = (unsigned char)(int_val >> 16);
    
    result ^= short_val;
    result ^= (char_val << 8);
    
    /* Mixed vector operations */
    vec_short[3] = get_index() & 0xFFFF;
    short middle = vec_short[3];
    result += middle;
    
    /* Float/int bitcasting */
    float f = 3.14159f;
    unsigned int fbits;
    memcpy(&fbits, &f, sizeof(fbits));  /* Type punning via memcpy */
    
    /* Extract halves of 64-bit value */
    unsigned long long big = 0x123456789ABCDEF0ULL;
    unsigned int low = (unsigned int)big;          /* Low 32 bits */
    unsigned int high = (unsigned int)(big >> 32); /* High 32 bits */
    
    result ^= low ^ high ^ fbits;
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    VOLATILE_VAR int buffer[256];
    VOLATILE_VAR int* ptr1 = buffer;
    VOLATILE_VAR int** ptr2 = &ptr1;
    VOLATILE_VAR int*** ptr3 = &ptr2;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Initialize buffer with volatile pattern */
    for (VOLATILE_VAR int i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    
    /* Complex pointer dereferencing */
    result = ***ptr3;  /* Triple indirection */
    
    /* Array indexing with volatile index */
    VOLATILE_VAR int idx = get_index() & 0xFF;
    result += buffer[idx];
    result += buffer[idx + 1];
    
    /* Structure with pointer chasing */
    struct nested node1, node2;
    node1.data[0] = 0x11111111;
    node1.data[1] = 0x22222222;
    node1.next = &node2;
    node2.data[0] = 0x33333333;
    node2.next = NULL;
    
    /* Chain of structure accesses */
    result ^= node1.next->data[0];
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = buffer;
    for (int i = 0; i < 10; i++) {
        volatile_ptr[i] = volatile_ptr[i] ^ 0x55555555;
    }
    
    /* Multi-dimensional array with complex addressing */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    VOLATILE_VAR int row = get_index() & 0x7;
    VOLATILE_VAR int col = get_index() & 0x7;
    result += matrix[row][col];
    
    /* Prevent dead store elimination */
    escape(buffer);
    escape(&node1);
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    printf("Result checksum: 0x%08X\n", total);
    
    /* Use result to prevent optimization */
    if (total == 0xDEADBEEF) {
        printf("Impossible match!\n");
    }
    
    return (int)(total & 0x7FFFFFFF);
}

/* Dummy external functions */
int get_index(void) {
    static volatile int counter = 0;
    return counter++;
}

void escape(void* p) {
    asm volatile ("" : : "r"(p) : "memory");
}
