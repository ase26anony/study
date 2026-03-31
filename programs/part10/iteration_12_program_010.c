/* test_resources.c - Generate specific RTL patterns for GCC resource.cc coverage */

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

/* ====== ZERO_EXTRACT patterns ====== */

/* Bitfield structure */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int payload:12;
    unsigned int checksum:8;
    unsigned int flags:8;
} NOINLINE;

/* Test ZERO_EXTRACT through bitfield operations */
NOINLINE static uint32_t test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packet pkt;
    VOLATILE_VAR uint32_t raw_value = 0xDEADBEEF;
    uint32_t result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    pkt.header = (raw_value >> 28) & 0xF;
    pkt.payload = (raw_value >> 16) & 0xFFF;
    pkt.checksum = (raw_value >> 8) & 0xFF;
    pkt.flags = raw_value & 0xFF;
    
    /* Explicit bit extraction that may use ZERO_EXTRACT */
    result |= ((raw_value >> 24) & 0xFF) << 0;   /* Extract byte 3 */
    result |= ((raw_value >> 16) & 0xFF) << 8;   /* Extract byte 2 */
    result |= ((raw_value >> 8) & 0xFF) << 16;   /* Extract byte 1 */
    result |= (raw_value & 0xFF) << 24;          /* Extract byte 0 */
    
    /* Another bitfield structure with different layout */
    struct {
        unsigned int a:1;
        unsigned int b:2;
        unsigned int c:5;
        unsigned int d:8;
        unsigned int e:16;
    } bits;
    
    bits.a = (result >> 31) & 0x1;
    bits.b = (result >> 29) & 0x3;
    bits.c = (result >> 24) & 0x1F;
    bits.d = (result >> 16) & 0xFF;
    bits.e = result & 0xFFFF;
    
    return result ^ bits.e ^ pkt.header ^ pkt.payload;
}

/* ====== STRICT_LOW_PART patterns ====== */

NOINLINE static uint32_t test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_reg = 0x12345678;
    VOLATILE_VAR uint16_t half_reg;
    VOLATILE_VAR uint8_t byte_reg;
    uint32_t result = 0;
    
    /* Byte-sized store into wider integer - may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&wide_reg = 0xFF;  /* Modify only low byte */
    
    /* Union for type punning */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } u;
    u.full = 0x87654321;
    
    /* Modify individual bytes - compiler must preserve other bytes */
    u.bytes[0] = (uint8_t)(wide_reg & 0xFF);
    u.bytes[1] = (uint8_t)((wide_reg >> 8) & 0xFF);
    u.bytes[2] = (uint8_t)((wide_reg >> 16) & 0xFF);
    u.bytes[3] = (uint8_t)((wide_reg >> 24) & 0xFF);
    
    /* Arithmetic that truncates to low part */
    half_reg = wide_reg & 0xFFFF;          /* Keep only low 16 bits */
    byte_reg = wide_reg & 0xFF;            /* Keep only low 8 bits */
    
    /* Inline assembly that explicitly uses low byte register */
    uint32_t asm_out;
    uint32_t asm_in = 0xABCD1234;
    asm volatile (
        "movb %b1, %b0\n\t"                /* %b modifier for low byte */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    result = u.full ^ half_reg ^ byte_reg ^ asm_out;
    
    /* Force partial register update in loop */
    for (int i = 0; i < 4; i++) {
        *(volatile uint8_t*)((char*)&wide_reg + i) = i * 0x11;
    }
    
    return result ^ wide_reg;
}

/* ====== SUBREG patterns ====== */

NOINLINE static uint32_t test_subreg(void) {
    /* GCC vector extensions for SUBREG patterns */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    VOLATILE_VAR v4si vec_a = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_b = {10, 20, 30, 40, 50, 60, 70, 80};
    uint32_t result = 0;
    
    /* Extract vector elements - often uses SUBREG */
    int elem0 = vec_a[0];
    int elem1 = vec_a[1];
    short helem0 = vec_b[0];
    short helem1 = vec_b[1];
    
    /* Type punning between different sizes */
    float f = 3.14159f;
    uint32_t i;
    memcpy(&i, &f, sizeof(f));  /* Type punning via memcpy */
    
    /* Cast between different integer sizes */
    int32_t big = 0x12345678;
    int16_t small = (int16_t)big;          /* Truncation */
    int8_t tiny = (int8_t)big;             /* Further truncation */
    
    /* Mix types in expressions */
    result = elem0 + elem1 + helem0 + helem1 + small + tiny;
    
    /* Structure with mixed-size members */
    struct mixed {
        char c;
        short s;
        int i;
        long long ll;
    } m;
    
    m.c = (char)i;
    m.s = (short)(i >> 8);
    m.i = i;
    m.ll = (long long)i << 32;
    
    /* Access subparts through pointers */
    int32_t* ptr32 = &m.i;
    int16_t* ptr16 = (int16_t*)ptr32;
    int8_t* ptr8 = (int8_t*)ptr32;
    
    result ^= *ptr32 ^ *ptr16 ^ *ptr8;
    
    return result;
}

/* ====== Memory operand patterns ====== */

NOINLINE static uint32_t test_memory_operand(void) {
    /* Complex memory addressing modes */
    VOLATILE_VAR uint32_t buffer[64];
    VOLATILE_VAR uint32_t* ptr1 = buffer;
    VOLATILE_VAR uint32_t** ptr2 = &ptr1;
    VOLATILE_VAR uint32_t*** ptr3 = &ptr2;
    uint32_t result = 0;
    
    /* Initialize buffer */
    for (int i = 0; i < 64; i++) {
        buffer[i] = i * 0x1001;
    }
    
    /* Multi-level pointer dereferencing */
    result = ***ptr3;                      /* Triple dereference */
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = get_index() & 63;
    result += buffer[idx];                 /* Non-constant offset */
    result += buffer[idx + 1];
    result += buffer[idx * 2];
    
    /* Structure with nested arrays */
    struct nested {
        int data[8][8];
        int* ptrs[4];
    } s;
    
    /* Initialize nested structure */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            s.data[i][j] = i * 8 + j;
        }
    }
    
    /* Complex addressing: s.data[idx][idx/2] */
    result += s.data[idx & 7][(idx >> 1) & 7];
    
    /* Pointer chasing through array */
    uint32_t* chase = buffer;
    for (int i = 0; i < 4; i++) {
        result += *chase;
        chase = (uint32_t*)((char*)chase + buffer[i] % 56);
    }
    
    /* Volatile memory operations that won't be eliminated */
    VOLATILE_VAR uint32_t* volatile vptr = buffer;
    result += vptr[0];
    result += vptr[1];
    
    return result;
}

/* ====== Main function ====== */

int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing resource pattern generation...\n");
    
    /* Run all tests */
    checksum ^= test_zero_extract();
    printf("Zero extract test completed\n");
    
    checksum ^= test_strict_low_part();
    printf("Strict low part test completed\n");
    
    checksum ^= test_subreg();
    printf("Subreg test completed\n");
    
    checksum ^= test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    return (checksum == 0) ? 0 : 1;
}

/* External functions to prevent optimization */
int get_index(void) {
    static volatile int counter = 0;
    return counter++ & 0x3F;
}

void escape(void* p) {
    asm volatile ("" : : "r"(p) : "memory");
}
