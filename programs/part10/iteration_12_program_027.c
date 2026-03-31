/* test_resources.c - Generate specific RTL patterns for GCC resource tracking coverage */

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

/* Global volatile variables to prevent optimization */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_packed {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int status:4;
    unsigned int reserved:20;
} __attribute__((packed));

struct nested_bitfield {
    struct {
        unsigned int low:8;
        unsigned int high:8;
    } bytes;
    unsigned int combined:16;
};

NOINLINE static unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packed bf = {0};
    VOLATILE_VAR struct nested_bitfield nbf = {0};
    VOLATILE_VAR unsigned int raw_value = 0xABCD1234;
    unsigned int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf.flag = 0x5;          /* 3-bit field */
    bf.value = 0x12 & 0x1F; /* 5-bit field with masking */
    bf.status = 0xA;        /* 4-bit field */
    
    /* Nested bitfield access */
    nbf.bytes.low = (raw_value >> 0) & 0xFF;
    nbf.bytes.high = (raw_value >> 8) & 0xFF;
    nbf.combined = (raw_value >> 16) & 0xFFFF;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    result |= (bf.flag << 0);
    result |= (bf.value << 3);
    result |= (bf.status << 8);
    
    /* Complex bitfield expression */
    unsigned int mask = 0x1F; /* 5-bit mask */
    unsigned int shift = global_index & 0x3; /* 0-3 shift */
    result |= ((raw_value >> shift) & mask) << 16;
    
    /* Bitfield in conditional */
    if (bf.flag & 0x4) {
        result |= 0x80000000;
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned int another = 0xCAFEBABE;
    unsigned int result = 0;
    
    /* Union for type punning - may generate low-part accesses */
    union {
        unsigned int full;
        unsigned char bytes[4];
        unsigned short halves[2];
    } u;
    u.full = wide_reg;
    
    /* Byte-sized stores into integers (x86: movb) */
    u.bytes[0] = 0xAA;  /* May generate STRICT_LOW_PART for byte store */
    u.bytes[2] = 0xBB;
    
    /* Half-word store */
    u.halves[1] = 0xCCDD;
    
    result = u.full;
    
    /* Pointer-based byte store - force low-part access */
    unsigned char* byte_ptr = (unsigned char*)&wide_reg;
    byte_ptr[1] = 0x77;  /* Modify second byte */
    
    /* Arithmetic that truncates to low part */
    unsigned int temp = wide_reg + another;
    unsigned char low_byte = temp & 0xFF;  /* Explicit truncation */
    result ^= (low_byte << 8);
    
    /* Inline assembly with low-byte modifier (x86-specific) */
    #if defined(__i386__) || defined(__x86_64__)
    unsigned int asm_in = 0x12345678;
    unsigned int asm_out;
    asm volatile (
        "movl %1, %0\n\t"
        "movb %%al, %b0\n\t"  /* %b0 accesses low byte - may generate STRICT_LOW_PART */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "eax"
    );
    result ^= asm_out;
    #endif
    
    return result ^ wide_reg;
}

/* ========== SUBREG patterns ========== */
NOINLINE static unsigned int test_subreg(void) {
    /* GCC vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    
    unsigned int result = 0;
    
    /* Vector element extraction - often uses SUBREG */
    result += vec[0];
    result += vec[global_index % 4];
    
    /* Type punning between different sizes */
    unsigned int int_val = 0x12345678;
    unsigned short short_val = (unsigned short)int_val;  /* Truncation */
    unsigned char char_val = (unsigned char)(int_val >> 16);
    
    result += short_val;
    result += char_val;
    
    /* Float/int bitcasting - may use SUBREG for reinterpretation */
    union {
        float f;
        unsigned int i;
    } float_union;
    float_union.f = 3.14159f;
    result ^= float_union.i;  /* Access same bits as different type */
    
    /* Mixed vector operations */
    short_vec[3] = int_val & 0xFFFF;  /* 32-bit to 16-bit subreg */
    result += short_vec[3];
    
    /* Complex subreg chain */
    unsigned long long ll_val = 0x1122334455667788ULL;
    unsigned int* ptr = (unsigned int*)&ll_val;
    result += ptr[0];  /* Access low 32 bits */
    result += ptr[1];  /* Access high 32 bits */
    
    return result;
}

/* ========== Complex Memory Operand patterns ========== */
struct nested_struct {
    int a;
    int b[3];
    struct {
        int x;
        int y;
    } inner;
};

NOINLINE static unsigned int test_memory_operand(void) {
    /* Complex memory hierarchy */
    static VOLATILE_VAR int buffer[256];
    static VOLATILE_VAR struct nested_struct nested[4];
    
    /* Initialize with non-zero values */
    for (int i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    for (int i = 0; i < 4; i++) {
        nested[i].a = i * 10;
        nested[i].b[0] = i * 20;
        nested[i].b[1] = i * 30;
        nested[i].b[2] = i * 40;
        nested[i].inner.x = i * 50;
        nested[i].inner.y = i * 60;
    }
    
    unsigned int result = 0;
    VOLATILE_VAR int idx = global_index;
    
    /* Multi-level pointer dereferencing */
    int* ptr1 = buffer;
    int** ptr2 = &ptr1;
    int*** ptr3 = &ptr2;
    
    result += ***ptr3;  /* Triple indirection */
    result += (*ptr2)[idx % 256];  /* Array through pointer */
    
    /* Complex structure addressing */
    result += nested[1].b[2];
    result += nested[idx % 4].inner.x;
    
    /* Pointer arithmetic with volatile */
    VOLATILE_VAR int* volatile_ptr = buffer + 128;
    result += *(volatile_ptr - 64);
    result += volatile_ptr[idx % 128];
    
    /* Memory operand with side effect in address calculation */
    result += buffer[get_index() % 256];
    
    /* Volatile memory operations that won't be optimized away */
    VOLATILE_VAR int volatile_target = 0;
    volatile_target = nested[2].inner.y;
    result += volatile_target;
    
    /* String operation that walks memory */
    char str_buf[64];
    strcpy(str_buf, "Complex memory access pattern");
    result += str_buf[idx % 64];
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Starting RTL pattern generation tests...\n");
    
    /* Initialize global volatile variables */
    global_index = 42;
    global_ptr = &checksum;
    
    /* Run all pattern tests */
    checksum ^= test_zero_extract();
    printf("  test_zero_extract completed\n");
    
    checksum ^= test_strict_low_part();
    printf("  test_strict_low_part completed\n");
    
    checksum ^= test_subreg();
    printf("  test_subreg completed\n");
    
    checksum ^= test_memory_operand();
    printf("  test_memory_operand completed\n");
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Dummy external functions to prevent optimization */
int get_index(void) {
    return rand() % 100;
}

void* get_ptr(void) {
    static int dummy;
    return &dummy;
}
