#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void* get_ptr(void);

/* Global volatile variables to force memory operations */
VOLATILE int global_index = 0;
VOLATILE void* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_struct {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int reserved:20;
} NOINLINE;

struct mixed_bitfields {
    unsigned short low:5;
    unsigned short high:11;
    unsigned int extended:20;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_struct bf1 = {0};
    struct mixed_bitfields bf2 = {0};
    VOLATILE unsigned int raw_value = 0xABCD1234;
    unsigned int result = 0;
    
    /* Direct bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = (raw_value >> 0) & 0x7;
    bf1.value = (raw_value >> 3) & 0x1F;
    bf1.mode = (raw_value >> 8) & 0xF;
    
    /* Bitfield extraction through multiple steps */
    bf2.low = bf1.value & 0x1F;
    bf2.high = (bf1.mode << 3) | bf1.flag;
    
    /* Explicit bit extraction that may compile to ZERO_EXTRACT */
    unsigned int extracted = (raw_value >> 12) & 0xFFF;
    result = bf2.low | (bf2.high << 5) | (extracted << 16);
    
    /* Complex bitfield operation with masking */
    struct {
        unsigned int a:7;
        unsigned int b:9;
        unsigned int c:16;
    } bf3;
    
    bf3.a = (result >> 0) & 0x7F;
    bf3.b = (result >> 7) & 0x1FF;
    bf3.c = (result >> 16) & 0xFFFF;
    
    return bf3.a + bf3.b + bf3.c;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE unsigned int wide_reg = 0xDEADBEEF;
    unsigned int result = 0;
    
    /* Byte-sized store into integer - may generate STRICT_LOW_PART */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } u;
    u.full = wide_reg;
    
    /* Modify individual bytes while keeping others intact */
    u.bytes[0] = 0xAA;  /* Low byte store */
    u.bytes[2] = 0xBB;  /* Third byte store */
    
    /* Pointer casting for byte access */
    *(volatile unsigned char*)&wide_reg = 0xCC;
    
    /* Arithmetic truncation that must preserve high bits */
    unsigned int temp = wide_reg;
    unsigned char low_byte = temp & 0xFF;  /* Explicit truncation */
    
    /* Inline assembly forcing low-part register access */
    unsigned int asm_out;
    unsigned int asm_in = 0x12345678;
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (asm_out)
        : "r" (asm_in)
        : "cc"
    );
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        u.bytes[i] = (u.bytes[i] + low_byte) & 0xFF;
    }
    
    result = u.full + asm_out + temp;
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    VOLATILE v4si vec = {1, 2, 3, 4};
    VOLATILE v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    unsigned int result = 0;
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    
    /* Type punning through unions */
    union {
        float f;
        int i;
        short s[2];
    } pun;
    
    pun.f = 3.14159f;
    result += pun.i;          /* SUBREG from float to int */
    result += pun.s[0];       /* SUBREG from int to short */
    
    /* Explicit casts between different sizes */
    long long big = 0x1122334455667788LL;
    int truncated = (int)big;           /* SUBREG from 64 to 32 bits */
    short halved = (short)truncated;    /* SUBREG from 32 to 16 bits */
    
    /* Mixed vector operations */
    short_vec[1] = elem0;
    short_vec[3] = halved;
    
    /* Access vector as different type */
    int* as_ints = (int*)&vec;
    result += as_ints[1] + as_ints[3];
    
    /* Complex subregister chain */
    unsigned long long val64 = 0xAABBCCDDEEFF1122ULL;
    unsigned int val32 = (unsigned int)val64;
    unsigned short val16 = (unsigned short)val32;
    unsigned char val8 = (unsigned char)val16;
    
    result += val32 + val16 + val8 + truncated;
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[3];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    VOLATILE int buffer[256];
    VOLATILE int* ptr1 = buffer;
    VOLATILE int** ptr2 = &ptr1;
    VOLATILE int*** ptr3 = &ptr2;
    unsigned int result = 0;
    
    /* Initialize buffer with values */
    for (int i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    
    /* Multi-level pointer dereferencing */
    result += ***ptr3;                     /* Triple pointer dereference */
    result += **(ptr2 + global_index);     /* Complex addressing */
    
    /* Complex array indexing with volatile index */
    VOLATILE int idx = get_index() % 256;
    result += buffer[idx] + buffer[idx * 2 % 256];
    
    /* Structure with pointer chasing */
    struct nested node1, node2, node3;
    node1.data[0] = 100; node1.data[1] = 200; node1.data[2] = 300;
    node2.data[0] = 400; node2.data[1] = 500; node2.data[2] = 600;
    node3.data[0] = 700; node3.data[1] = 800; node3.data[2] = 900;
    
    node1.next = &node2;
    node2.next = &node3;
    node3.next = &node1;
    
    /* Complex memory access pattern */
    struct nested* current = &node1;
    for (int i = 0; i < 5; i++) {
        result += current->data[i % 3];
        current = current->next;
    }
    
    /* Volatile memory operations that can't be optimized away */
    VOLATILE int* volatile_ptr = (VOLATILE int*)get_ptr();
    if (volatile_ptr) {
        result += *volatile_ptr;
    }
    
    /* Mixed indexing modes */
    result += *(buffer + idx) + buffer[idx + 1] + 3[buffer];
    
    return result;
}

/* ========== Main test driver ========== */
int get_index(void) {
    static int counter = 0;
    return counter++ % 100;
}

void* get_ptr(void) {
    static char buffer[1024];
    return buffer;
}

int main(void) {
    unsigned int total = 0;
    
    printf("Starting RTL pattern tests...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    printf("Zero-extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict-low-part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: %u\n", total);
    
    /* Force use of all results */
    if (total > 0) {
        return 0;
    }
    return 1;
}
