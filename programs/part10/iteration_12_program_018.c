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
    VOLATILE_VAR struct bitfield_large bf2 = {0};
    VOLATILE_VAR unsigned int result = 0;
    
    /* Direct bitfield assignments - may generate ZERO_EXTRACT */
    bf.flag = 5;
    bf.value = get_index() & 0x1F;
    bf.mode = 3;
    
    /* Bitfield extraction via masking */
    unsigned int raw = *(unsigned int*)&bf;
    unsigned int extracted = (raw >> 3) & 0x1F;  /* Extract 'value' field */
    
    /* Large bitfield operations */
    bf2.high = 0xDEADBEEF;
    bf2.low = 0xCAFEBABE;
    
    /* Complex bitfield extraction */
    unsigned long long combined = *(unsigned long long*)&bf2;
    unsigned int high_part = (combined >> 32) & 0xFFFFFFFF;
    unsigned int low_part = combined & 0xFFFFFFFF;
    
    /* Multiple extractions in sequence */
    result = bf.flag + extracted + high_part + low_part;
    
    /* Bitfield in loop - forces multiple ZERO_EXTRACT operations */
    for (VOLATILE_VAR int i = 0; i < 3; i++) {
        bf.flag = (bf.flag + 1) & 0x7;
        result += (raw >> (i * 2)) & 0x3;
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int int_var = 0x12345678;
    VOLATILE_VAR unsigned short short_var = 0;
    VOLATILE_VAR unsigned char char_var = 0;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&int_var = 0xFF;
    result += int_var;
    
    /* Union for type punning with byte access */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x87654321;
    pun.bytes[1] = get_index() & 0xFF;  /* Modify only one byte */
    result += pun.full;
    
    /* Truncation preserving high bits context */
    unsigned int temp = 0x89ABCDEF;
    char_var = temp & 0xFF;  /* Explicit truncation */
    result += char_var;
    
    /* Multiple byte operations */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        *(volatile unsigned char*)((char*)&int_var + i) = i * 0x11;
    }
    result += int_var;
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x5555AAAA;
    asm volatile (
        "movb %b1, %b0\n\t"
        "movw %w1, %w0"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    result += asm_out;
    
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR int int_val = 0x12345678;
    VOLATILE_VAR short short_val = 0;
    VOLATILE_VAR float float_val = 3.14159f;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Type casting between different sizes */
    short_val = (short)int_val;  /* May generate SUBREG */
    result += short_val;
    
    /* Float/int bitcasting */
    unsigned int float_bits = *(unsigned int*)&float_val;
    result += float_bits;
    
    /* GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element extraction */
    int elem = vec[get_index() & 0x3];
    short short_elem = vec_short[get_index() & 0x7];
    
    result += elem + short_elem;
    
    /* Mixed vector operations */
    vec[0] = short_elem;  /* Store short into int vector element */
    result += vec[0];
    
    /* Pointer casting with different types */
    unsigned char* byte_ptr = (unsigned char*)&int_val;
    for (int i = 0; i < 4; i++) {
        result += byte_ptr[i];
    }
    
    return result;
}

/* ========== Memory operand patterns ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    VOLATILE_VAR unsigned int buffer[16];
    VOLATILE_VAR unsigned int** ptr_array;
    VOLATILE_VAR struct nested complex_struct[3];
    VOLATILE_VAR unsigned int result = 0;
    
    /* Initialize structures */
    for (int i = 0; i < 16; i++) {
        buffer[i] = i * 0x11111111;
    }
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            complex_struct[i].data[j] = i * 10 + j;
        }
        if (i < 2) complex_struct[i].next = &complex_struct[i + 1];
        else complex_struct[i].next = NULL;
    }
    
    /* Multi-level pointer dereferencing */
    unsigned int* ptr1 = buffer;
    unsigned int** ptr2 = &ptr1;
    unsigned int*** ptr3 = &ptr2;
    
    result += ***ptr3;  /* Triple dereference */
    result += **(ptr3 + 0);  /* Complex addressing */
    
    /* Array indexing with volatile index */
    VOLATILE_VAR int idx = get_index() & 0xF;
    result += buffer[idx];
    result += buffer[idx + 1];
    
    /* Structure field access with pointer chasing */
    struct nested* current = &complex_struct[0];
    while (current) {
        result += current->data[get_index() & 0x3];
        current = current->next;
    }
    
    /* Complex addressing modes */
    result += buffer[(idx * 3 + 7) & 0xF];
    
    /* Volatile memory operations */
    *(volatile unsigned int*)(buffer + 8) = 0xDEADBEEF;
    result += *(volatile unsigned int*)(buffer + 8);
    
    /* Pointer arithmetic with multiple bases */
    unsigned int* base1 = buffer;
    unsigned int* base2 = buffer + 8;
    result += base1[idx] + base2[idx];
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: 0x%08X\n", total);
    
    return 0;
}

/* Dummy external functions */
int get_index(void) {
    static volatile int counter = 0;
    return ++counter;
}

void* get_ptr(void) {
    static volatile char buffer[64];
    return buffer;
}
