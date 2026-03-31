/* test_resource_patterns.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int opaque(int x);
extern void* opaque_ptr(void* p);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char global_byte = 0;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_pack {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int reserved:20;
} NOINLINE;

struct bitfield_pack2 {
    unsigned short low:7;
    unsigned short high:9;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_pack bf = {0};
    VOLATILE_VAR struct bitfield_pack2 bf2 = {0};
    VOLATILE_VAR unsigned int raw = 0xDEADBEEF;
    unsigned int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf.flag = 5;
    bf.value = 17;
    bf.mode = 9;
    
    /* Extract bitfields manually - may generate ZERO_EXTRACT */
    result |= (bf.flag << 0);
    result |= (bf.value << 3);
    result |= (bf.mode << 8);
    
    /* Bit masking operations */
    bf2.low = (raw >> 4) & 0x7F;  /* Could generate ZERO_EXTRACT */
    bf2.high = (raw >> 11) & 0x1FF;
    
    /* Complex bitfield extraction */
    unsigned int mask = (1 << 5) - 1;
    unsigned int extracted = (raw >> global_index) & mask;
    result ^= extracted;
    
    /* Multiple bitfield operations */
    struct {
        unsigned int a:2, b:2, c:2, d:2;
    } packed = {0};
    
    packed.a = (raw >> 0) & 3;
    packed.b = (raw >> 2) & 3;
    packed.c = (raw >> 4) & 3;
    packed.d = (raw >> 6) & 3;
    
    result += packed.a + packed.b + packed.c + packed.d;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0x12345678;
    VOLATILE_VAR unsigned short half_reg = 0;
    VOLATILE_VAR unsigned char byte_reg = 0;
    unsigned int result = 0;
    
    /* Byte-sized store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0xFF;
    result += wide_reg;
    
    /* Another byte store */
    ((volatile unsigned char*)&wide_reg)[1] = 0xAA;
    result += wide_reg;
    
    /* Union for type punning - byte access to wider type */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x87654321;
    pun.bytes[2] = global_byte;  /* Byte store to wider register */
    result += pun.full;
    
    /* Truncation that preserves high bits */
    half_reg = wide_reg & 0xFFFF;  /* May generate low-part access */
    result += half_reg;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        ((volatile unsigned char*)&wide_reg)[i] = i * 0x11;
    }
    result += wide_reg;
    
    /* Inline assembly forcing low-byte register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x89ABCDEF;
    asm volatile (
        "movb %b1, %0\n\t"  /* %b1 accesses low byte of register */
        : "=r" (asm_out)
        : "r" (asm_in)
        : "cc"
    );
    result += asm_out;
    
    /* More assembly with different constraints */
    unsigned short asm_word;
    asm volatile (
        "movw %w1, %0\n\t"  /* %w1 accesses low word (16-bit) */
        : "=r" (asm_word)
        : "r" (asm_in)
        : "cc"
    );
    result += asm_word;
    
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE unsigned int test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    unsigned int result = 0;
    
    /* Extract vector elements - generates SUBREG */
    result += vec[0];
    result += vec[global_index % 4];
    result += vec_short[3];
    
    /* Type punning between different sizes */
    VOLATILE_VAR int int_val = 0x44332211;
    VOLATILE_VAR short short_val;
    
    /* Cast between different integer sizes */
    short_val = (short)int_val;  /* May generate SUBREG */
    result += short_val;
    
    /* Access through different pointer types */
    unsigned char* byte_ptr = (unsigned char*)&int_val;
    for (int i = 0; i < 4; i++) {
        result += byte_ptr[i];
    }
    
    /* Float/int bitcasting */
    VOLATILE_VAR float f = 3.14159f;
    VOLATILE_VAR int i;
    
    /* Type punning through union */
    union {
        float f;
        int i;
    } converter;
    converter.f = f;
    i = converter.i;  /* Generates SUBREG on some targets */
    result += i;
    
    /* Pointer casting for sub-register access */
    unsigned long long big = 0x1122334455667788ULL;
    unsigned int* half = (unsigned int*)&big;
    result += half[0] + half[1];
    
    /* Mixed size operations */
    char small = 127;
    int promoted = small * 2;  /* Promotion generates SUBREG */
    result += promoted;
    
    return result;
}

/* ========== Complex Memory Operand patterns ========== */
struct nested {
    int data[3];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    /* Complex memory addressing modes */
    VOLATILE_VAR int array[100];
    VOLATILE_VAR int* ptrs[10];
    VOLATILE_VAR struct nested nodes[5];
    unsigned int result = 0;
    
    /* Initialize data */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        ptrs[i] = &array[i * 10];
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 3; j++) {
            nodes[i].data[j] = i * 10 + j;
        }
        nodes[i].next = (i < 4) ? &nodes[i + 1] : NULL;
    }
    
    /* Multi-level pointer dereferencing */
    int*** triple_ptr = (int***)opaque_ptr(ptrs);
    if (triple_ptr && *triple_ptr && **triple_ptr) {
        result += ***triple_ptr;
    }
    
    /* Complex array indexing with volatile index */
    result += array[global_index % 100];
    result += ptrs[global_index % 10][global_index % 10];
    
    /* Structure field access with pointer chasing */
    struct nested* current = &nodes[0];
    for (int i = 0; i < 3 && current; i++) {
        result += current->data[i % 3];
        current = current->next;
    }
    
    /* Memory access with complex address calculation */
    int offset = opaque(global_index);
    result += *(volatile int*)((char*)array + offset * sizeof(int));
    
    /* Multiple indirections */
    int** double_ptr = &ptrs[2];
    result += (*double_ptr)[3];
    
    /* Volatile memory operations that can't be optimized away */
    volatile int* volatile_ptr = array;
    for (int i = 0; i < 10; i++) {
        result += volatile_ptr[opaque(i) % 20];
    }
    
    /* Stack-relative addressing with frame pointer */
    int local_array[20];
    for (int i = 0; i < 20; i++) {
        local_array[i] = i * 7;
    }
    result += local_array[global_index % 20];
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Run all tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    /* Use opaque function to prevent dead code elimination */
    total = opaque(total);
    
    printf("Result checksum: 0x%08X\n", total);
    
    /* Return non-zero if any test failed (simplified check) */
    return (total == 0) ? 1 : 0;
}

/* Opaque function definitions to prevent optimization */
int opaque(int x) {
    static volatile int state = 0;
    state ^= x;
    return state;
}

void* opaque_ptr(void* p) {
    static volatile intptr_t holder = 0;
    holder ^= (intptr_t)p;
    return (void*)holder;
}
