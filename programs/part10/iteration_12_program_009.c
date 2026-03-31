/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void use_value(int val);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char *volatile global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE static int test_zero_extract(void) {
    /* Bitfield structures that may compile to ZERO_EXTRACT */
    struct bitfield1 {
        unsigned int flag:3;
        unsigned int value:5;
        unsigned int mode:4;
    } bf1;
    
    struct bitfield2 {
        unsigned long long high:32;
        unsigned long long low:24;
        unsigned long long extra:8;
    } bf2;
    
    VOLATILE_VAR unsigned int source = 0xABCD1234;
    VOLATILE_VAR unsigned long long source64 = 0xDEADBEEFCAFEBABEULL;
    
    /* Bitfield assignments - may generate ZERO_EXTRACT for stores */
    bf1.flag = (source >> 0) & 0x7;
    bf1.value = (source >> 3) & 0x1F;
    bf1.mode = (source >> 8) & 0xF;
    
    bf2.high = (source64 >> 32) & 0xFFFFFFFF;
    bf2.low = source64 & 0xFFFFFF;
    bf2.extra = (source64 >> 56) & 0xFF;
    
    /* Explicit bit extraction that may use ZERO_EXTRACT */
    unsigned int extracted1 = (source >> 5) & 0x3FF;  /* 10-bit field */
    unsigned int extracted2 = (source >> 15) & 0x7FFF; /* 15-bit field */
    
    /* Bitfield extraction from structure */
    unsigned int combined = (bf1.flag << 16) | (bf1.value << 8) | bf1.mode;
    
    /* Complex bit manipulation */
    unsigned int mask = 0x00FF00FF;
    unsigned int masked = (source & mask) | ((source >> 8) & mask);
    
    return bf1.flag + bf1.value + bf1.mode + 
           (bf2.high & 0xFF) + (bf2.low & 0xFF) + bf2.extra +
           extracted1 + extracted2 + combined + masked;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int int_var = 0x12345678;
    VOLATILE_VAR unsigned short short_var = 0xABCD;
    VOLATILE_VAR unsigned char byte_var = 0xEF;
    
    /* Byte-sized stores into integers - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&int_var = 0xFF;
    *(volatile unsigned char*)((char*)&int_var + 1) = 0xEE;
    *(volatile unsigned char*)((char*)&int_var + 2) = 0xDD;
    *(volatile unsigned char*)((char*)&int_var + 3) = 0xCC;
    
    /* Union for type punning - may generate low-part accesses */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    
    pun.full = 0xAABBCCDD;
    pun.bytes[0] = byte_var;  /* Low byte store */
    pun.bytes[1] = byte_var + 1;
    
    /* Truncation operations preserving high bits */
    unsigned int temp = int_var;
    unsigned char low_byte = temp & 0xFF;  /* May use STRICT_LOW_PART */
    unsigned char next_byte = (temp >> 8) & 0xFF;
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_in = 0x87654321;
    unsigned int asm_out;
    
    /* x86-specific: %b0 modifier accesses low byte */
    asm volatile (
        "movb %b1, %b0\n\t"
        "movb %b1, %%al\n\t"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "%al"
    );
    
    /* Mixed-size operations */
    short_var = int_var & 0xFFFF;  /* Truncate to 16 bits */
    byte_var = short_var & 0xFF;   /* Truncate to 8 bits */
    
    return int_var + short_var + byte_var + pun.full + low_byte + next_byte + asm_out;
}

/* ========== SUBREG patterns ========== */
NOINLINE static int test_subreg(void) {
    /* Vector extensions for SUBREG accesses */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec_int[0];
    int elem1 = vec_int[1];
    short selem0 = vec_short[0];
    short selem1 = vec_short[1];
    
    /* Type punning through casts - may generate SUBREG */
    float float_val = 3.14159f;
    int int_val;
    
    /* Bitcast through union */
    union {
        float f;
        int i;
    } converter;
    
    converter.f = float_val;
    int_val = converter.i;  /* Type punning */
    
    /* Explicit size-changing casts */
    long long big_val = 0x1122334455667788LL;
    int truncated = (int)big_val;  /* SUBREG for truncation */
    short halved = (short)truncated;
    char quartered = (char)halved;
    
    /* Mixed-type arithmetic with subregisters */
    short s1 = 1000;
    short s2 = 2000;
    int promoted = s1 * s2;  /* Promotions use SUBREG */
    
    /* Pointer casting for subregister access */
    int array[4] = {100, 200, 300, 400};
    short *short_view = (short*)array;
    short first_half = short_view[0];  /* Accesses low 16 bits of int */
    short second_half = short_view[1]; /* Accesses high 16 bits */
    
    return elem0 + elem1 + selem0 + selem1 + int_val + truncated + 
           halved + quartered + promoted + first_half + second_half;
}

/* ========== Memory operand patterns ========== */
NOINLINE static int test_memory_operand(void) {
    /* Complex memory addressing modes */
    VOLATILE_VAR int buffer[256];
    VOLATILE_VAR int *ptr1 = buffer;
    VOLATILE_VAR int **ptr2 = &ptr1;
    VOLATILE_VAR int ***ptr3 = &ptr2;
    
    /* Initialize buffer */
    for (int i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    
    /* Multi-level pointer dereferencing */
    int val1 = ***ptr3;
    int val2 = **(ptr2 + global_index % 64);
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = get_index() % 256;
    int val3 = buffer[idx];
    int val4 = buffer[idx + 1];
    int val5 = buffer[idx * 2];
    
    /* Structure with nested arrays */
    struct nested {
        int data[16][16];
        int *pointers[8];
    } nested_struct;
    
    /* Initialize nested structure */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            nested_struct.data[i][j] = i * 100 + j;
        }
    }
    
    /* Complex structure field access */
    int val6 = nested_struct.data[5][5];
    int val7 = nested_struct.data[idx % 16][(idx + 1) % 16];
    
    /* Pointer arithmetic with multiple bases */
    int *arr_ptr = buffer + 128;
    int val8 = *(arr_ptr - 64);
    int val9 = *(arr_ptr + idx);
    
    /* Volatile memory operations */
    VOLATILE_VAR int volatile_mem[100];
    for (int i = 0; i < 100; i++) {
        volatile_mem[i] = i * 7;
    }
    
    int val10 = volatile_mem[50];
    volatile_mem[25] = val10 * 2;
    
    return val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 + val9 + val10;
}

/* ========== Main test driver ========== */
int main(void) {
    int total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Total checksum: %d\n", total);
    
    /* Use the result to prevent dead code elimination */
    if (total > 0) {
        printf("All tests executed successfully\n");
    }
    
    return 0;
}

/* External functions to prevent optimization */
int get_index(void) {
    static int counter = 0;
    return counter++ % 100;
}

void use_value(int val) {
    /* Prevent optimization */
    asm volatile ("" : : "r"(val));
}
