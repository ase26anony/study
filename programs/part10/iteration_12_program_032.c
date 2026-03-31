/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void escape(void*);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_s {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int extra:8;
    unsigned int pad:16;
} NOINLINE;

struct bitfield_large {
    unsigned long long low:20;
    unsigned long long high:20;
    unsigned long long middle:24;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_s bf1 = {0};
    struct bitfield_s bf2 = {0};
    struct bitfield_large bfl = {0};
    VOLATILE_VAR unsigned int temp = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = 5;
    bf1.value = 20;
    bf2.flag = bf1.value & 0x7;  /* Extract low 3 bits */
    bf2.value = (bf1.flag << 2) | 0x1;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int val = 0xABCD1234;
    unsigned int extracted = (val >> 8) & 0xFFF;  /* Extract bits 8-19 */
    bf1.extra = extracted & 0xFF;
    
    /* Large bitfield operations */
    bfl.low = 0x12345;
    bfl.high = 0x6789A;
    bfl.middle = (bfl.low >> 4) | (bfl.high << 16);
    
    /* Complex bitfield expression */
    temp = ((bf1.flag << 16) | (bf1.value << 8) | bf1.extra);
    temp ^= (bfl.low & 0xFFFFF);
    
    return temp + bf2.flag + bf2.value;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int int_var = 0xDEADBEEF;
    VOLATILE_VAR unsigned short short_var = 0;
    VOLATILE_VAR unsigned char byte_var = 0;
    
    /* Byte-sized store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&int_var = 0x42;
    
    /* Union for type punning with byte access */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x12345678;
    pun.bytes[1] = 0xAA;  /* Modify only one byte */
    
    /* Truncation operations that preserve high bits */
    unsigned int source = 0x87654321;
    unsigned char low_byte = source & 0xFF;  /* Explicit truncation */
    unsigned short low_word = source & 0xFFFF;
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x89ABCDEF;
    asm volatile (
        "movb %b1, %0\n\t"  /* %b1 accesses low byte of register */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        pun.bytes[i] = (source >> (i * 8)) & 0xFF;
    }
    
    return int_var + pun.full + low_byte + low_word + asm_out;
}

/* ========== SUBREG patterns ========== */
NOINLINE unsigned int test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    short selem = vec_short[3];
    
    /* Type punning between different sizes */
    unsigned int int_val = 0x3F800000;  /* 1.0f in IEEE 754 */
    float float_val;
    memcpy(&float_val, &int_val, sizeof(float_val));
    
    /* Cast between different integer sizes */
    unsigned long long big_val = 0x1122334455667788ULL;
    unsigned int small_val = (unsigned int)big_val;  /* Truncation */
    unsigned short smaller_val = (unsigned short)small_val;
    
    /* Mixed size operations */
    vec_int[1] = (int)vec_short[4] * 2;
    vec_short[5] = (short)(vec_int[3] / 2);
    
    /* Complex expression with mixed types */
    unsigned int result = elem0 + elem2 + selem + small_val + smaller_val;
    result += (unsigned int)float_val;
    
    return result;
}

/* ========== Complex memory operand patterns ========== */
NOINLINE unsigned int test_memory_operand(void) {
    /* Complex pointer structure */
    VOLATILE_VAR unsigned int buffer[64];
    VOLATILE_VAR unsigned int* ptr1 = buffer;
    VOLATILE_VAR unsigned int** ptr2 = &ptr1;
    VOLATILE_VAR unsigned int*** ptr3 = &ptr2;
    
    /* Initialize buffer with volatile writes */
    for (VOLATILE_VAR int i = 0; i < 64; i++) {
        buffer[i] = i * 0x1001;
    }
    
    /* Multi-level pointer dereferencing */
    unsigned int val1 = ***ptr3;
    (**ptr2)++;
    (*ptr1) = val1 ^ 0x55555555;
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index % 64;
    unsigned int val2 = buffer[idx];
    unsigned int val3 = buffer[idx + 1];
    unsigned int val4 = buffer[idx * 2];
    
    /* Structure with pointer chasing */
    struct node {
        unsigned int data;
        struct node* next;
    };
    
    struct node nodes[4];
    for (int i = 0; i < 3; i++) {
        nodes[i].data = i * 0x11111111;
        nodes[i].next = &nodes[i + 1];
    }
    nodes[3].data = 0x33333333;
    nodes[3].next = NULL;
    
    /* Pointer chasing through structure */
    unsigned int sum = 0;
    struct node* current = &nodes[0];
    while (current) {
        sum += current->data;
        current = current->next;
    }
    
    /* Volatile memory operations */
    VOLATILE_VAR unsigned int* volatile_ptr = buffer;
    volatile_ptr[10] = val2;
    volatile_ptr[20] = val3;
    
    /* Complex address calculation */
    unsigned int* addr = &buffer[0] + (val1 & 0xF);
    *addr = sum;
    
    return val1 + val2 + val3 + val4 + sum + *addr;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing resource pattern generation...\n");
    
    /* Run all tests and accumulate results */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_memory_operand();
    
    /* Force side effects to be observable */
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Prevent dead code elimination */
    escape(&checksum);
    
    return (checksum == 0) ? 1 : 0;
}

/* Dummy external functions */
int get_index(void) { return rand() % 100; }
void escape(void* p) { asm volatile ("" : : "r"(p) : "memory"); }
