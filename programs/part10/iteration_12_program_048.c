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

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_s {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int pad:20;
} NOINLINE;

struct packed_bits {
    unsigned short a:2;
    unsigned short b:6;
    unsigned short c:8;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_s bf1, bf2;
    struct packed_bits pb;
    VOLATILE_VAR unsigned int raw_val = 0xDEADBEEF;
    unsigned int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = 5;
    bf1.value = 17;  /* Will be truncated to 5 bits */
    bf1.mode = 9;
    
    bf2.flag = bf1.value;  /* Extract from bitfield */
    bf2.value = bf1.flag;
    bf2.mode = bf1.mode;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int mask = (1 << 5) - 1;
    unsigned int shift = 3;
    
    /* Multiple extraction patterns */
    result |= (raw_val >> 0) & 0xFF;          /* Low byte */
    result |= (raw_val >> 8) & 0x1F;          /* 5-bit field */
    result |= (raw_val >> 13) & 0x7;          /* 3-bit field */
    result |= (raw_val >> 16) & ((1 << 12) - 1); /* 12-bit field */
    
    /* Packed bitfield operations */
    pb.a = (raw_val >> 0) & 0x3;
    pb.b = (raw_val >> 2) & 0x3F;
    pb.c = (raw_val >> 8) & 0xFF;
    
    /* Extract from packed structure */
    result |= (unsigned int)pb.a << 16;
    result |= (unsigned int)pb.b << 8;
    result |= (unsigned int)pb.c;
    
    /* Complex extraction chain */
    unsigned int temp = raw_val;
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        unsigned int field = (temp >> (i * 8)) & 0xF;
        result ^= field << (i * 4);
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0x12345678;
    VOLATILE_VAR unsigned short half_reg = 0;
    VOLATILE_VAR unsigned char byte_reg = 0;
    unsigned int result = 0;
    
    /* Byte store into wider integer (x86: movb) */
    *(VOLATILE_VAR unsigned char*)&wide_reg = 0xFF;
    result = wide_reg;
    
    /* Union for type punning - may generate low-part access */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0xDEADBEEF;
    pun.bytes[0] = 0x11;  /* Modify low byte only */
    pun.bytes[1] = 0x22;  /* Modify second byte */
    result ^= pun.full;
    
    /* Truncation preserving high bits in source */
    half_reg = wide_reg & 0xFFFF;
    byte_reg = wide_reg & 0xFF;
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %b0\n\t"           /* Low byte move */
        "movw %w1, %w0\n\t"           /* Low word move */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    result += asm_out;
    
    /* Multiple byte operations */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        ((VOLATILE_VAR unsigned char*)&wide_reg)[i] = i * 0x11;
    }
    result ^= wide_reg;
    
    /* Arithmetic that truncates to byte */
    unsigned int accumulator = 0;
    for (VOLATILE_VAR int i = 0; i < 100; i++) {
        accumulator += i;
        byte_reg = accumulator & 0xFF;  /* Keep only low byte */
    }
    result += byte_reg;
    
    return result;
}

/* ========== SUBREG patterns ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    unsigned int result = 0;
    
    /* Extract vector elements - often SUBREG */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    result = elem0 + elem2;
    
    /* Type punning between float and int */
    float f = 3.14159f;
    unsigned int bits;
    memcpy(&bits, &f, sizeof(bits));  /* Bitcast */
    result ^= bits;
    
    /* Cast between different integer sizes */
    unsigned long long big = 0x1122334455667788ULL;
    unsigned int lower = (unsigned int)big;          /* Truncation */
    unsigned int upper = (unsigned int)(big >> 32);  /* High part */
    result += lower ^ upper;
    
    /* Mixed vector operations */
    short s = vec_short[3];
    int extended = (int)s;  /* Sign extend - may use SUBREG */
    result += extended;
    
    /* Complex subregister chain */
    unsigned short us = 0xABCD;
    unsigned char uc = (unsigned char)us;
    unsigned int ui = (unsigned int)uc;
    unsigned long ul = (unsigned long)ui;
    result += (unsigned int)ul;
    
    /* Pointer casting for subreg access */
    unsigned int val = 0x89ABCDEF;
    unsigned short* sp = (unsigned short*)&val;
    unsigned char* cp = (unsigned char*)&val;
    result += sp[0] + sp[1] + cp[2];
    
    /* Float to int conversion */
    int ifloat = (int)vec_float[1];  /* Convert float to int */
    result += ifloat;
    
    return result;
}

/* ========== Complex Memory Operand patterns ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    /* Complex memory addressing */
    VOLATILE_VAR int array[256];
    VOLATILE_VAR int* ptr1 = array;
    VOLATILE_VAR int** ptr2 = &ptr1;
    VOLATILE_VAR int*** ptr3 = &ptr2;
    unsigned int result = 0;
    
    /* Initialize array with volatile index */
    for (VOLATILE_VAR int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Multi-level pointer dereference */
    result += ***ptr3;
    result += **(ptr2 + global_index % 16);
    
    /* Complex array indexing */
    int idx1 = get_index() % 256;
    int idx2 = get_index() % 128;
    result += array[idx1] + array[idx2 * 2];
    
    /* Structure with pointer chasing */
    struct nested n1, n2, n3;
    n1.data[0] = 100; n1.data[1] = 200; n1.data[2] = 300; n1.data[3] = 400;
    n2.data[0] = 500; n2.data[1] = 600; n2.data[2] = 700; n2.data[3] = 800;
    n3.data[0] = 900; n3.data[1] = 1000; n3.data[2] = 1100; n3.data[3] = 1200;
    
    n1.next = &n2;
    n2.next = &n3;
    n3.next = &n1;
    
    /* Chain of memory accesses */
    result += n1.next->next->data[2];
    result += n2.next->data[get_index() % 4];
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = array;
    for (VOLATILE_VAR int i = 0; i < 10; i++) {
        result += *(volatile_ptr + i * 7);
    }
    
    /* Mixed offset calculations */
    int* base = array + 128;
    result += base[-10];
    result += base[10];
    result += *(base + (get_index() & 31));
    
    /* Simulate stack-like access pattern */
    int* stack_ptr = array + 200;
    for (int i = 0; i < 8; i++) {
        stack_ptr--;
        result += *stack_ptr;
    }
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: 0x%08X\n", total);
    
    return (int)(total & 0x7FFFFFFF);
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 256;
}

void* get_ptr(void) {
    static char buffer[1024];
    return buffer;
}
