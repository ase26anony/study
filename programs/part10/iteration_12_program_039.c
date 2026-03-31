/* test_resource_patterns.c */
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
struct bitfield_struct {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int pad:20;
} NOINLINE;

struct packed_bitfields {
    unsigned short field1:5;
    unsigned short field2:6;
    unsigned short field3:5;
} NOINLINE;

NOINLINE int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_struct bf;
    VOLATILE_VAR struct packed_bitfields packed;
    VOLATILE_VAR unsigned int raw_value = 0x12345678;
    int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf.flag = 5;
    bf.value = 20;
    bf.mode = 9;
    
    /* Extract via bitfield member access */
    result += bf.flag;
    result += bf.value;
    result += bf.mode;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    packed.field1 = (raw_value >> 0) & 0x1F;
    packed.field2 = (raw_value >> 5) & 0x3F;
    packed.field3 = (raw_value >> 11) & 0x1F;
    
    /* More complex extraction patterns */
    result += (raw_value >> 8) & 0xFF;      /* Extract byte */
    result += (raw_value >> 16) & 0x7;      /* Extract 3 bits */
    result += (raw_value >> 4) & 0xF;       /* Extract 4 bits */
    
    /* Nested extraction */
    unsigned int temp = raw_value ^ 0xAAAAAAAA;
    result += (temp >> 12) & 0xF;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_int = 0xDEADBEEF;
    VOLATILE_VAR uint16_t half_word;
    VOLATILE_VAR uint8_t byte_val;
    int result = 0;
    
    /* Byte store into wider integer (may generate STRICT_LOW_PART) */
    *(volatile uint8_t*)&wide_int = 0x42;
    result += wide_int;
    
    /* Another byte store */
    ((volatile uint8_t*)&wide_int)[1] = 0x99;
    result += wide_int;
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } pun;
    pun.full = 0x12345678;
    pun.bytes[2] = 0xAA;  /* Modify only one byte */
    result += pun.full;
    
    /* Truncation preserving high bits context */
    half_word = wide_int & 0xFFFF;
    result += half_word;
    
    /* Complex expression with byte truncation */
    byte_val = (wide_int >> 8) & 0xFF;
    result += byte_val;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        ((volatile uint8_t*)&wide_int)[i] = i * 0x11;
    }
    result += wide_int;
    
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR float f = 3.14159f;
    VOLATILE_VAR int i;
    int result = 0;
    
    /* Type punning between float and int */
    i = *(int*)&f;  /* Bitcast via pointer */
    result += i;
    
    /* Vector element extraction (often SUBREG) */
    int elem = vec[global_index % 4];
    result += elem;
    
    /* Short to int promotion with sign extension */
    short s = -100;
    int promoted = s;  /* May involve SUBREG */
    result += promoted;
    
    /* Vector to scalar conversion */
    short_vec[3] = 999;
    short sv = short_vec[2];
    result += sv;
    
    /* Mixed size operations */
    char c = 'A';
    int char_as_int = c;  /* Zero/sign extension via SUBREG */
    result += char_as_int;
    
    /* Union for subregister access */
    union {
        double d;
        uint32_t parts[2];
    } dpun;
    dpun.d = 2.71828;
    result += dpun.parts[0];  /* Access half of double */
    result += dpun.parts[1];
    
    return result;
}

/* ========== Complex Memory Operand patterns ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE int test_memory_operand(void) {
    VOLATILE_VAR int buffer[64];
    VOLATILE_VAR int* ptr1 = buffer;
    VOLATILE_VAR int** ptr2 = &ptr1;
    VOLATILE_VAR int*** ptr3 = &ptr2;
    VOLATILE_VAR struct nested nodes[3];
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 64; i++) {
        buffer[i] = i * 3;
    }
    
    nodes[0].data[0] = 100;
    nodes[0].next = &nodes[1];
    nodes[1].data[0] = 200;
    nodes[1].next = &nodes[2];
    nodes[2].data[0] = 300;
    nodes[2].next = NULL;
    
    /* Multi-level pointer dereference */
    result += ***ptr3;
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index;
    result += buffer[idx % 64];
    result += buffer[(idx + 1) % 64];
    result += buffer[(idx + 2) % 64];
    
    /* Structure field access with pointer chasing */
    result += nodes[0].data[0];
    result += nodes[0].next->data[0];
    result += nodes[0].next->next->data[0];
    
    /* Volatile memory operations */
    *(volatile int*)(buffer + 10) = 0xABCD;
    result += *(volatile int*)(buffer + 10);
    
    /* Pointer arithmetic with dereference */
    int* p = buffer + 20;
    result += *p;
    result += *(p + 1);
    result += *(p - 1);
    
    /* Mixed offset calculation */
    result += buffer[global_index % 8 * 2 + 1];
    
    return result;
}

/* ========== Inline Assembly patterns ========== */
NOINLINE int test_inline_asm(void) {
    VOLATILE_VAR int in = 0x12345678;
    VOLATILE_VAR char out_char;
    VOLATILE_VAR short out_short;
    VOLATILE_VAR int result = 0;
    
    /* Access low byte of register (%b modifier) */
    asm volatile (
        "movb %b1, %0"
        : "=r" (out_char)
        : "r" (in)
        : "cc"
    );
    result += out_char;
    
    /* Access low word of register (%w modifier) */
    asm volatile (
        "movw %w1, %0"
        : "=r" (out_short)
        : "r" (in)
        : "cc"
    );
    result += out_short;
    
    /* Memory operand with complex addressing */
    VOLATILE_VAR int array[4] = {10, 20, 30, 40};
    VOLATILE_VAR int idx = 2;
    VOLATILE_VAR int asm_result;
    
    asm volatile (
        "movl (%1, %2, 4), %0"
        : "=r" (asm_result)
        : "r" (array), "r" (idx)
        : "memory"
    );
    result += asm_result;
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Initialize global variables */
    global_index = 7;
    global_ptr = malloc(256);
    if (global_ptr) {
        memset(global_ptr, 0xAA, 256);
    }
    
    /* Run all tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    total += test_inline_asm();
    
    printf("Total checksum: %d\n", total);
    printf("All tests completed.\n");
    
    if (global_ptr) {
        free(global_ptr);
    }
    
    return total != 0 ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) { return 42; }
void* get_ptr(void) { return malloc(16); }
