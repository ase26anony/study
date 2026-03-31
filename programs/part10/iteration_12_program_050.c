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
    unsigned int extra:8;
    unsigned int pad:16;
} NOINLINE;

struct bitfield_struct2 {
    unsigned long long high:32;
    unsigned long long low:32;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_struct bf;
    struct bitfield_struct2 bf2;
    VOLATILE_VAR unsigned int source = 0xABCD1234;
    VOLATILE_VAR unsigned long long source64 = 0xDEADBEEFCAFEBABEULL;
    
    /* Direct bitfield assignments - may generate ZERO_EXTRACT */
    bf.flag = (source >> 5) & 0x7;      /* Extract 3 bits */
    bf.value = (source >> 8) & 0x1F;    /* Extract 5 bits */
    bf.extra = source & 0xFF;           /* Extract 8 bits */
    
    /* 64-bit bitfield operations */
    bf2.low = source64 & 0xFFFFFFFF;
    bf2.high = (source64 >> 32) & 0xFFFFFFFF;
    
    /* Complex extraction with variable shift */
    VOLATILE_VAR int shift = global_index & 0x1F;
    unsigned int extracted = (source >> shift) & ((1 << 10) - 1);
    
    /* Return checksum */
    return bf.flag + bf.value + bf.extra + (unsigned int)bf2.low + 
           (unsigned int)bf2.high + extracted;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0x87654321;
    VOLATILE_VAR unsigned char* byte_ptr;
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    pun.full = wide_reg;
    pun.bytes[0] = 0xFF;                /* Modify low byte only */
    
    /* Pointer cast to access low byte */
    byte_ptr = (unsigned char*)&wide_reg;
    byte_ptr[0] = 0xAA;                 /* Direct byte store */
    
    /* Arithmetic truncation in context where high bits must be preserved */
    VOLATILE_VAR unsigned int temp = wide_reg;
    unsigned char low_byte = temp & 0xFF;  /* Explicit truncation */
    
    /* Inline assembly for explicit low-part access */
    unsigned int result;
    asm volatile (
        "movb %b1, %b0\n\t"             /* %b modifier accesses low byte */
        : "=r"(result)
        : "r"(wide_reg)
        : "cc"
    );
    
    return pun.full + wide_reg + low_byte + result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE_VAR int int_val = 0x12345678;
    VOLATILE_VAR short short_val;
    VOLATILE_VAR float float_val = 3.14159f;
    VOLATILE_VAR int int_from_float;
    
    /* Type punning through casts - may generate SUBREG */
    short_val = (short)int_val;         /* Truncating cast */
    
    /* Bitcast float to int */
    int_from_float = *(int*)&float_val; /* Type punning */
    
    /* Vector operations with element extraction */
    v4si vec = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Extract vector elements - often uses SUBREG */
    int elem0 = vec[0];
    short elem3 = vec_short[3];
    
    /* Complex subregister access pattern */
    union {
        v4si v;
        int a[4];
    } vec_union;
    vec_union.v = vec;
    int elem_via_union = vec_union.a[2];
    
    return short_val + int_from_float + elem0 + elem3 + elem_via_union;
}

/* ========== Complex Memory Operand patterns ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    /* Multi-level pointer structure */
    int buffer[16];
    int** ptr2ptr;
    int* ptr_array[4];
    struct nested node1, node2;
    
    /* Initialize */
    for (int i = 0; i < 16; i++) buffer[i] = i * 100;
    for (int i = 0; i < 4; i++) ptr_array[i] = &buffer[i * 2];
    
    ptr2ptr = &ptr_array[1];
    node1.next = &node2;
    node2.next = &node1;
    
    /* Complex addressing modes */
    VOLATILE_VAR int idx = global_index % 8;
    
    /* Multi-level dereference */
    int val1 = ***&ptr2ptr;             /* Triple indirection */
    
    /* Array indexing with volatile index */
    int val2 = buffer[idx * 2 + 1];
    
    /* Structure pointer chasing */
    int val3 = node1.next->next->data[2];
    
    /* Pointer arithmetic with complex offset */
    int* volatile_ptr = (int*)global_ptr;
    int val4 = volatile_ptr ? volatile_ptr[idx + 1] : 0;
    
    /* Volatile memory operation to prevent elimination */
    VOLATILE_VAR int volatile_target = 0;
    volatile_target = buffer[5];
    
    return val1 + val2 + val3 + val4 + volatile_target;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Initialize globals */
    global_index = 7;
    global_ptr = malloc(32);
    if (global_ptr) {
        memset(global_ptr, 0x42, 32);
    }
    
    /* Run all pattern tests */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_memory_operand();
    
    /* Cleanup */
    if (global_ptr) free(global_ptr);
    
    printf("Final checksum: 0x%08X\n", checksum);
    return 0;
}
