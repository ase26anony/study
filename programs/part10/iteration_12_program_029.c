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
NOINLINE unsigned int test_zero_extract(void) {
    /* Bitfield structure - may compile to ZERO_EXTRACT */
    struct bitfield_struct {
        unsigned int flag:3;
        unsigned int value:5;
        unsigned int mode:4;
        unsigned int reserved:20;
    } bf;
    
    /* Union for bitfield access */
    union {
        struct bitfield_struct bf;
        unsigned int raw;
    } u;
    
    VOLATILE_VAR unsigned int temp = 0x12345678;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    u.raw = temp;
    u.bf.flag = (temp >> 2) & 0x7;      /* Extract bits 2-4 */
    u.bf.value = (temp >> 5) & 0x1F;    /* Extract bits 5-9 */
    u.bf.mode = (temp >> 10) & 0xF;     /* Extract bits 10-13 */
    
    /* Explicit bit extraction that may use ZERO_EXTRACT */
    unsigned int extracted = 0;
    extracted |= ((temp >> 3) & 0x1) << 0;   /* Bit 3 -> position 0 */
    extracted |= ((temp >> 7) & 0x3) << 1;   /* Bits 7-8 -> positions 1-2 */
    extracted |= ((temp >> 15) & 0x7) << 3;  /* Bits 15-17 -> positions 3-5 */
    
    /* Complex extraction with variable shift */
    VOLATILE_VAR int shift = get_index() & 0x1F;
    unsigned int var_extract = (temp >> shift) & ((1 << 8) - 1);
    
    return u.raw + extracted + var_extract;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned char byte_val = 0xCC;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    
    pun.full = wide_reg;
    
    /* Store to low byte while preserving high bytes */
    pun.bytes[0] = byte_val;                     /* Direct byte store */
    *(volatile unsigned char*)&wide_reg = 0xAA;  /* Cast pointer store */
    
    /* Arithmetic truncation to byte */
    unsigned char truncated = (wide_reg & 0xFF);  /* May need low part */
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        pun.bytes[i] = (wide_reg >> (i * 8)) & 0xFF;
    }
    
    /* Inline assembly forcing low-byte register access */
    unsigned int asm_result;
    asm volatile (
        "movb %1, %b0\n\t"        /* %b0 = low byte of output register */
        "andb $0xF0, %b0\n\t"     /* Operate on low byte */
        : "=r"(asm_result)
        : "r"(byte_val)
        : "cc"
    );
    
    return pun.full + truncated + asm_result;
}

/* ========== SUBREG patterns ========== */
NOINLINE unsigned int test_subreg(void) {
    /* Vector extension for SUBREG accesses */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Type punning between different sizes */
    VOLATILE_VAR int int_val = 0x12345678;
    short short_val = (short)int_val;           /* Truncation cast */
    char char_val = (char)int_val;              /* Further truncation */
    
    /* Union for type punning */
    union {
        float f;
        int i;
        short s[2];
    } float_int;
    
    float_int.f = 3.14159f;
    short_val = float_int.s[0];                 /* Access subpart */
    
    /* Vector element extraction */
    int elem0 = vec_int[0];                     /* May use SUBREG */
    short elem3 = vec_short[3];
    
    /* Mixed size operations */
    vec_int[1] = short_val;                     /* Store smaller into vector element */
    vec_short[2] = (short)float_int.i;          /* Type conversion store */
    
    /* Pointer casting for subregister access */
    int* int_ptr = &int_val;
    short* short_ptr = (short*)int_ptr;         /* Aliased access */
    short_ptr[0] = 0xABCD;                      /* Modify low part */
    
    return int_val + short_val + char_val + elem0 + elem3;
}

/* ========== Complex Memory Operand patterns ========== */
NOINLINE unsigned int test_memory_operand(void) {
    /* Multi-level pointer structure */
    static VOLATILE_VAR int data[256];
    static VOLATILE_VAR int* ptr_array[16];
    static VOLATILE_VAR int** ptr_to_ptr[8];
    
    /* Initialize pointer chains */
    for (int i = 0; i < 256; i++) data[i] = i;
    for (int i = 0; i < 16; i++) ptr_array[i] = &data[i * 16];
    for (int i = 0; i < 8; i++) ptr_to_ptr[i] = &ptr_array[i * 2];
    
    VOLATILE_VAR int idx1 = get_index() & 7;
    VOLATILE_VAR int idx2 = get_index() & 15;
    VOLATILE_VAR int idx3 = get_index() & 255;
    
    /* Complex addressing modes */
    int val1 = ***ptr_to_ptr;                   /* Triple dereference */
    int val2 = ptr_array[idx1][idx2];           /* 2D array access */
    int val3 = *(*(ptr_to_ptr[2]) + 3);         /* Pointer arithmetic + deref */
    
    /* Structure with nested arrays */
    struct nested {
        int matrix[4][4];
        int* pointers[4];
        struct nested* next;
    } node;
    
    /* Initialize structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            node.matrix[i][j] = i * 4 + j;
        }
        node.pointers[i] = &data[i * 16];
    }
    node.next = NULL;
    
    /* Complex structure access chain */
    int val4 = node.matrix[1][2];
    int val5 = *node.pointers[3];
    int val6 = node.pointers[2][5];
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = &data[0];
    int val7 = *volatile_ptr;
    volatile_ptr = &data[128];
    int val8 = *volatile_ptr;
    
    /* Pointer chasing loop */
    int sum = 0;
    int* current = &data[0];
    for (int i = 0; i < 10; i++) {
        sum += *current;
        current = &data[(current - data + 5) % 256];
    }
    
    return val1 + val2 + val3 + val4 + val5 + val6 + val7 + val8 + sum;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Run all pattern tests */
    checksum += test_zero_extract();
    printf("Zero extract test completed\n");
    
    checksum += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    checksum += test_subreg();
    printf("Subreg test completed\n");
    
    checksum += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    return (checksum != 0) ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 256;
}

void* get_ptr(void) {
    static char buffer[1024];
    return buffer;
}
