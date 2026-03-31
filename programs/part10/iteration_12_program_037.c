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
extern void escape(void*);

/* Global volatile variables to prevent optimization */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ====== ZERO_EXTRACT patterns ====== */
struct bitfield_packed {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int status:8;
    unsigned int reserved:12;
} __attribute__((packed));

struct bitfield_nested {
    struct {
        unsigned int low:8;
        unsigned int high:8;
    } bytes;
    unsigned int combined:16;
};

NOINLINE static unsigned int test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packed bf1 = {0};
    VOLATILE_VAR struct bitfield_nested bf2 = {0};
    VOLATILE_VAR unsigned int temp = 0xABCD1234;
    unsigned int result = 0;
    
    /* Direct bitfield assignments - may generate ZERO_EXTRACT */
    bf1.flag = 5;
    bf1.value = 20;
    bf1.mode = 9;
    bf1.status = 0xFF;
    
    /* Bitfield extraction via masking */
    result = (temp >> 8) & 0xFF;           /* Extract byte */
    result |= ((temp >> 16) & 0xF) << 8;   /* Extract nibble */
    
    /* Complex bitfield manipulation */
    bf2.bytes.low = temp & 0xFF;
    bf2.bytes.high = (temp >> 8) & 0xFF;
    bf2.combined = (bf2.bytes.high << 8) | bf2.bytes.low;
    
    /* Multiple extractions in sequence */
    for (int i = 0; i < 4; i++) {
        unsigned int mask = (1 << (i * 4)) - 1;
        result ^= (temp >> (i * 4)) & mask;
    }
    
    /* Return checksum */
    return result + bf1.flag + bf1.value + bf2.combined;
}

/* ====== STRICT_LOW_PART patterns ====== */
NOINLINE static unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned char byte_store;
    unsigned int result = 0;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x12345678;
    
    /* Byte-sized stores into integers - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0xAA;        /* Store byte */
    pun.bytes[1] = 0xBB;                               /* Store into union */
    
    /* Truncation operations preserving high bits */
    byte_store = wide_reg & 0xFF;                      /* Extract low byte */
    result = byte_store;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        pun.bytes[i] = (wide_reg >> (i * 8)) & 0xFF;
        result += pun.bytes[i];
    }
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_in = 0x87654321;
    unsigned int asm_out;
    
    /* x86-specific: %b0 accesses low byte of register */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    result += asm_out;
    
    /* More complex truncation */
    unsigned short half_word;
    half_word = wide_reg & 0xFFFF;                     /* Extract low 16 bits */
    result += half_word;
    
    return result + pun.full;
}

/* ====== SUBREG patterns ====== */
NOINLINE static unsigned int test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    VOLATILE_VAR v4si vec_int = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    unsigned int result = 0;
    
    /* Type punning via casts - may generate SUBREG */
    float float_val = 3.14159f;
    int int_val;
    
    /* Bitcast float to int */
    memcpy(&int_val, &float_val, sizeof(float_val));
    result += int_val;
    
    /* Cast between different integer sizes */
    short short_val = 32767;
    int widened = (int)short_val;          /* Sign extension */
    result += widened;
    
    /* Vector element extraction */
    for (int i = 0; i < 4; i++) {
        int elem = vec_int[i];              /* May generate SUBREG */
        result += elem;
    }
    
    /* Mixed vector operations */
    v8hi shifted = vec_short << 1;
    for (int i = 0; i < 8; i++) {
        result += shifted[i];
    }
    
    /* Pointer casting for subregister access */
    unsigned long long big = 0x1122334455667788ULL;
    unsigned int* ptr = (unsigned int*)&big;
    result += ptr[0];                       /* Low 32 bits */
    result += ptr[1];                       /* High 32 bits */
    
    return result;
}

/* ====== Memory operand patterns ====== */
struct nested_struct {
    int data[4];
    struct nested_struct* next;
};

NOINLINE static unsigned int test_memory_operand(void) {
    /* Complex memory addressing modes */
    VOLATILE_VAR int array[256];
    VOLATILE_VAR struct nested_struct nodes[4];
    VOLATILE_VAR int*** triple_ptr;
    unsigned int result = 0;
    
    /* Initialize structures */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            nodes[i].data[j] = i * 10 + j;
        }
        if (i < 3) nodes[i].next = &nodes[i + 1];
        else nodes[i].next = &nodes[0];
    }
    
    /* Multi-level pointer dereferencing */
    int** ptr2 = malloc(sizeof(int*) * 4);
    for (int i = 0; i < 4; i++) {
        ptr2[i] = &array[i * 16];
    }
    triple_ptr = &ptr2;
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index % 64;
    result += array[idx];                           /* Volatile index */
    result += array[idx + 64];                      /* Offset indexing */
    
    /* Pointer chasing through structure */
    struct nested_struct* current = &nodes[0];
    for (int i = 0; i < 8; i++) {
        result += current->data[i % 4];             /* Structure field access */
        current = current->next;
    }
    
    /* Triple pointer dereference */
    result += ***triple_ptr;
    
    /* Volatile memory operations */
    *(volatile int*)(&array[128]) = result;
    result += *(volatile int*)(&array[128]);
    
    /* Complex address calculation */
    int offset = get_index() % 32;
    result += *(int*)((char*)&nodes[0] + offset);
    
    /* Cleanup */
    free(ptr2);
    
    return result;
}

/* ====== Main function ====== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all tests */
    checksum += test_zero_extract();
    printf("  test_zero_extract complete\n");
    
    checksum += test_strict_low_part();
    printf("  test_strict_low_part complete\n");
    
    checksum += test_subreg();
    printf("  test_subreg complete\n");
    
    checksum += test_memory_operand();
    printf("  test_memory_operand complete\n");
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Use result to prevent dead code elimination */
    if (checksum == 0xDEADBEEF) {
        printf("Impossible!\n");
    }
    
    return 0;
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 100;
}

void escape(void* p) {
    asm volatile ("" : : "r"(p) : "memory");
}
