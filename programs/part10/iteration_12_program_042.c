/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */

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

NOINLINE unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_s bf;
    VOLATILE_VAR struct packed_bitfield pbf;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Initialize */
    bf.flag = 5;
    bf.value = 17;  /* Will be truncated to 5 bits */
    bf.mode = 9;
    
    pbf.a = 2;
    pbf.b = 42;
    pbf.c = 128;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    unsigned int temp = bf.value;
    result += temp;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int combined = (bf.flag << 8) | bf.value;
    unsigned int extracted = (combined >> 3) & 0x1F;  /* Extract 5 bits */
    result += extracted;
    
    /* Multiple extractions */
    for (VOLATILE_VAR int i = 0; i < 3; i++) {
        unsigned int mask = (1 << (i + 1)) - 1;
        unsigned int bits = (combined >> i) & mask;
        result += bits;
    }
    
    /* Packed bitfield operations */
    pbf.b = (pbf.a << 1) | (pbf.c & 0x3);
    result += pbf.b;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_val = 0x12345678;
    VOLATILE_VAR unsigned char byte_val = 0;
    VOLATILE_VAR unsigned short half_val = 0;
    VOLATILE_VAR unsigned int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_val = 0xFF;
    result += wide_val;
    
    /* Another byte store */
    ((volatile unsigned char*)&wide_val)[1] = 0xAA;
    result += wide_val;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0xDEADBEEF;
    pun.bytes[2] = 0xCC;  /* Modify only one byte */
    result += pun.full;
    
    /* Truncation operations that preserve high bits */
    unsigned int source = 0x87654321;
    byte_val = source & 0xFF;  /* Explicit truncation */
    result += byte_val;
    
    /* Multiple truncations in loop */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        half_val = (source >> (i * 8)) & 0xFFFF;
        result += half_val;
    }
    
    /* Inline assembly forcing low-part access on x86 */
    #if defined(__i386__) || defined(__x86_64__)
    unsigned int asm_in = 0x11223344;
    unsigned int asm_out;
    asm volatile (
        "movb %%al, %1\n\t"
        "movb $0x99, %%al\n\t"
        "movb %%al, %0"
        : "=m"(asm_out), "+m"(byte_val)
        : "a"(asm_in)
        : "memory"
    );
    result += asm_out + byte_val;
    #endif
    
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR unsigned int result = 0;
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    result += elem0 + elem2;
    
    /* Type punning between different sizes */
    unsigned int int_val = 0x3F800000;  /* 1.0f in float */
    float float_val;
    memcpy(&float_val, &int_val, sizeof(float));
    result += (unsigned int)(float_val * 100.0f);
    
    /* Cast between short and int */
    unsigned short short_val = 0xABCD;
    unsigned int extended = (unsigned int)short_val;  /* Zero extension */
    result += extended;
    
    /* Sign extension */
    short signed_short = -100;
    int signed_int = (int)signed_short;  /* Sign extension */
    result += (unsigned int)signed_int;
    
    /* Mixed vector operations */
    short_vec[3] = vec[1] + short_vec[2];
    result += short_vec[3];
    
    /* Pointer casting for subregister access */
    unsigned long long big_val = 0x123456789ABCDEF0ULL;
    unsigned int* ptr = (unsigned int*)&big_val;
    result += ptr[0] + ptr[1];  /* Access halves of 64-bit value */
    
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    /* Complex memory addressing */
    VOLATILE_VAR int array[100];
    VOLATILE_VAR int* ptr_array[10];
    VOLATILE_VAR struct nested nodes[5];
    VOLATILE_VAR unsigned int result = 0;
    
    /* Initialize */
    for (int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    for (int i = 0; i < 10; i++) {
        ptr_array[i] = &array[i * 7];
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 4; j++) {
            nodes[i].data[j] = i * 10 + j;
        }
        if (i < 4) nodes[i].next = &nodes[i + 1];
        else nodes[i].next = NULL;
    }
    
    /* Multi-level pointer dereferencing */
    int*** triple_ptr = (int***)malloc(sizeof(int**));
    int** double_ptr = (int**)malloc(sizeof(int*));
    *double_ptr = &array[42];
    *triple_ptr = double_ptr;
    
    result += ***triple_ptr;  /* Complex memory access */
    
    /* Array indexing with volatile index */
    VOLATILE_VAR int idx = global_index % 50;
    result += array[idx] + array[idx + 25];
    
    /* Structure field access with pointer chasing */
    struct nested* current = &nodes[0];
    for (int i = 0; i < 3 && current != NULL; i++) {
        result += current->data[1];
        current = current->next;
    }
    
    /* Volatile memory operations */
    volatile int* volatile_ptr = &array[75];
    result += *volatile_ptr;
    *volatile_ptr = result & 0xFF;
    
    /* Complex address calculation */
    int offset = get_index() % 20;
    result += *(ptr_array[3] + offset);
    
    /* Cleanup */
    free(double_ptr);
    free(triple_ptr);
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing resource pattern generation...\n");
    
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

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 100;
}

void* get_ptr(void) {
    static char buffer[256];
    return buffer;
}
