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
    unsigned int data:8;
    unsigned int pad:16;
} NOINLINE;

/* Test 1: Bitfield operations that may generate ZERO_EXTRACT */
NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_struct bf;
    VOLATILE_VAR unsigned int raw_val = 0xABCD1234;
    
    /* Direct bitfield assignments */
    bf.flag = (raw_val >> 0) & 0x7;
    bf.value = (raw_val >> 3) & 0x1F;
    bf.data = (raw_val >> 8) & 0xFF;
    
    /* Bitfield extraction via masking */
    unsigned int extracted = 0;
    extracted |= (bf.flag & 0x7) << 0;
    extracted |= (bf.value & 0x1F) << 3;
    extracted |= (bf.data & 0xFF) << 8;
    
    /* Complex bitfield expression */
    unsigned int combined = ((bf.data << 5) | bf.value) & 0x1FF;
    combined = (combined >> bf.flag) & 0x3F;
    
    return extracted + combined;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_reg = 0xDEADBEEF;
    VOLATILE_VAR uint8_t byte_val = 0x42;
    unsigned int result = 0;
    
    /* Byte store into wider integer (may generate STRICT_LOW_PART) */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } u;
    u.full = wide_reg;
    u.bytes[1] = byte_val;  /* Modify only one byte */
    result += u.full;
    
    /* Pointer cast for byte access */
    *(volatile uint8_t*)&wide_reg = 0xAA;
    result += wide_reg;
    
    /* Arithmetic truncation preserving high bits */
    uint32_t temp = wide_reg;
    uint8_t low_byte = temp & 0xFF;  /* Explicit truncation */
    temp = (temp & ~0xFF) | (low_byte ^ 0x55);
    result += temp;
    
    /* Inline assembly forcing low-byte register access */
    uint32_t asm_out;
    uint32_t asm_in = 0x12345678;
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    result += asm_out;
    
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE unsigned int test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element extraction */
    int elem0 = vec_int[0];
    short elem3 = vec_short[3];
    
    /* Type punning through unions */
    union {
        float f;
        uint32_t i;
    } pun;
    pun.f = 3.14159f;
    uint32_t int_bits = pun.i;  /* Bitcast via union */
    
    /* Explicit casts between different sizes */
    uint64_t wide = 0x1122334455667788ULL;
    uint32_t narrow = (uint32_t)wide;  /* Truncating cast */
    uint16_t narrower = (uint16_t)narrow;
    
    /* Mixed-size operations */
    short s_val = -100;
    int promoted = s_val * 2;  /* Sign extension then operation */
    
    return elem0 + elem3 + int_bits + narrower + promoted;
}

/* ========== Complex memory operand patterns ========== */
NOINLINE unsigned int test_memory_operand(void) {
    /* Multi-level pointer structure */
    volatile uint32_t ***triple_ptr = NULL;
    volatile uint32_t **double_ptr = NULL;
    volatile uint32_t *single_ptr = NULL;
    
    /* Allocate and initialize memory */
    uint32_t buffer[16];
    for (int i = 0; i < 16; i++) {
        buffer[i] = i * 0x11111111;
    }
    
    /* Complex addressing modes */
    single_ptr = buffer;
    double_ptr = &single_ptr;
    triple_ptr = &double_ptr;
    
    /* Volatile index prevents constant propagation */
    VOLATILE_VAR int idx = global_index % 16;
    
    /* Multi-level dereference */
    uint32_t val1 = ***triple_ptr + idx;
    
    /* Structure with nested arrays */
    struct nested {
        int data[4][4];
        volatile int index;
    } ns;
    
    ns.index = idx;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ns.data[i][j] = i * 4 + j;
        }
    }
    
    /* Complex array indexing */
    uint32_t val2 = ns.data[idx % 4][(idx + 1) % 4];
    
    /* Pointer arithmetic with volatile */
    volatile uint32_t *volatile_ptr = buffer;
    vol_ptr += idx;
    uint32_t val3 = *vol_ptr;
    
    /* Memory barrier to prevent reordering */
    asm volatile("" ::: "memory");
    
    return val1 + val2 + val3;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Initialize global variables */
    global_index = 7;
    global_ptr = malloc(64);
    if (global_ptr) {
        memset(global_ptr, 0xCC, 64);
    }
    
    /* Run all pattern tests */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_memory_operand();
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    if (global_ptr) {
        free(global_ptr);
    }
    
    return (checksum != 0) ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) { return 42; }
void* get_ptr(void) { return malloc(1); }
