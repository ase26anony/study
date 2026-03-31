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
struct bitfield_s {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int pad:20;
} NOINLINE;

struct packed_bitfield {
    unsigned short a:2;
    unsigned short b:6;
    unsigned short c:8;
} NOINLINE;

NOINLINE int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_s bf;
    VOLATILE_VAR struct packed_bitfield pbf;
    VOLATILE_VAR unsigned int raw = 0xDEADBEEF;
    int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf.flag = 5;
    bf.value = 20;
    bf.mode = 9;
    result += bf.flag;
    
    /* Extract via bitfield */
    pbf.a = 2;
    pbf.b = 42;
    pbf.c = 128;
    result += pbf.b;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    unsigned int mask = 0x1F;  /* 5 bits */
    unsigned int shift = 8;
    unsigned int extracted = (raw >> shift) & mask;
    result += extracted;
    
    /* Multiple extractions */
    extracted = (raw >> 16) & 0xFF;
    result += extracted;
    
    /* Compound extraction */
    unsigned int field1 = (raw & 0x7) << 2;
    unsigned int field2 = (raw >> 4) & 0x3;
    result += field1 | field2;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
union int_bytes {
    uint32_t full;
    uint8_t bytes[4];
    uint16_t halves[2];
} NOINLINE;

NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t big_int = 0x12345678;
    VOLATILE_VAR union int_bytes u;
    VOLATILE_VAR uint8_t byte_var = 0;
    int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&big_int = 0xFF;
    result += big_int & 0xFF;
    
    /* Another byte store with offset */
    *(volatile uint8_t*)((char*)&big_int + 1) = 0xAA;
    result += (big_int >> 8) & 0xFF;
    
    /* Using union for byte access */
    u.full = 0x87654321;
    u.bytes[2] = 0xCC;
    result += u.full;
    
    /* Truncation preserving high bits in source */
    uint32_t source = 0xABCD1234;
    uint8_t low_byte = source & 0xFF;  /* May use STRICT_LOW_PART */
    result += low_byte;
    
    /* Inline assembly forcing low-byte register access */
    uint32_t asm_in = 0xDEADBEEF;
    uint32_t asm_out;
    asm volatile (
        "movb %%al, %1\n\t"
        "movl %1, %0"
        : "=r"(asm_out)
        : "m"(asm_in), "a"(asm_in)
        : "memory"
    );
    result += asm_out & 0xFF;
    
    /* 16-bit low part access */
    *(volatile uint16_t*)&big_int = 0x8888;
    result += big_int & 0xFFFF;
    
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE int test_subreg(void) {
    VOLATILE_VAR v4si vec_int = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    VOLATILE_VAR uint64_t big_val = 0x1122334455667788ULL;
    int result = 0;
    
    /* Type punning through casts - may generate SUBREG */
    float f = 3.14159f;
    uint32_t i;
    memcpy(&i, &f, sizeof(f));  /* Type punning */
    result += i & 0xFFFF;
    
    /* Direct cast (may generate SUBREG on some architectures) */
    short s = (short)big_val;
    result += s;
    
    /* Vector element extraction */
    int elem = vec_int[2];  /* Likely generates SUBREG */
    result += elem;
    
    /* Mixed vector operations */
    vec_short[3] = 99;
    result += vec_short[3];
    
    /* Float to int bitcast via union */
    union {
        float f;
        uint32_t i;
    } pun;
    pun.f = 2.71828f;
    result += pun.i;
    
    /* Subregister access via pointer */
    uint32_t *ptr32 = (uint32_t*)&big_val;
    result += ptr32[0];  /* Low 32 bits */
    result += ptr32[1];  /* High 32 bits */
    
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[4];
    struct nested *next;
};

NOINLINE int test_memory_operand(void) {
    VOLATILE_VAR int buffer[100];
    VOLATILE_VAR int *ptr1, **ptr2, ***ptr3;
    VOLATILE_VAR struct nested nodes[3];
    VOLATILE_VAR int index;
    int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) buffer[i] = i * 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) nodes[i].data[j] = i * 10 + j;
        nodes[i].next = (i < 2) ? &nodes[i+1] : NULL;
    }
    
    /* Multi-level pointer dereferencing */
    ptr1 = &buffer[10];
    ptr2 = &ptr1;
    ptr3 = &ptr2;
    result += ***ptr3;
    
    /* Complex array indexing with volatile index */
    index = global_index;
    result += buffer[index * 2 + 5];
    
    /* Structure field access with pointer chasing */
    result += nodes[0].next->next->data[2];
    
    /* Volatile memory operations */
    *(volatile int*)&buffer[50] = 999;
    result += buffer[50];
    
    /* Pointer arithmetic with multiple dereferences */
    int *p = buffer;
    p += get_index() % 50;
    result += *p;
    result += *(p + 5);
    
    /* Memory access through function return */
    void *vp = get_ptr();
    if (vp) result += *(int*)vp;
    
    /* Complex addressing mode simulation */
    result += buffer[(index << 2) + (result & 0xF)];
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) { return 42; }
void* get_ptr(void) { static int dummy = 0xABCD; return &dummy; }
