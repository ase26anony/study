/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE volatile

/* Opaque function to prevent constant propagation */
extern int get_index(void);

/* Global volatile variables to force memory operations */
VOLATILE int g_index = 0;
VOLATILE char *g_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_s {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int extra:8;
    unsigned int pad:16;
} NOINLINE;

struct bitfield_large {
    unsigned long long low:12;
    unsigned long long high:20;
    unsigned long long middle:32;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_s bf1, bf2;
    struct bitfield_large bfl;
    VOLATILE unsigned int result = 0;
    
    /* Initialize with non-zero values */
    bf1.flag = 5;
    bf1.value = 20;
    bf1.extra = 100;
    
    bf2.flag = 2;
    bf2.value = 31;
    bf2.extra = 200;
    
    bfl.low = 0xABC;
    bfl.high = 0xDEF12;
    bfl.middle = 0x12345678;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.value = bf2.flag;          /* Extract from bitfield */
    bf2.extra = bf1.value << 2;    /* Shift and assign to bitfield */
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int raw = 0xDEADBEEF;
    unsigned int mask = 0x1F;      /* 5-bit mask */
    unsigned int shift = g_index & 7;
    
    /* Multiple extraction patterns */
    result |= (raw >> shift) & mask;
    result |= (bfl.middle >> 4) & 0xFFF;
    result |= ((unsigned int)bfl.high << 8) & 0xFFFF00;
    
    /* Compound extraction */
    unsigned int temp = (raw >> 16) | (raw << 16);
    result |= (temp >> (shift + 1)) & (mask << 1);
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE unsigned int full_reg = 0x12345678;
    VOLATILE unsigned short half_reg = 0;
    VOLATILE unsigned char byte_reg = 0;
    unsigned int result = 0;
    
    /* Byte-sized store into integer - may generate STRICT_LOW_PART */
    *(VOLATILE unsigned char*)&full_reg = 0xFF;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x87654321;
    
    /* Store to low byte only */
    pun.bytes[0] = g_index & 0xFF;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        pun.bytes[i] = (pun.bytes[i] + i) & 0xFF;
    }
    
    /* Arithmetic that truncates to low part */
    half_reg = full_reg & 0xFFFF;
    byte_reg = (full_reg >> 8) & 0xFF;
    
    /* Inline assembly forcing low-byte register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x89ABCDEF;
    asm volatile (
        "movb %b1, %b0\n\t"
        "movw %w1, %w0\n\t"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    result = full_reg + pun.full + half_reg + byte_reg + asm_out;
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE v4si vec_int = {1, 2, 3, 4};
    VOLATILE v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE float f = 3.14159f;
    VOLATILE int i = 0;
    unsigned int result = 0;
    
    /* Type punning between float and int */
    i = *(VOLATILE int*)&f;  /* Bitcast via pointer */
    
    /* Vector element extraction - often SUBREG */
    int elem = vec_int[g_index % 4];
    short selem = vec_short[g_index % 8];
    
    /* Mixing types of different sizes */
    short s = (short)i;
    char c = (char)(i >> 16);
    
    /* Union for subregister access */
    union {
        long long ll;
        int i[2];
        short s[4];
    } converter;
    converter.ll = 0x1122334455667788ULL;
    
    /* Access different-sized views */
    result += converter.i[0];
    result += converter.s[2];
    
    /* Complex subregister pattern */
    v4si vec2 = vec_int + (v4si){5, 5, 5, 5};
    int sum = 0;
    for (int j = 0; j < 4; j++) {
        sum += vec2[j];  /* Each access may be SUBREG */
    }
    
    result += sum + elem + selem + s + c;
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[4];
    struct nested *next;
};

NOINLINE unsigned int test_memory_operand(void) {
    VOLATILE int buffer[64];
    VOLATILE int *ptr1, **ptr2, ***ptr3;
    VOLATILE struct nested nodes[4];
    unsigned int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        buffer[i] = i * 3;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            nodes[i].data[j] = i * 10 + j;
        }
        nodes[i].next = (i < 3) ? &nodes[i + 1] : NULL;
    }
    
    ptr1 = &buffer[0];
    ptr2 = &ptr1;
    ptr3 = &ptr2;
    
    /* Multi-level pointer dereferencing */
    result += ***ptr3;
    result += **(ptr2 + (g_index & 1));
    
    /* Complex array indexing */
    VOLATILE int idx1 = get_index() % 64;
    VOLATILE int idx2 = get_index() % 64;
    
    result += buffer[idx1] * buffer[idx2];
    result += buffer[idx1 + idx2];
    
    /* Structure field access with pointer chasing */
    struct nested *current = &nodes[0];
    for (int i = 0; i < 4 && current; i++) {
        result += current->data[i % 4];
        current = current->next;
    }
    
    /* Volatile memory operations that won't be optimized away */
    VOLATILE int *volatile_ptr = buffer;
    for (int i = 0; i < 8; i++) {
        result += *(volatile_ptr + i * 2);
    }
    
    /* Memory operation with side effect */
    buffer[0] = result;
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing GCC resource patterns...\n");
    
    /* Initialize global */
    g_index = 42;
    
    /* Run all tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Total checksum: %u (0x%08X)\n", total, total);
    
    return total != 0 ? 0 : 1;
}

/* Dummy implementation of opaque function */
int get_index(void) {
    static int counter = 0;
    return counter++ % 32;
}
