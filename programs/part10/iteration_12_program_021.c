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
extern void* get_ptr(void);

/* Test data structures */
struct bitfield_packet {
    unsigned int header:4;
    unsigned int payload:12;
    unsigned int checksum:8;
    unsigned int flags:8;
};

/* Union for low-part access */
union int_bytes {
    uint32_t full;
    uint8_t bytes[4];
    struct {
        uint8_t b0, b1, b2, b3;
    };
};

/* Vector type for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Global volatile variables to prevent optimization */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR void* global_ptr = NULL;
VOLATILE_VAR int global_counter = 0;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE static uint32_t test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_packet packet = {0};
    VOLATILE_VAR uint32_t raw_value = 0xDEADBEEF;
    uint32_t result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    packet.header = (raw_value >> 28) & 0xF;  /* Extract 4 bits */
    packet.payload = (raw_value >> 12) & 0xFFF; /* Extract 12 bits */
    packet.checksum = (raw_value >> 4) & 0xFF;  /* Extract 8 bits */
    packet.flags = raw_value & 0xFF;           /* Extract 8 bits */
    
    /* More explicit bit extraction */
    result |= ((raw_value >> 24) & 0xFF) << 0;   /* Byte 3 */
    result |= ((raw_value >> 16) & 0xFF) << 8;   /* Byte 2 */
    result |= ((raw_value >> 8) & 0xFF) << 16;   /* Byte 1 */
    result |= (raw_value & 0xFF) << 24;          /* Byte 0 */
    
    /* Complex bitfield manipulation */
    struct {
        unsigned int a:3;
        unsigned int b:5;
        unsigned int c:7;
        unsigned int d:9;
        unsigned int e:8;
    } complex_bf = {0};
    
    complex_bf.a = (result >> 0) & 0x7;
    complex_bf.b = (result >> 3) & 0x1F;
    complex_bf.c = (result >> 8) & 0x7F;
    complex_bf.d = (result >> 15) & 0x1FF;
    complex_bf.e = (result >> 24) & 0xFF;
    
    /* Mix bitfield and non-bitfield operations */
    return result + packet.header + packet.payload + 
           complex_bf.a + complex_bf.d;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static uint32_t test_strict_low_part(void) {
    VOLATILE_VAR uint32_t wide_reg = 0x12345678;
    VOLATILE_VAR union int_bytes converter;
    uint32_t result = 0;
    
    converter.full = wide_reg;
    
    /* Byte-sized stores that may generate STRICT_LOW_PART */
    *(volatile uint8_t*)&wide_reg = 0xFF;  /* Modify only low byte */
    
    /* Using union for byte access */
    converter.bytes[0] = 0xAA;  /* Low byte access */
    converter.bytes[1] = 0xBB;  /* Second byte */
    
    /* Arithmetic that truncates to low part */
    uint32_t temp = wide_reg;
    uint8_t low_byte = temp & 0xFF;  /* Explicit truncation */
    uint16_t low_word = temp & 0xFFFF;
    
    /* Inline assembly for explicit low-part register access */
    uint32_t asm_out;
    uint32_t asm_in = 0x87654321;
    
    /* x86-specific: %b0 modifier accesses low byte of register */
    asm volatile (
        "movl %1, %0\n\t"
        "movb %b0, %b0\n\t"  /* Force low byte operation */
        : "=r"(asm_out)
        : "r"(asm_in)
        : "cc"
    );
    
    /* Multiple low-part operations */
    result = converter.full + low_byte + low_word + asm_out;
    
    /* Force register pressure to keep values in registers */
    for (VOLATILE_VAR int i = 0; i < 4; i++) {
        *(volatile uint8_t*)((char*)&result + i) = converter.bytes[i];
    }
    
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE static uint32_t test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR float f = 3.14159f;
    VOLATILE_VAR uint32_t i;
    uint32_t result = 0;
    
    /* Type punning float/int - may generate SUBREG */
    i = *(uint32_t*)&f;  /* Bitcast through pointer */
    result += i;
    
    /* Vector element extraction - often uses SUBREG */
    int elem0 = vec[0];
    int elem2 = vec[2];
    result += elem0 + elem2;
    
    /* Short/int mixing */
    VOLATILE_VAR short s = 1000;
    VOLATILE_VAR int extended = s;  /* Sign extension */
    VOLATILE_VAR int truncated = (short)extended;  /* Truncation */
    
    result += extended + truncated;
    
    /* Vector to scalar conversion */
    for (int j = 0; j < 8; j++) {
        result += short_vec[j];  /* Each access may be SUBREG */
    }
    
    /* Mixed-size operations in expressions */
    VOLATILE_VAR char c = 'A';
    VOLATILE_VAR int mixed = c * 256 + s / 2;
    result += mixed;
    
    /* Pointer-based type punning */
    union {
        float f;
        uint32_t u;
    } pun = {3.14f};
    result += pun.u;
    
    return result;
}

/* ========== Complex Memory Operand patterns ========== */
NOINLINE static uint32_t test_memory_operand(void) {
    /* Complex memory hierarchy */
    static VOLATILE_VAR int buffer[256];
    static VOLATILE_VAR int* ptr_array[16];
    VOLATILE_VAR int*** triple_ptr;
    uint32_t result = 0;
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    
    for (int i = 0; i < 16; i++) {
        ptr_array[i] = &buffer[i * 16];
    }
    
    triple_ptr = &ptr_array[0];
    
    /* Complex addressing modes */
    VOLATILE_VAR int index = global_index % 16;
    
    /* Multi-level pointer dereference */
    result += ***triple_ptr;
    
    /* Array indexing with variable index */
    result += buffer[index * 4 + 2];
    
    /* Pointer chasing */
    VOLATILE_VAR int** ptr2 = &ptr_array[0];
    result += *(*ptr2 + 3);
    
    /* Structure with nested arrays */
    struct nested {
        int data[4][4];
        struct nested* next;
    } node1, node2;
    
    memset(&node1, 0, sizeof(node1));
    memset(&node2, 0, sizeof(node2));
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            node1.data[i][j] = i * 10 + j;
        }
    }
    
    node1.next = &node2;
    
    /* Complex structure access */
    result += node1.data[1][2];
    result += node1.next->data[2][3];
    
    /* Volatile memory operations */
    VOLATILE_VAR int* volatile volatile_ptr = buffer;
    for (VOLATILE_VAR int i = 0; i < 10; i++) {
        result += *(volatile_ptr + i * 7);
    }
    
    /* Simulated linked list traversal */
    struct list_node {
        int value;
        struct list_node* next;
    } nodes[5];
    
    for (int i = 0; i < 5; i++) {
        nodes[i].value = i * 100;
        nodes[i].next = (i < 4) ? &nodes[i + 1] : NULL;
    }
    
    struct list_node* current = &nodes[0];
    while (current) {
        result += current->value;
        current = current->next;
    }
    
    return result;
}

/* ========== Main test driver ========== */
int main(void) {
    uint32_t total = 0;
    
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
    
    /* Ensure results are used */
    printf("Final checksum: %u (0x%08X)\n", total, total);
    
    /* Force side effects */
    global_counter = total % 1000;
    
    return (int)(total & 0x7FFFFFFF);
}

/* Dummy external functions */
int get_index(void) { return global_counter; }
void* get_ptr(void) { return &global_counter; }
