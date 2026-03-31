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
extern void escape(void*);

/* Global volatile variables to prevent optimizations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char global_byte = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_s {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int extra:8;
    unsigned int padding:16;
} NOINLINE;

struct packed_bitfield {
    unsigned short a:4;
    unsigned short b:4;
    unsigned short c:4;
    unsigned short d:4;
} NOINLINE;

NOINLINE static unsigned int test_zero_extract(void) {
    struct bitfield_s bf1 = {0};
    struct packed_bitfield bf2 = {0};
    VOLATILE_VAR unsigned int temp = 0x12345678;
    unsigned int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = (temp >> 3) & 0x7;      /* Extract 3 bits */
    bf1.value = (temp >> 8) & 0x1F;    /* Extract 5 bits */
    
    /* Nested bitfield extraction */
    bf2.a = (bf1.flag & 0x3);
    bf2.b = (bf1.value >> 1) & 0x7;
    
    /* Complex bit extraction expression */
    result = ((temp & 0xFF00) >> 8) | ((temp & 0xFF) << 8);
    
    /* Multiple extractions in one expression */
    result ^= ((bf1.extra << 16) | (bf1.value << 8) | bf1.flag);
    
    /* Force side effects */
    global_byte = bf1.flag;
    
    return result + bf2.a + bf2.b;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned short half_reg = 0;
    VOLATILE_VAR unsigned char byte_reg = 0;
    unsigned int result = 0;
    
    /* Union for type punning - may generate low-part accesses */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = wide_reg;
    
    /* Byte-sized stores into wider integers */
    pun.bytes[0] = 0xAA;          /* Low byte store */
    pun.bytes[1] = global_byte;   /* Volatile byte store */
    
    /* Truncation preserving high bits */
    half_reg = wide_reg & 0xFFFF;
    byte_reg = wide_reg & 0xFF;
    
    /* Inline assembly forcing low-byte register access */
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (byte_reg)
        : "r" (wide_reg)
        : "cc"
    );
    
    /* Multiple low-part operations */
    result = pun.full;
    result += half_reg;
    result += byte_reg;
    
    /* Force low-part arithmetic */
    wide_reg = (wide_reg & ~0xFF) | (byte_reg + 1);
    
    return result + wide_reg;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static unsigned int test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR float f = 3.14159f;
    VOLATILE_VAR int i = 0;
    unsigned int result = 0;
    
    /* Type punning between float and int */
    i = *(int*)&f;                /* Bitcast via pointer - may use SUBREG */
    result = i;
    
    /* Vector element extraction */
    int elem = vec_int[global_index % 4];  /* Non-constant index */
    result += elem;
    
    /* Mixed-size vector operations */
    short half = vec_short[global_index % 8];
    result += half;
    
    /* Explicit casts between different integer sizes */
    long long big = 0x123456789ABCDEF0LL;
    int small = (int)big;         /* Truncation */
    short smaller = (short)small; /* Further truncation */
    
    result += smaller;
    
    /* Union for subregister access */
    union {
        double d;
        int parts[2];
    } dpun;
    dpun.d = 2.71828;
    result += dpun.parts[0];      /* Access low 32 bits of double */
    
    return result;
}

/* ========== Complex Memory Operand patterns ========== */
struct nested {
    int data[3];
    struct nested* next;
};

NOINLINE static unsigned int test_memory_operand(void) {
    VOLATILE_VAR int buffer[256];
    VOLATILE_VAR int* ptr1 = buffer;
    VOLATILE_VAR int** ptr2 = &ptr1;
    VOLATILE_VAR int*** ptr3 = &ptr2;
    unsigned int result = 0;
    
    /* Initialize buffer with pattern */
    for (int i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    
    /* Complex pointer dereferencing */
    result += ***ptr3;                     /* Triple indirection */
    result += **(ptr2 + global_index % 16); /* Pointer arithmetic */
    
    /* Multi-level array indexing */
    int idx1 = get_index() % 64;
    int idx2 = get_index() % 4;
    int idx3 = get_index() % 2;
    
    result += buffer[idx1 * 4 + idx2 * 2 + idx3];
    
    /* Structure with pointer chasing */
    struct nested n1, n2, n3;
    n1.data[0] = 100; n1.data[1] = 200; n1.data[2] = 300;
    n2.data[0] = 400; n2.data[1] = 500; n2.data[2] = 600;
    n3.data[0] = 700; n3.data[1] = 800; n3.data[2] = 900;
    
    n1.next = &n2;
    n2.next = &n3;
    n3.next = &n1;
    
    /* Chain of structure accesses */
    result += n1.next->next->data[global_index % 3];
    
    /* Volatile memory operations */
    VOLATILE_VAR char* byte_ptr = (char*)buffer;
    for (int i = 0; i < 32; i++) {
        byte_ptr[i * 8] = i;  /* Scattered stores */
    }
    
    /* Complex address calculation */
    result += *(int*)((char*)buffer + sizeof(int) * (global_index % 64));
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    printf("  Zero-extract test completed\n");
    
    total += test_strict_low_part();
    printf("  Strict-low-part test completed\n");
    
    total += test_subreg();
    printf("  Subreg test completed\n");
    
    total += test_memory_operand();
    printf("  Memory operand test completed\n");
    
    printf("Total checksum: %u\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        return 0;
    } else {
        return 1;
    }
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 256;
}

void escape(void* p) {
    /* Prevent optimization of pointer */
    asm volatile ("" : : "r"(p) : "memory");
}
