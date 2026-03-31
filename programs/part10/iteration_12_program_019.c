/* test_resources.c - Generate specific RTL patterns for GCC resource.cc coverage */

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

/* Global volatile variables to prevent optimizations */
VOLATILE int global_index = 0;
VOLATILE void* global_ptr = NULL;

/* ====== ZERO_EXTRACT patterns ====== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfield_packet {
    unsigned int header: 4;
    unsigned int data: 12;
    unsigned int footer: 8;
    unsigned int checksum: 8;
} __attribute__((packed));

NOINLINE static unsigned int test_zero_extract(void) {
    VOLATILE struct bitfield_packet packet;
    packet.header = 0xA;
    packet.data = 0xABC;
    packet.footer = 0x3F;
    packet.checksum = 0;
    
    /* Force bitfield extraction operations */
    unsigned int extracted = 0;
    
    /* These should generate ZERO_EXTRACT RTL */
    extracted |= (packet.data >> 4) & 0xFF;      /* Extract middle 8 bits */
    extracted |= (packet.header << 28);          /* Move to high bits */
    
    /* Complex bitfield manipulation */
    unsigned int combined = (packet.data << 8) | packet.footer;
    extracted |= (combined >> 2) & 0x3FFF;       /* Another extraction */
    
    /* Bitfield assignment that might use ZERO_EXTRACT */
    packet.checksum = (extracted >> 4) & 0xF;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    unsigned int raw = *(unsigned int*)&packet;
    extracted |= (raw >> 16) & 0xFFFF;           /* Extract lower half */
    
    return extracted + packet.checksum;
}

/* ====== STRICT_LOW_PART patterns ====== */

NOINLINE static unsigned int test_strict_low_part(void) {
    VOLATILE unsigned int wide_reg = 0x12345678;
    VOLATILE unsigned int result = 0;
    
    /* Byte-sized stores that may generate STRICT_LOW_PART */
    unsigned char* byte_ptr = (unsigned char*)&wide_reg;
    
    /* Store to low byte - may use STRICT_LOW_PART on x86 */
    byte_ptr[0] = 0xFF;                          /* Modify low byte only */
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } converter;
    converter.full = wide_reg;
    
    /* Modify individual bytes */
    converter.bytes[1] = 0xAA;                   /* Modify second byte */
    
    /* Arithmetic that truncates to low part */
    result = wide_reg & 0xFF;                    /* Keep only low byte */
    result |= (converter.full & 0xFF00);         /* Keep second byte */
    
    /* Inline assembly for explicit low-part access */
    unsigned int asm_out;
    __asm__ volatile (
        "movb %b1, %0\n\t"                       /* %b1 = low byte of input */
        : "=r" (asm_out)
        : "r" (wide_reg)
        : "cc"
    );
    result += asm_out;
    
    /* More byte operations */
    *(volatile unsigned char*)&wide_reg = 0x42;  /* Direct low-byte store */
    
    return result + wide_reg;
}

/* ====== SUBREG patterns ====== */

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static unsigned int test_subreg(void) {
    VOLATILE v4si vec = {1, 2, 3, 4};
    VOLATILE v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE unsigned int result = 0;
    
    /* Type punning between different sizes */
    unsigned int int_val = 0xDEADBEEF;
    unsigned short short_val;
    
    /* Cast that may generate SUBREG */
    short_val = (unsigned short)int_val;         /* Truncate to 16 bits */
    result += short_val;
    
    /* Access vector elements - often uses SUBREG */
    result += vec[0] + vec[2];                   /* Extract elements */
    
    /* Mix vector types */
    short_vec[3] = (short)vec[1];                /* Cross-type assignment */
    
    /* Float/int punning */
    float f = 3.14159f;
    unsigned int int_bits;
    memcpy(&int_bits, &f, sizeof(f));           /* Type pun via memcpy */
    result += int_bits & 0xFFFF;                /* Extract low half */
    
    /* More subregister accesses */
    unsigned long long big = 0x123456789ABCDEF0ULL;
    unsigned int lower = (unsigned int)big;      /* Extract low 32 bits */
    unsigned int upper = (unsigned int)(big >> 32); /* Extract high 32 bits */
    
    result += lower + upper;
    
    /* Complex vector operation */
    v4si vec2 = vec + (v4si){5, 6, 7, 8};
    result += vec2[global_index % 4];           /* Volatile index */
    
    return result;
}

/* ====== Memory operand patterns ====== */

/* Complex structure for memory addressing */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE static unsigned int test_memory_operand(void) {
    VOLATILE unsigned int result = 0;
    
    /* Multi-level pointer dereferencing */
    int*** triple_ptr = (int***)malloc(sizeof(int**));
    int** double_ptr = (int**)malloc(sizeof(int*));
    int* single_ptr = (int*)malloc(sizeof(int) * 4);
    
    *single_ptr = 0x1234;
    *double_ptr = single_ptr;
    *triple_ptr = double_ptr;
    
    /* Complex memory access - walks address expression */
    result += ***triple_ptr;                     /* Triple dereference */
    
    /* Array with volatile index */
    VOLATILE int array[100];
    for (VOLATILE int i = 0; i < 10; i++) {
        array[i * 7] = i;                       /* Non-constant stride */
    }
    result += array[global_index % 100];
    
    /* Structure field access with pointer chasing */
    struct nested node1, node2;
    node1.data[0] = 100;
    node1.data[1] = 200;
    node1.next = &node2;
    node2.data[0] = 300;
    node2.next = NULL;
    
    result += node1.next->data[0];              /* Structure pointer access */
    
    /* Volatile memory operations */
    VOLATILE int* volatile_ptr = array;
    volatile_ptr += get_index() % 50;           /* Opaque function call */
    result += *volatile_ptr;
    
    /* Cleanup */
    free(single_ptr);
    free(double_ptr);
    free(triple_ptr);
    
    return result;
}

/* ====== Main function ====== */

int main(void) {
    unsigned int total = 0;
    
    printf("Testing RTL pattern generation for resource.cc coverage...\n");
    
    /* Initialize global variables */
    global_index = 42;
    global_ptr = &global_index;
    
    /* Run all tests */
    total += test_zero_extract();
    printf("  Zero-extract test completed\n");
    
    total += test_strict_low_part();
    printf("  Strict-low-part test completed\n");
    
    total += test_subreg();
    printf("  Subreg test completed\n");
    
    total += test_memory_operand();
    printf("  Memory operand test completed\n");
    
    printf("Final checksum: %u\n", total);
    
    return (int)(total % 256);  /* Return non-zero to indicate execution */
}

/* Dummy external functions */
int get_index(void) { return global_index; }
void* get_ptr(void) { return global_ptr; }
