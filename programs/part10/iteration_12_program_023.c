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
    struct bitfield_s bf1 = {0};
    struct packed_bitfield bf2 = {0};
    VOLATILE_VAR unsigned int raw_val = 0xDEADBEEF;
    int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = (raw_val >> 3) & 0x7;  /* Explicit extraction */
    bf1.value = (raw_val >> 8) & 0x1F;
    
    /* Nested bitfield operations */
    bf2.a = bf1.flag & 0x3;
    bf2.b = (bf1.value << 2) | (bf1.flag >> 1);
    
    /* Complex extraction with variable shift */
    unsigned int mask = 0xFF;
    int shift = global_index & 0xF;
    bf2.c = (raw_val >> shift) & mask;
    
    /* Combine results */
    result = bf1.flag + bf1.value + bf2.a + bf2.b + bf2.c;
    
    /* Another ZERO_EXTRACT pattern: extracting bit ranges */
    unsigned int x = 0x12345678;
    unsigned int y = (x >> 12) & 0xFFF;  /* Extract 12 bits */
    result += y;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_reg = 0x12345678;
    VOLATILE_VAR uint32_t wide_reg2 = 0x9ABCDEF0;
    int result = 0;
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0xFF;
    result += wide_reg;
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } u;
    u.full = 0x87654321;
    u.bytes[0] = 0xAA;  /* Low byte store */
    result += u.full;
    
    /* Truncation preserving high bits context */
    uint8_t low_byte = wide_reg2 & 0xFF;
    /* Force use in computation that might keep high bits */
    wide_reg2 = (wide_reg2 & 0xFFFFFF00) | (low_byte + 1);
    result += wide_reg2;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        u.bytes[i] = (u.bytes[i] + i) & 0xFF;
    }
    result += u.full;
    
    /* Inline assembly forcing low-part access on x86 */
    #if defined(__i386__) || defined(__x86_64__)
    uint32_t asm_in = 0x11223344;
    uint32_t asm_out;
    asm volatile (
        "movb %%al, %1\n\t"
        "movl %1, %0"
        : "=r"(asm_out)
        : "m"(*(volatile uint8_t*)&asm_in), "a"(asm_in)
        : "memory"
    );
    result += asm_out;
    #endif
    
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE int test_subreg(void) {
    /* Vector extensions for SUBREG patterns */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec1 = {1, 2, 3, 4};
    v8hi vec2 = {10, 20, 30, 40, 50, 60, 70, 80};
    int result = 0;
    
    /* Vector element extraction - often SUBREG */
    int elem0 = vec1[0];
    int elem2 = vec1[2];
    result += elem0 + elem2;
    
    /* Type punning between different sizes */
    uint32_t dword = 0x12345678;
    uint16_t word = *(volatile uint16_t*)&dword;  /* Low word */
    result += word;
    
    /* Float/int reinterpretation */
    float f = 3.14159f;
    uint32_t int_bits = *(volatile uint32_t*)&f;
    result += int_bits & 0xFFFF;
    
    /* Mixed vector operations */
    vec2[3] = vec1[1];  /* int -> short truncation */
    result += vec2[3];
    
    /* Complex subregister access through pointer */
    uint64_t qword = 0x0123456789ABCDEFULL;
    uint32_t* dword_ptr = (uint32_t*)&qword;
    result += dword_ptr[0] + dword_ptr[1];
    
    /* Union for subregister access */
    union {
        double d;
        uint32_t i[2];
    } du;
    du.d = 2.71828;
    result += du.i[0] ^ du.i[1];
    
    return result;
}

/* ========== Memory operand patterns ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE int test_memory_operand(void) {
    VOLATILE_VAR int buffer[64];
    VOLATILE_VAR int* ptr1 = buffer;
    VOLATILE_VAR int** ptr2 = &ptr1;
    VOLATILE_VAR int*** ptr3 = &ptr2;
    int result = 0;
    
    /* Initialize buffer */
    for (int i = 0; i < 64; i++) {
        buffer[i] = i * 3;
    }
    
    /* Complex pointer dereferencing */
    result += ***ptr3;  /* Triple indirection */
    
    /* Array indexing with volatile index */
    VOLATILE_VAR int idx = global_index % 64;
    result += buffer[idx] + buffer[idx + 1];
    
    /* Structure with pointer chasing */
    struct nested n1, n2;
    n1.data[0] = 100;
    n1.data[1] = 200;
    n1.next = &n2;
    n2.data[0] = 300;
    n2.data[1] = 400;
    n2.next = NULL;
    
    result += n1.next->data[0];  /* Structure field through pointer */
    
    /* Multi-level array access */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    VOLATILE_VAR int row = get_index() % 8;
    VOLATILE_VAR int col = get_index() % 8;
    result += matrix[row][col];
    
    /* Volatile memory operations */
    *(volatile int*)(buffer + 16) = 0xABCD;
    result += *(volatile int*)(buffer + 16);
    
    /* Pointer arithmetic with complex addressing */
    int* p = buffer + 32;
    result += p[global_index & 3] + p[(global_index + 1) & 3];
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    int total = 0;
    
    printf("Testing resource patterns...\n");
    
    /* Initialize globals */
    global_index = 42;
    global_ptr = malloc(256);
    if (global_ptr) {
        memset(global_ptr, 0xAA, 256);
    }
    
    /* Run all pattern tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    printf("Result checksum: %d\n", total);
    
    if (global_ptr) {
        free(global_ptr);
    }
    
    return total != 0 ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 64;
}

void* get_ptr(void) {
    static char buffer[128];
    return buffer;
}
