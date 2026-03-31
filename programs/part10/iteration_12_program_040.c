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
    unsigned short a:2;
    unsigned short b:6;
    unsigned short c:8;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_struct bf;
    struct packed_bitfields pb;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Initialize with volatile to prevent constant folding */
    bf.flag = global_index & 0x7;
    bf.value = (global_index >> 3) & 0x1F;
    bf.mode = (global_index >> 8) & 0xF;
    
    pb.a = global_index & 0x3;
    pb.b = (global_index >> 2) & 0x3F;
    pb.c = (global_index >> 8) & 0xFF;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    unsigned int temp = 0;
    temp |= (bf.flag << 16);
    temp |= (bf.value << 8);
    temp |= bf.mode;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int mask = (1 << bf.value) - 1;
    unsigned int extracted = (temp >> bf.flag) & mask;
    
    /* Another bitfield extraction pattern */
    unsigned int combined = (pb.a << 14) | (pb.b << 8) | pb.c;
    unsigned int field_b = (combined >> 8) & 0x3F;  /* Extract 6-bit field */
    
    result = extracted + field_b + temp;
    
    /* Complex bitfield operation in loop */
    for (VOLATILE_VAR int i = 0; i < 3; i++) {
        bf.flag = (bf.flag >> 1) | ((bf.value & 1) << 2);
        bf.value = (bf.value + i) & 0x1F;
    }
    
    return result + bf.flag;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0x12345678;
    VOLATILE_VAR unsigned char byte_var = 0;
    unsigned int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0xFF;
    
    /* Another byte store */
    unsigned int another = 0xABCDEF00;
    *(volatile unsigned char*)&another = global_index & 0xFF;
    
    /* Union for byte access */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } u;
    u.full = 0xDEADBEEF;
    u.bytes[1] = 0xCC;  /* Modify only low part of register */
    
    /* Arithmetic that truncates to byte */
    unsigned int temp = wide_reg + another;
    byte_var = temp & 0xFF;  /* Explicit truncation */
    
    /* Inline assembly forcing low-byte register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x87654321;
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (asm_out)
        : "r" (asm_in)
        : "cc"
    );
    
    /* Multiple byte operations */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        u.bytes[i] = (u.bytes[i] + i) & 0x7F;
    }
    
    result = wide_reg + another + u.full + asm_out + byte_var;
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    unsigned int result = 0;
    
    /* Vector element extraction - may generate SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    
    /* Type punning through casts */
    float f = 3.14159f;
    unsigned int float_bits;
    memcpy(&float_bits, &f, sizeof(float_bits));
    
    /* Short to int promotion */
    short s = -32768;
    int promoted = s;  /* Sign extension may use SUBREG */
    
    /* Mixed size operations */
    v8hi temp = short_vec + (v8hi){1, 1, 1, 1, 1, 1, 1, 1};
    short first = temp[0];
    
    /* Pointer casting for subregister access */
    unsigned long long big = 0x1122334455667788ULL;
    unsigned int lower = (unsigned int)big;  /* Truncation */
    unsigned int upper = (unsigned int)(big >> 32);
    
    /* Complex vector operation */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        vec[i] = vec[i] * 2 + i;
    }
    
    /* Extract from vector to scalar */
    result = elem0 + elem2 + float_bits + promoted + first + lower + upper;
    
    /* Additional type mixing */
    unsigned char bytes[4];
    memcpy(bytes, &result, 4);
    unsigned int reconstructed = (bytes[3] << 24) | (bytes[2] << 16) | 
                                 (bytes[1] << 8) | bytes[0];
    
    return reconstructed + vec[0];
}

/* ========== Memory operand patterns ========== */
struct nested {
    int data[3];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    VOLATILE_VAR int array[100];
    VOLATILE_VAR int* ptrs[10];
    unsigned int result = 0;
    
    /* Initialize with volatile index */
    for (VOLATILE_VAR int i = 0; i < 100; i++) {
        array[i] = i * 2 + global_index;
    }
    
    /* Multi-level pointer dereferencing */
    int*** triple_ptr = (int***)get_ptr();
    if (triple_ptr && *triple_ptr && **triple_ptr) {
        result += ***triple_ptr;
    }
    
    /* Complex array indexing */
    VOLATILE_VAR int idx1 = get_index() % 50;
    VOLATILE_VAR int idx2 = get_index() % 30;
    
    int val1 = array[idx1 * 2];
    int val2 = array[idx2 + 10];
    
    /* Structure with pointer chasing */
    struct nested node1, node2;
    node1.data[0] = 100;
    node1.data[1] = 200;
    node1.data[2] = 300;
    node1.next = &node2;
    
    node2.data[0] = 400;
    node2.data[1] = 500;
    node2.data[2] = 600;
    node2.next = &node1;
    
    /* Pointer chasing loop */
    struct nested* current = &node1;
    for (VOLATILE_VAR int i = 0; i < 3; i++) {
        result += current->data[i % 3];
        current = current->next;
    }
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = array + 50;
    int volatile_val = *volatile_ptr;
    *volatile_ptr = volatile_val + 1;
    
    /* Complex addressing mode */
    int* base = array;
    int offset = idx1 * sizeof(int);
    int complex_addr_val = *(int*)((char*)base + offset + 16);
    
    /* Multiple memory accesses in expression */
    result = result + val1 + val2 + complex_addr_val + array[global_index % 100];
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int total = 0;
    
    /* Initialize globals */
    global_index = 42;
    global_ptr = &global_index;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: %u\n", total);
    
    return (int)(total % 256);
}

/* External functions to prevent optimization */
int get_index(void) {
    static VOLATILE_VAR int counter = 0;
    return counter++ % 100;
}

void* get_ptr(void) {
    static VOLATILE_VAR int data = 0x1234;
    static VOLATILE_VAR int* ptr1 = &data;
    static VOLATILE_VAR int** ptr2 = &ptr1;
    static VOLATILE_VAR int*** ptr3 = &ptr2;
    return (void*)ptr3;
}
