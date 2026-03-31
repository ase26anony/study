/* test_resources.c - Generate RTL patterns for GCC resource.cc coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int opaque(int x);
extern void* opaque_ptr(void* p);

/* ========== ZERO_EXTRACT patterns ========== */

/* Bitfield structure that may generate ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header : 4;
    unsigned int payload : 20;
    unsigned int checksum : 8;
} NOINLINE;

/* Complex bitfield operations */
NOINLINE static uint32_t test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packet pkt;
    VOLATILE_VAR uint32_t raw_data = 0xDEADBEEF;
    
    /* Direct bitfield assignment - may generate ZERO_EXTRACT */
    pkt.header = (raw_data >> 28) & 0xF;
    pkt.payload = (raw_data >> 8) & 0xFFFFF;
    pkt.checksum = raw_data & 0xFF;
    
    /* Bit extraction with variable shift */
    VOLATILE_VAR int shift = opaque(4);
    uint32_t extracted = (raw_data >> shift) & ((1 << 12) - 1);
    
    /* Nested bitfield in union */
    union {
        struct {
            unsigned int low : 16;
            unsigned int high : 16;
        } bits;
        uint32_t word;
    } converter;
    
    converter.word = raw_data;
    converter.bits.high = opaque(converter.bits.low);
    
    return pkt.header + pkt.payload + pkt.checksum + extracted + converter.word;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Union for type punning */
union reg_access {
    uint32_t dword;
    uint16_t word;
    uint8_t byte;
    uint8_t bytes[4];
} NOINLINE;

NOINLINE static uint32_t test_strict_low_part(void) {
    VOLATILE_VAR uint32_t reg = 0x12345678;
    VOLATILE_VAR union reg_access ra;
    ra.dword = reg;
    
    /* Byte store into wider register - may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&reg = 0xFF;  /* Modify only low byte */
    
    /* Union-based byte access */
    ra.bytes[1] = opaque(ra.byte) & 0x7F;
    
    /* Explicit truncation preserving high bits */
    uint32_t temp = reg;
    uint8_t low_byte = temp & 0xFF;  /* Access only low part */
    temp = (temp & 0xFFFFFF00) | (low_byte ^ 0x55);
    
    /* Inline assembly forcing low-part access on x86 */
    uint32_t result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "movb %%al, %0\n\t"
        : "=m"(result)
        : "r"(temp)
        : "%eax"
    );
    
    /* Another byte operation */
    uint16_t* half_ptr = (uint16_t*)&reg;
    *half_ptr = (*half_ptr + 1) & 0xFFFF;  /* Modify only low 16 bits */
    
    return reg + ra.dword + temp + result;
}

/* ========== SUBREG patterns ========== */

/* Vector type using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static uint32_t test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Type punning between different sizes */
    uint64_t big = 0x1122334455667788ULL;
    uint32_t* ptr32 = (uint32_t*)&big;
    uint32_t low = ptr32[0];  /* SUBREG access to lower part */
    uint32_t high = ptr32[1]; /* SUBREG access to upper part */
    
    /* Vector element extraction - often uses SUBREG */
    int elem = vec[opaque(2) % 4];
    
    /* Mixed vector operations */
    short_vec[3] = (short)(vec[1] + vec[2]);
    
    /* Float/int reinterpretation */
    VOLATILE_VAR float f = 3.14159f;
    VOLATILE_VAR uint32_t int_view;
    memcpy(&int_view, &f, sizeof(int_view));  /* Type punning via memcpy */
    
    /* Cast between different integer sizes */
    uint16_t short_val = (uint16_t)big;
    uint32_t extended = (uint32_t)short_val;  /* Zero/sign extension */
    
    return low + high + elem + short_vec[0] + int_view + extended;
}

/* ========== Complex Memory Operands ========== */

/* Complex structure for memory addressing */
struct node {
    int data;
    struct node* next;
    int array[7];
} NOINLINE;

NOINLINE static uint32_t test_memory_operand(void) {
    /* Multi-level pointer chasing */
    VOLATILE_VAR struct node nodes[4];
    VOLATILE_VAR struct node* ptr1 = &nodes[0];
    VOLATILE_VAR struct node** ptr2 = &ptr1;
    VOLATILE_VAR struct node*** ptr3 = &ptr2;
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        nodes[i].data = opaque(i);
        nodes[i].next = &nodes[(i + 1) % 4];
        for (int j = 0; j < 7; j++) {
            nodes[i].array[j] = opaque(i * 10 + j);
        }
    }
    
    /* Complex addressing modes */
    int sum = 0;
    VOLATILE_VAR int idx = opaque(2);
    
    /* Multi-level dereference */
    sum += (**ptr3)->data;
    sum += (**ptr3)->array[idx];
    
    /* Pointer arithmetic with variable index */
    sum += (*(ptr1 + (idx % 3))).data;
    
    /* Array indexing with complex expression */
    sum += nodes[opaque(1) % 4].array[opaque(2) % 7];
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = &nodes[0].data;
    sum += *volatile_ptr;
    sum += *(volatile_ptr + 1);
    
    /* Structure pointer chain */
    struct node* current = &nodes[0];
    for (int i = 0; i < 3; i++) {
        sum += current->array[current->data % 7];
        current = current->next;
    }
    
    return sum;
}

/* ========== Main test driver ========== */

/* Opaque function to prevent optimization */
int opaque(int x) {
    static VOLATILE_VAR int counter = 0;
    return (x ^ 0x55) + (counter++);
}

void* opaque_ptr(void* p) {
    static VOLATILE_VAR intptr_t offset = 0;
    return (void*)((intptr_t)p + (offset++ & 0xF));
}

int main(void) {
    uint32_t checksum = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all tests */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_memory_operand();
    
    /* Mix results to ensure all code executes */
    checksum = opaque(checksum);
    
    printf("Final checksum: 0x%08X\n", checksum);
    
    return checksum != 0 ? 0 : 1;
}
