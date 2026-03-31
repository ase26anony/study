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

NOINLINE int test_zero_extract(void) {
    struct bitfield_struct bf;
    struct packed_bitfields pb;
    VOLATILE_VAR unsigned int raw_value = 0xDEADBEEF;
    int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf.flag = (raw_value >> 3) & 0x7;      /* Extract 3 bits */
    bf.value = (raw_value >> 6) & 0x1F;    /* Extract 5 bits */
    bf.mode = (raw_value >> 11) & 0xF;     /* Extract 4 bits */
    
    /* More complex bitfield operations */
    pb.a = (raw_value >> 0) & 0x3;
    pb.b = (raw_value >> 2) & 0x3F;
    pb.c = (raw_value >> 8) & 0xFF;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int extracted = (raw_value >> global_index) & ((1 << 5) - 1);
    
    /* Combine results to prevent dead code elimination */
    result = bf.flag + bf.value + bf.mode + pb.a + pb.b + pb.c + extracted;
    
    /* Bitfield in conditional */
    if ((raw_value >> 16) & 0x1) {
        result ^= 0x55;
    }
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_int = 0x12345678;
    VOLATILE_VAR uint16_t half_int = 0;
    VOLATILE_VAR uint8_t byte_val = 0;
    int result = 0;
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&wide_int = 0xFF;  /* Store byte, preserve high bits */
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } pun;
    pun.full = 0xAABBCCDD;
    pun.bytes[1] = 0x11;  /* Modify only one byte */
    
    /* Truncation that might preserve high bits in register */
    half_int = wide_int & 0xFFFF;
    byte_val = (wide_int >> 8) & 0xFF;
    
    /* Inline assembly forcing low-part register access */
    uint32_t asm_in = 0x87654321;
    uint32_t asm_out;
    asm volatile (
        "movb %b1, %b0\n\t"    /* %b modifier accesses low byte */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    /* More explicit byte operations */
    for (int i = 0; i < 4; i++) {
        ((volatile uint8_t*)&wide_int)[i] = i * 0x11;
    }
    
    result = wide_int + half_int + byte_val + pun.full + asm_out;
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE int test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    int result = 0;
    
    /* Type punning between different sizes */
    uint32_t int_val = 0x12345678;
    uint16_t short_val;
    uint8_t byte_val;
    
    /* Casts that may generate SUBREG */
    short_val = (uint16_t)int_val;          /* 32-bit to 16-bit */
    byte_val = (uint8_t)int_val;            /* 32-bit to 8-bit */
    
    /* Float/int bitcasting */
    float f = 3.14159f;
    uint32_t float_bits;
    memcpy(&float_bits, &f, sizeof(float_bits));
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    short selem = short_vec[global_index % 8];
    
    /* Mixed size operations */
    result = int_val + short_val + byte_val + float_bits + elem0 + elem2 + selem;
    
    /* More complex SUBREG patterns */
    struct {
        int a;
        short b;
        char c;
    } mixed;
    mixed.a = 1000;
    mixed.b = 500;
    mixed.c = 100;
    
    /* Access through different sized pointers */
    int* ip = &mixed.a;
    short* sp = (short*)ip;
    char* cp = (char*)ip;
    
    result += *ip + *sp + *cp;
    
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE int test_memory_operand(void) {
    VOLATILE_VAR int buffer[256];
    VOLATILE_VAR int* ptr1 = buffer;
    VOLATILE_VAR int** ptr2 = &ptr1;
    VOLATILE_VAR int*** ptr3 = &ptr2;
    int result = 0;
    
    /* Initialize buffer */
    for (int i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    
    /* Complex pointer dereferencing */
    result += ***ptr3;                     /* Triple pointer dereference */
    result += *(*(ptr2) + global_index);   /* Pointer arithmetic + deref */
    
    /* Multi-level array indexing */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;
        }
    }
    
    VOLATILE_VAR int idx1 = global_index % 8;
    VOLATILE_VAR int idx2 = (global_index + 1) % 8;
    result += matrix[idx1][idx2];
    
    /* Structure with pointer chasing */
    struct nested n1, n2, n3;
    n1.data[0] = 100; n1.next = &n2;
    n2.data[0] = 200; n2.next = &n3;
    n3.data[0] = 300; n3.next = NULL;
    
    result += n1.next->next->data[0];      /* Multi-level struct access */
    
    /* Volatile memory operations */
    volatile int* volatile_ptr = buffer;
    for (int i = 0; i < 10; i++) {
        result += volatile_ptr[i * 7];     /* Non-constant stride */
    }
    
    /* Address computation with multiple components */
    result += buffer[global_index * 2 + 1];
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Initialize global variables */
    global_index = 42;
    global_ptr = malloc(1024);
    if (global_ptr) {
        memset(global_ptr, 0xAA, 1024);
    }
    
    /* Run all tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    /* Cleanup */
    if (global_ptr) free(global_ptr);
    
    printf("Total checksum: %d\n", total);
    printf("All tests completed successfully\n");
    
    return total != 0 ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) { return 7; }
void* get_ptr(void) { return malloc(16); }
