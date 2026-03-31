/* Test program to generate specific RTL patterns for GCC resource.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization */
volatile int global_counter = 0;
extern int opaque(int); /* Prevent constant propagation */

/* ========== ZERO_EXTRACT patterns ========== */
__attribute__((noinline, optimize("O0")))
int test_zero_extract(void) {
    /* Bitfield structures that may compile to ZERO_EXTRACT */
    struct bitfield1 {
        unsigned int flag:3;
        unsigned int value:5;
        unsigned int mode:4;
    } bf1;
    
    struct bitfield2 {
        unsigned long long high:32;
        unsigned long long low:24;
        unsigned long long extra:8;
    } bf2;
    
    volatile unsigned int source = 0xABCD1234;
    int result = 0;
    
    /* Direct bitfield assignments */
    bf1.flag = (source >> 0) & 0x7;
    bf1.value = (source >> 3) & 0x1F;
    bf1.mode = (source >> 8) & 0xF;
    
    /* Bitfield extraction via masking */
    unsigned int extracted = (source >> 5) & 0xFF;  /* Potential ZERO_EXTRACT */
    result += extracted;
    
    /* Complex bitfield manipulation */
    bf2.low = source & 0xFFFFFF;
    bf2.high = (source >> 24) & 0xFF;
    bf2.extra = (source >> 16) & 0xFF;
    
    /* Manual bitfield extraction that might use ZERO_EXTRACT */
    unsigned int mask = 0x3F;  /* 6 bits */
    unsigned int shift = opaque(10) % 20;
    unsigned int field = (source >> shift) & mask;
    result += field;
    
    /* Union with bitfields */
    union {
        unsigned int full;
        struct {
            unsigned int a:10;
            unsigned int b:10;
            unsigned int c:12;
        } parts;
    } u;
    
    u.full = source;
    result += u.parts.b;  /* Bitfield access */
    
    return result + bf1.flag + bf1.value + bf2.low;
}

/* ========== STRICT_LOW_PART patterns ========== */
__attribute__((noinline, optimize("O0")))
int test_strict_low_part(void) {
    volatile unsigned int wide_reg = 0xDEADBEEF;
    volatile unsigned char byte_target;
    int result = 0;
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0x42;
    result += wide_reg;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    
    pun.full = 0x12345678;
    pun.bytes[1] = 0xAA;  /* Modify only one byte */
    result += pun.full;
    
    /* Truncation that preserves high bits */
    unsigned int temp = wide_reg;
    unsigned char low_byte = temp & 0xFF;  /* Explicit truncation */
    result += low_byte;
    
    /* Inline assembly forcing low-part access */
    unsigned int in_val = 0x87654321;
    unsigned char out_byte;
    
    /* x86-specific: %b0 modifier accesses low byte */
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r"(out_byte)
        : "r"(in_val)
        : "cc"
    );
    result += out_byte;
    
    /* Multiple byte operations */
    volatile unsigned short *half_ptr = (volatile unsigned short*)&wide_reg;
    *half_ptr = 0x1234;  /* Store to low 16 bits */
    result += wide_reg;
    
    return result;
}

/* ========== SUBREG patterns ========== */
__attribute__((noinline, optimize("O0")))
int test_subreg(void) {
    /* Vector extensions often use SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    int result = 0;
    
    /* Vector element extraction - often SUBREG */
    result += vec_int[2];  /* Extract element */
    result += vec_short[5];
    
    /* Type punning between different sizes */
    unsigned int int_val = 0x12345678;
    unsigned short short_val;
    
    /* Cast that may generate SUBREG */
    short_val = (unsigned short)int_val;
    result += short_val;
    
    /* Float/int reinterpretation */
    float f = 3.14159f;
    unsigned int fi;
    
    /* Bitcast via union - may use SUBREG */
    union {
        float f;
        unsigned int i;
    } float_union;
    
    float_union.f = f;
    result += float_union.i & 0xFF;
    
    /* Pointer casting for sub-register access */
    unsigned long long big = 0x1122334455667788ULL;
    unsigned int *as_int = (unsigned int*)&big;
    result += as_int[1];  /* Access high 32 bits on little-endian */
    
    /* Mixed-size operations */
    short s1 = 1000;
    int i1 = s1 * 2;  /* Promotion then operation */
    result += i1;
    
    return result;
}

/* ========== Complex Memory Operands ========== */
__attribute__((noinline, optimize("O0")))
int test_memory_operand(void) {
    /* Complex addressing modes */
    volatile int buffer[100];
    volatile int *ptr1, **ptr2, ***ptr3;
    volatile int index1, index2;
    
    /* Initialize with opaque values to prevent constant folding */
    index1 = opaque(0) % 50;
    index2 = opaque(1) % 50;
    
    /* Multi-level pointer dereferencing */
    int local = 42;
    ptr1 = &local;
    ptr2 = &ptr1;
    ptr3 = &ptr2;
    
    int result = ***ptr3;  /* Triple indirection */
    
    /* Complex array indexing */
    result += buffer[index1 * 2 + index2];
    
    /* Structure with nested arrays */
    struct nested {
        int data[10][10];
        volatile int *next;
    } nested_struct;
    
    /* Initialize */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            nested_struct.data[i][j] = i * 10 + j;
        }
    }
    
    /* Complex memory access pattern */
    volatile int idx_i = opaque(2) % 10;
    volatile int idx_j = opaque(3) % 10;
    result += nested_struct.data[idx_i][idx_j];
    
    /* Pointer arithmetic with volatile */
    volatile char *char_ptr = (volatile char*)buffer;
    char_ptr += opaque(4) * sizeof(int);
    result += *char_ptr;
    
    /* Memory access with side effect */
    volatile int *volatile_ptr = (volatile int*)buffer;
    volatile_ptr[10] = result;
    result += volatile_ptr[10];
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    printf("Starting RTL pattern generation tests...\n");
    
    /* Run all tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    /* Use result to prevent dead code elimination */
    printf("Final checksum: %d\n", total);
    
    /* Force use of global to prevent optimization */
    global_counter = total % 256;
    
    return global_counter;
}

/* Opaque function to prevent constant propagation */
int opaque(int x) {
    /* Use system time or other unpredictable source */
    static int counter = 0;
    return (x * 1103515245 + 12345 + counter++) & 0x7FFFFFFF;
}
