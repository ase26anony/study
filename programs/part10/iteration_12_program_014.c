/* test_resources.c - Generate specific RTL patterns for GCC resource tracking */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int get_index(void);
extern void escape(void*);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char global_byte = 0;

/* ========== ZERO_EXTRACT patterns ========== */
typedef struct {
    unsigned int flag:3;      /* 3-bit field */
    unsigned int value:5;     /* 5-bit field */
    unsigned int mode:4;      /* 4-bit field */
    unsigned int reserved:20; /* padding */
} bitfield_t;

NOINLINE static unsigned int test_zero_extract(void) {
    VOLATILE_VAR bitfield_t bf = {0};
    VOLATILE_VAR unsigned int raw = 0xDEADBEEF;
    unsigned int result = 0;
    
    /* Direct bitfield assignments - may generate ZERO_EXTRACT */
    bf.flag = 0x5;      /* 3-bit field */
    bf.value = 0x1A & 0x1F;  /* 5-bit field, masked */
    bf.mode = 0x9;      /* 4-bit field */
    
    /* Bitwise extraction that may compile to ZERO_EXTRACT */
    result |= (raw >> 3) & 0x7;    /* Extract 3 bits */
    result |= (raw >> 8) & 0x1F;   /* Extract 5 bits */
    result |= (raw >> 16) & 0xF;   /* Extract 4 bits */
    
    /* Complex extraction with variable shift */
    int shift = global_index & 0x7;
    result |= (raw >> shift) & ((1 << 3) - 1);
    
    /* Bitfield extraction via pointer */
    bitfield_t* bf_ptr = &bf;
    result |= bf_ptr->flag;
    result |= bf_ptr->value << 3;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0x12345678;
    VOLATILE_VAR unsigned char byte_store;
    unsigned int result = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0xFF;
    result = wide_reg;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } u;
    u.full = 0x87654321;
    u.bytes[0] = global_byte;  /* Low byte store */
    result ^= u.full;
    
    /* Truncation preserving high bits context */
    unsigned int temp = wide_reg;
    unsigned char low_byte = temp & 0xFF;  /* May need to preserve high bits */
    result += low_byte;
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        unsigned char* ptr = (unsigned char*)&wide_reg + i;
        *ptr = i * 0x11;
    }
    result ^= wide_reg;
    
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE static unsigned int test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi short_vec = {10, 20, 30, 40, 50, 60, 70, 80};
    unsigned int result = 0;
    
    /* Vector element extraction - often SUBREG */
    result += vec[0];      /* Extract first element */
    result += vec[global_index & 3];  /* Variable index */
    
    /* Type punning between different sizes */
    unsigned int int_val = 0x89ABCDEF;
    unsigned short short_val = *(unsigned short*)&int_val;  /* SUBREG access */
    result += short_val;
    
    /* Float/int bitcasting */
    float f = 3.14159f;
    unsigned int int_bits = *(unsigned int*)&f;  /* SUBREG for type conversion */
    result ^= int_bits;
    
    /* Mixed-size operations */
    long long big = 0x1122334455667788LL;
    int lower = (int)big;          /* Truncation to 32-bit */
    int upper = (int)(big >> 32);  /* High part */
    result += lower + upper;
    
    /* Vector lane access */
    short_vec[3] = 99;  /* SUBREG store */
    result += short_vec[2];
    
    return result;
}

/* ========== Complex Memory Operands ========== */
typedef struct node {
    struct node* next;
    int data;
    char payload[8];
} node_t;

NOINLINE static unsigned int test_memory_operand(void) {
    /* Complex memory addressing */
    VOLATILE_VAR node_t nodes[4];
    VOLATILE_VAR node_t* node_ptr = &nodes[0];
    VOLATILE_VAR int*** triple_ptr;
    unsigned int result = 0;
    
    /* Initialize linked structure */
    for (int i = 0; i < 3; i++) {
        nodes[i].next = &nodes[i + 1];
        nodes[i].data = i * 100;
        memset(nodes[i].payload, i * 0x11, sizeof(nodes[i].payload));
    }
    nodes[3].next = NULL;
    nodes[3].data = 300;
    
    /* Multi-level pointer chasing */
    int val1 = 42;
    int* ptr1 = &val1;
    int** ptr2 = &ptr1;
    triple_ptr = &ptr2;
    
    result += ***triple_ptr;  /* Triple dereference */
    
    /* Complex array indexing with volatile */
    int idx = get_index() & 0x3;
    result += nodes[idx].data;  /* Base + index + field offset */
    result += nodes[idx].payload[global_index & 0x7];
    
    /* Pointer arithmetic with structure fields */
    node_t* current = node_ptr;
    while (current) {
        result += current->data;
        current = current->next;
    }
    
    /* Volatile memory operations */
    VOLATILE_VAR int mem_buffer[16];
    for (VOLATILE_VAR int i = 0; i < 16; i++) {
        mem_buffer[i] = i * 0x100;
        result += mem_buffer[global_index & 0xF];
    }
    
    /* Inline assembly with memory operand */
    int asm_out;
    asm volatile (
        "movl %1, %0\n\t"
        : "=r"(asm_out)
        : "m"(mem_buffer[2])
        : "memory"
    );
    result += asm_out;
    
    return result;
}

/* ========== Inline Assembly for Specific RTL ========== */
NOINLINE static unsigned int test_inline_asm(void) {
    unsigned int result = 0;
    unsigned int in_val = 0x12345678;
    unsigned char out_byte;
    
    /* STRICT_LOW_PART via inline assembly */
    asm volatile (
        "movb %b1, %0\n\t"
        : "=m"(out_byte)
        : "r"(in_val)
        : "memory"
    );
    result = out_byte;
    
    /* SUBREG via inline assembly */
    unsigned short out_word;
    asm volatile (
        "movw %w1, %0\n\t"
        : "=m"(out_word)
        : "r"(in_val)
        : "memory"
    );
    result += out_word;
    
    /* Memory operand with complex addressing */
    int array[4] = {10, 20, 30, 40};
    int idx = global_index & 0x3;
    int asm_result;
    asm volatile (
        "movl (%1,%2,4), %0\n\t"
        : "=r"(asm_result)
        : "r"(array), "r"(idx)
        : "memory"
    );
    result += asm_result;
    
    return result;
}

/* ========== Main Execution ========== */
int main(void) {
    unsigned int checksum = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Execute all pattern tests */
    checksum += test_zero_extract();
    checksum += test_strict_low_part();
    checksum += test_subreg();
    checksum += test_memory_operand();
    checksum += test_inline_asm();
    
    /* Force side effects to be observable */
    printf("Final checksum: 0x%08X\n", checksum);
    
    /* Use escape analysis to prevent dead code elimination */
    escape(&checksum);
    
    return (int)(checksum & 0x7FFFFFFF);
}

/* Dummy external functions */
int get_index(void) { return global_index; }
void escape(void* p) { asm volatile ("" : : "r"(p) : "memory"); }
