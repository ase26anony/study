/* test_resource_patterns.c */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void* get_ptr(void);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE unsigned int test_zero_extract(void) {
    /* Bitfield structure that may compile to ZERO_EXTRACT */
    struct bitfield_pack {
        unsigned int flag:3;
        unsigned int value:5;
        unsigned int mode:4;
        unsigned int reserved:20;
    } pack;
    
    /* Union for bitfield access */
    union {
        struct bitfield_pack bf;
        unsigned int raw;
    } u;
    
    VOLATILE_VAR unsigned int source = 0xDEADBEEF;
    
    /* Bitfield assignments - may generate ZERO_EXTRACT */
    u.raw = source;
    u.bf.flag = (source >> 5) & 0x7;      /* Explicit extraction */
    u.bf.value = (source >> 8) & 0x1F;     /* Another extraction */
    u.bf.mode = (source >> 13) & 0xF;      /* And another */
    
    /* Manual bitfield extraction using masking */
    unsigned int extracted = 0;
    extracted |= ((source >> 3) & 0x7) << 0;   /* 3-bit field at offset 3 */
    extracted |= ((source >> 10) & 0x1F) << 3; /* 5-bit field at offset 10 */
    extracted |= ((source >> 20) & 0xF) << 8;  /* 4-bit field at offset 20 */
    
    /* Complex bitfield expression */
    unsigned int composite = 
        ((source & 0xFF) << 16) | 
        (((source >> 16) & 0xFF) << 8) | 
        ((source >> 24) & 0xFF);
    
    return u.raw + extracted + composite;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0x12345678;
    VOLATILE_VAR unsigned char byte_var = 0;
    
    /* Byte-sized store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0xFF;
    
    /* Union for byte access */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } converter;
    
    converter.full = 0xAABBCCDD;
    converter.bytes[1] = 0x11;  /* Modify only low part of the 32-bit value */
    
    /* Truncation operation preserving high bits */
    unsigned int temp = wide_reg;
    byte_var = temp & 0xFF;      /* Explicit truncation to low byte */
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        converter.bytes[i] = (wide_reg >> (i * 8)) & 0xFF;
    }
    
    /* Inline assembly forcing low-byte register access */
    unsigned int asm_result;
    unsigned int asm_input = 0x87654321;
    
    /* x86-specific: %b0 modifier accesses low byte of register */
    asm volatile (
        "movl %1, %0\n\t"
        "movb $0x42, %%al\n\t"
        "movb %%al, %b0"
        : "=r"(asm_result)
        : "r"(asm_input)
        : "al"
    );
    
    return wide_reg + byte_var + converter.full + asm_result;
}

/* ========== SUBREG patterns ========== */
NOINLINE unsigned int test_subreg(void) {
    /* Vector extensions for SUBREG patterns */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Type punning through casts - may generate SUBREG */
    unsigned int int_val = 0xDEADBEEF;
    unsigned short short_val = (unsigned short)int_val;  /* Truncating cast */
    unsigned char char_val = (unsigned char)int_val;     /* Further truncation */
    
    /* Float/int bitcasting */
    float float_val = 3.14159f;
    unsigned int float_as_int;
    memcpy(&float_as_int, &float_val, sizeof(float_val));
    
    /* Vector element extraction */
    int elem0 = vec_int[0];      /* May use SUBREG to access vector element */
    short elem3 = vec_short[3];  /* Another subregister access */
    
    /* Mixed-size operations */
    vec_int[2] = short_val;      /* Store short into int vector element */
    vec_short[5] = char_val;     /* Store char into short vector element */
    
    /* Pointer casting for subregister access */
    unsigned long long big_val = 0x123456789ABCDEF0ULL;
    unsigned int* ptr_to_low = (unsigned int*)&big_val;
    unsigned int low_part = *ptr_to_low;  /* Access low 32 bits of 64-bit value */
    
    return int_val + short_val + char_val + float_as_int + 
           elem0 + elem3 + low_part + vec_int[0] + vec_short[0];
}

/* ========== Complex Memory Operand patterns ========== */
NOINLINE unsigned int test_memory_operand(void) {
    /* Complex memory addressing structures */
    struct level3 { int data; };
    struct level2 { struct level3* l3; int pad[3]; };
    struct level1 { struct level2* l2; int pad[7]; };
    struct root { struct level1* l1; int pad[15]; };
    
    /* Allocate and initialize test structures */
    struct level3 l3_obj = { .data = 42 };
    struct level2 l2_obj = { .l3 = &l3_obj };
    struct level1 l1_obj = { .l2 = &l2_obj };
    struct root root_obj = { .l1 = &l1_obj };
    
    VOLATILE_VAR struct root* volatile_root = &root_obj;
    
    /* Multi-level pointer dereferencing */
    int result = 0;
    result = volatile_root->l1->l2->l3->data;
    
    /* Array with volatile index */
    VOLATILE_VAR int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    VOLATILE_VAR int idx = global_index % 100;
    result += array[idx];           /* Non-constant array index */
    result += array[idx + 1];       /* Another non-constant access */
    result += array[idx * 2];       /* More complex addressing */
    
    /* Pointer arithmetic with multiple dereferences */
    int** double_ptr = (int**)malloc(sizeof(int*) * 10);
    for (int i = 0; i < 10; i++) {
        double_ptr[i] = (int*)malloc(sizeof(int) * 5);
        for (int j = 0; j < 5; j++) {
            double_ptr[i][j] = i * 10 + j;
        }
    }
    
    VOLATILE_VAR int idx2 = (global_index * 3) % 10;
    result += **(double_ptr + idx2);           /* Double dereference */
    result += *(double_ptr[idx2] + 2);         /* Array + offset */
    
    /* Cleanup */
    for (int i = 0; i < 10; i++) {
        free(double_ptr[i]);
    }
    free(double_ptr);
    
    /* Structure field with complex address calculation */
    struct large_struct {
        int header[8];
        int data[64];
        int footer[8];
    } large;
    
    VOLATILE_VAR struct large_struct* large_ptr = &large;
    result += large_ptr->data[global_index % 64];
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Initialize global volatile variables */
    global_index = 42;
    global_ptr = malloc(100);
    if (global_ptr) {
        memset(global_ptr, 0xAA, 100);
    }
    
    /* Execute all pattern tests */
    checksum += test_zero_extract();
    printf("  Zero-extract test completed\n");
    
    checksum += test_strict_low_part();
    printf("  Strict-low-part test completed\n");
    
    checksum += test_subreg();
    printf("  Subreg test completed\n");
    
    checksum += test_memory_operand();
    printf("  Memory operand test completed\n");
    
    /* Cleanup */
    if (global_ptr) free(global_ptr);
    
    printf("Final checksum: 0x%08X\n", checksum);
    printf("All tests completed. Check RTL dumps for ZERO_EXTRACT, STRICT_LOW_PART, SUBREG patterns.\n");
    
    return 0;
}

/* Dummy external functions */
int get_index(void) { return 73; }
void* get_ptr(void) { return malloc(1); }
