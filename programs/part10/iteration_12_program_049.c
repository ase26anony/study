/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

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

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfields {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int data:20;
} NOINLINE;

/* Test function for ZERO_EXTRACT RTL */
NOINLINE static unsigned int test_zero_extract(void) {
    VOLATILE struct bitfields bf;
    VOLATILE unsigned int raw_value;
    unsigned int result = 0;
    
    /* Initialize */
    bf.flag = 5;
    bf.value = 17;  /* Will be truncated to 5 bits */
    bf.mode = 9;
    bf.data = 123456;
    
    /* Operations that may generate ZERO_EXTRACT */
    
    /* 1. Direct bitfield assignment */
    raw_value = bf.value;
    result += raw_value;
    
    /* 2. Bitfield extraction via masking */
    VOLATILE unsigned int packed = 0xABCD1234;
    unsigned int extracted = (packed >> 8) & 0xFFF;  /* 12-bit extraction */
    result += extracted;
    
    /* 3. Multiple extractions with different widths */
    for (VOLATILE int i = 0; i < 3; i++) {
        unsigned int mask = (1 << (i + 4)) - 1;
        unsigned int bits = (packed >> (i * 4)) & mask;
        result += bits;
    }
    
    /* 4. Bitfield in conditional */
    if (bf.flag & 0x4) {
        result += 1000;
    }
    
    /* 5. Complex extraction pattern */
    VOLATILE unsigned int source = 0xDEADBEEF;
    unsigned int low_nibble = source & 0xF;
    unsigned int high_byte = (source >> 24) & 0xFF;
    unsigned int middle_bits = (source >> 12) & 0x7FF;
    
    result = result + low_nibble + high_byte + middle_bits;
    
    return result;
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Union for byte access to integer */
union byte_access {
    uint32_t full;
    uint8_t bytes[4];
    uint16_t words[2];
} NOINLINE;

/* Test function for STRICT_LOW_PART RTL */
NOINLINE static uint32_t test_strict_low_part(void) {
    VOLATILE uint32_t accumulator = 0;
    VOLATILE union byte_access data;
    
    data.full = 0x12345678;
    
    /* 1. Byte store into integer (may generate STRICT_LOW_PART) */
    *(VOLATILE uint8_t*)&data.full = 0xFF;  /* Modify low byte only */
    accumulator += data.full;
    
    /* 2. Multiple byte operations */
    for (VOLATILE int i = 0; i < 4; i++) {
        data.bytes[i] = i * 64;
        accumulator += data.full;
    }
    
    /* 3. Word operations (16-bit low part) */
    data.words[0] = 0xABCD;  /* Low word */
    accumulator += data.full;
    
    /* 4. Arithmetic that truncates to byte */
    VOLATILE uint32_t big_val = 0x87654321;
    uint8_t truncated = big_val & 0xFF;  /* Explicit truncation */
    accumulator += truncated;
    
    /* 5. Inline assembly forcing low-part access */
    uint32_t asm_out;
    uint32_t asm_in = 0x12345678;
    
    /* x86 byte operation - %b0 accesses low byte of register */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    accumulator += asm_out;
    
    /* 6. Mixed-size operations */
    VOLATILE uint16_t half;
    half = data.full;  /* Implicit truncation */
    accumulator += half;
    
    return accumulator;
}

/* ==================== SUBREG patterns ==================== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Test function for SUBREG RTL */
NOINLINE static int test_subreg(void) {
    VOLATILE int result = 0;
    
    /* 1. Vector operations with element extraction */
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.5f, 2.5f, 3.5f, 4.5f};
    
    /* Extract elements - may use SUBREG */
    for (VOLATILE int i = 0; i < 4; i++) {
        int elem = vec_int[i];
        result += elem;
    }
    
    /* 2. Type punning via casts */
    VOLATILE float f = 3.14159f;
    VOLATILE int i = *(int*)&f;  /* Bitcast via pointer */
    result += i & 0xFFFF;
    
    /* 3. Mixed-size arithmetic */
    VOLATILE short s = -12345;
    VOLATILE int extended = s;  /* Sign extension */
    result += extended;
    
    /* 4. Union type punning */
    union {
        float f;
        int i;
        short s[2];
    } pun;
    
    pun.f = 2.71828f;
    result += pun.s[0] + pun.s[1];
    
    /* 5. Complex vector pattern */
    v4si vec_a = {10, 20, 30, 40};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    
    /* Extract and process each element */
    for (VOLATILE int j = 0; j < 4; j++) {
        VOLATILE int temp = vec_c[j];
        result += temp * j;
    }
    
    return result;
}

/* ==================== Memory operand patterns ==================== */

/* Complex structure for memory addressing */
struct nested {
    int data[8];
    struct nested* next;
};

/* Test function for complex memory operands */
NOINLINE static int test_memory_operand(void) {
    VOLATILE int result = 0;
    
    /* 1. Multi-level pointer dereferencing */
    VOLATILE int*** triple_ptr = NULL;
    VOLATILE int** double_ptr = NULL;
    VOLATILE int* single_ptr = NULL;
    VOLATILE int value = 42;
    
    single_ptr = &value;
    double_ptr = &single_ptr;
    triple_ptr = &double_ptr;
    
    /* Complex dereference chain */
    if (triple_ptr && *triple_ptr && **triple_ptr) {
        result += ***triple_ptr;
    }
    
    /* 2. Array with volatile index */
    VOLATILE int array[100];
    for (VOLATILE int i = 0; i < 100; i++) {
        array[i] = i * 3;
    }
    
    VOLATILE int idx = global_index % 100;
    result += array[idx];
    result += array[idx + 1];
    result += array[idx + 2];
    
    /* 3. Structure field chasing */
    struct nested node1, node2, node3;
    memset(&node1, 0, sizeof(node1));
    memset(&node2, 0, sizeof(node2));
    memset(&node3, 0, sizeof(node3));
    
    node1.data[0] = 111;
    node1.next = &node2;
    node2.data[1] = 222;
    node2.next = &node3;
    node3.data[2] = 333;
    node3.next = &node1;
    
    /* Complex structure access */
    result += node1.next->next->data[2];
    result += node1.next->data[global_index % 8];
    
    /* 4. Volatile memory operations */
    VOLATILE char* buffer = (VOLATILE char*)malloc(256);
    if (buffer) {
        for (VOLATILE int i = 0; i < 256; i++) {
            buffer[i] = i;
        }
        
        /* Access with complex addressing */
        VOLATILE int offset = get_index() % 256;
        result += buffer[offset];
        result += buffer[offset + 1] * 2;
        result += buffer[offset + 2] * 3;
        
        free((void*)buffer);
    }
    
    /* 5. Mixed addressing modes */
    VOLATILE int matrix[10][10];
    for (VOLATILE int i = 0; i < 10; i++) {
        for (VOLATILE int j = 0; j < 10; j++) {
            matrix[i][j] = i * 10 + j;
        }
    }
    
    VOLATILE int row = global_index % 10;
    VOLATILE int col = (global_index / 10) % 10;
    result += matrix[row][col];
    result += matrix[col][row];
    
    return result;
}

/* ==================== Main test driver ==================== */

/* Opaque function to prevent optimization */
int get_index(void) {
    static VOLATILE int counter = 0;
    return counter++ % 256;
}

void* get_ptr(void) {
    static VOLATILE char buffer[1024];
    return buffer + (get_index() % 512);
}

int main(void) {
    unsigned int total = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Run all pattern tests */
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
