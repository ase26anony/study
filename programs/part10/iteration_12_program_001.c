#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* Opaque function to prevent constant propagation */
extern int get_index(void);
extern void* get_ptr(void);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_struct {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int data:8;
    unsigned int extra:16;
} NOINLINE;

NOINLINE int test_zero_extract(void) {
    struct bitfield_struct bf;
    VOLATILE_VAR unsigned int raw = 0xDEADBEEF;
    int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf.flag = (raw >> 5) & 0x7;        /* Extract 3 bits */
    bf.value = (raw >> 8) & 0x1F;      /* Extract 5 bits */
    bf.data = (raw >> 13) & 0xFF;      /* Extract 8 bits */
    
    /* Complex bitfield extraction with shifting */
    unsigned int mask = 0x3F;
    unsigned int shift = get_index() & 0xF;
    bf.extra = (raw >> shift) & mask;  /* Variable extraction */
    
    /* Multiple extractions in sequence */
    result += bf.flag;
    result += bf.value << 3;
    result += bf.data << 8;
    result += bf.extra << 16;
    
    /* Bitfield in conditional */
    if (bf.flag & 0x4) {
        result ^= bf.value;
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_int = 0x12345678;
    VOLATILE_VAR uint8_t byte_store;
    int result = 0;
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&wide_int = 0xFF;          /* Store low byte */
    result += wide_int;
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } pun;
    pun.full = 0xAABBCCDD;
    pun.bytes[0] = get_index() & 0xFF;            /* Low byte store */
    result += pun.full;
    
    /* Multiple byte operations */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        uint8_t* ptr = (uint8_t*)&wide_int + i;
        *ptr = (*ptr + 1) & 0x7F;                 /* Modify individual bytes */
    }
    result += wide_int;
    
    /* Arithmetic truncation preserving high bits */
    uint32_t source = 0x87654321;
    uint8_t low_byte = source & 0xFF;             /* Explicit truncation */
    result += low_byte;
    
    /* Inline assembly for explicit low-part access */
    uint32_t asm_in = 0xDEADBEEF;
    uint32_t asm_out;
    asm volatile (
        "movb %b1, %0\n\t"        /* %b1 = low byte of input register */
        : "=r" (asm_out)
        : "r" (asm_in)
        : "cc"
    );
    result += asm_out;
    
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE int test_subreg(void) {
    int result = 0;
    
    /* Vector operations with element extraction */
    v4si vec = {1, 2, 3, 4};
    vec += (v4si){5, 6, 7, 8};
    
    /* Extract vector elements - may generate SUBREG */
    result += vec[0];      /* Access 32-bit element from 128-bit vector */
    result += vec[2];
    
    /* Type punning between different sizes */
    uint32_t word = 0x12345678;
    uint16_t half = *(uint16_t*)&word;    /* Cast to smaller type */
    result += half;
    
    /* Float/int bitcasting */
    float f = 3.14159f;
    uint32_t bits = *(uint32_t*)&f;       /* Type punning */
    result += bits & 0xFFFF;
    
    /* Mixed vector types */
    v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    short_vec[3] = get_index() & 0x7FFF;  /* Modify 16-bit element */
    result += short_vec[3];
    
    /* Structure with mixed types */
    struct mixed {
        int a;
        short b;
        char c;
    } m = {100, 200, 300};
    m.b = m.a & 0xFFFF;    /* Truncate int to short */
    result += m.b + m.c;
    
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE int test_memory_operand(void) {
    VOLATILE_VAR int result = 0;
    
    /* Multi-level pointer dereferencing */
    int*** triple_ptr = (int***)malloc(sizeof(int**));
    *triple_ptr = (int**)malloc(sizeof(int*));
    **triple_ptr = (int*)malloc(sizeof(int));
    ***triple_ptr = 42;
    result += ***triple_ptr;
    
    /* Clean up */
    free(**triple_ptr);
    free(*triple_ptr);
    free(triple_ptr);
    
    /* Complex array indexing with volatile */
    int array[100];
    for (VOLATILE_VAR int i = 0; i < 100; i++) {
        array[i] = i * 2;
    }
    
    VOLATILE_VAR int idx1 = get_index() % 100;
    VOLATILE_VAR int idx2 = get_index() % 50;
    result += array[idx1] + array[idx2 * 2];
    
    /* Structure field chasing */
    struct nested n1, n2;
    n1.data[0] = 100;
    n1.data[1] = 200;
    n1.next = &n2;
    n2.data[0] = 300;
    n2.next = NULL;
    
    result += n1.next->data[0];      /* Complex memory addressing */
    result += n1.data[global_index & 0x3];
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = (int*)malloc(sizeof(int) * 10);
    for (VOLATILE_VAR int i = 0; i < 10; i++) {
        volatile_ptr[i] = i * 3;
    }
    result += volatile_ptr[global_index % 10];
    free((void*)volatile_ptr);
    
    /* Pointer arithmetic with multiple dereferences */
    int buffer[20];
    int* ptr = buffer + 5;
    *ptr = 999;
    ptr += get_index() % 10;
    result += *ptr;
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Initialize global variables */
    global_index = 42;
    global_ptr = malloc(100);
    if (global_ptr) memset(global_ptr, 0, 100);
    
    /* Run all tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    /* Clean up */
    if (global_ptr) free(global_ptr);
    
    printf("Total checksum: %d\n", total);
    printf("All tests completed.\n");
    
    return total != 0 ? 0 : 1;
}

/* Opaque function implementations */
int get_index(void) {
    static VOLATILE_VAR int counter = 0;
    return ++counter;
}

void* get_ptr(void) {
    static char buffer[256];
    return buffer + (get_index() % 200);
}
