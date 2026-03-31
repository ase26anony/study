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

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_pack {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int reserved:20;
} NOINLINE;

struct bitfield_large {
    unsigned long long high:32;
    unsigned long long low:32;
} NOINLINE;

NOINLINE static unsigned test_zero_extract(void) {
    VOLATILE_VAR struct bitfield_pack bf1 = {0};
    VOLATILE_VAR struct bitfield_large bf2 = {0};
    VOLATILE_VAR unsigned int temp = 0x12345678;
    unsigned result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = (temp >> 5) & 0x7;      /* Extract 3 bits */
    bf1.value = (temp >> 8) & 0x1F;    /* Extract 5 bits */
    bf1.mode = (temp >> 16) & 0xF;     /* Extract 4 bits */
    
    /* Complex bitfield extraction */
    bf2.low = temp & 0xFFFFFFFF;
    bf2.high = (temp >> 4) & 0x0FFFFFFF;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    unsigned extracted = (temp >> get_index()) & ((1 << 7) - 1);
    
    /* Combine results */
    result = bf1.flag + bf1.value + bf1.mode + 
             (unsigned)(bf2.low & 0xFF) + (unsigned)(bf2.high & 0xFF) + 
             extracted;
    
    /* Force side effect */
    *(VOLATILE_VAR unsigned*)&bf1 = result;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE static unsigned test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned char byte_store;
    unsigned result = 0;
    
    /* Byte store into wider integer - may generate STRICT_LOW_PART */
    *(VOLATILE_VAR unsigned char*)&wide_reg = 0x42;
    result += wide_reg & 0xFF;
    
    /* Another byte store */
    ((VOLATILE_VAR unsigned char*)&wide_reg)[1] = 0x99;
    result += (wide_reg >> 8) & 0xFF;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0xCAFEBABE;
    pun.bytes[0] = get_index() & 0xFF;  /* Low byte store */
    result += pun.full & 0xFF;
    
    /* Arithmetic truncation in context where high bits must be preserved */
    unsigned int source = 0x12345678;
    unsigned char dest;
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r"(dest)
        : "r"(source)
        : "cc"
    );
    result += dest;
    
    /* Explicit truncation with masking */
    unsigned int preserved = 0xF0F0F0F0;
    unsigned char truncated = (preserved & 0xFF) + 1;
    /* Force use of preserved in a way that keeps it alive */
    result += truncated + (preserved >> 24);
    
    return result;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE static unsigned test_subreg(void) {
    VOLATILE_VAR v4si vec = {1, 2, 3, 4};
    VOLATILE_VAR v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    VOLATILE_VAR float f = 3.14159f;
    VOLATILE_VAR int i = 0;
    unsigned result = 0;
    
    /* Type punning float/int - may generate SUBREG */
    i = *(VOLATILE_VAR int*)&f;
    result += i & 0xFF;
    
    /* Vector element extraction */
    int elem = vec[get_index() % 4];
    result += elem;
    
    /* Short/int mixing */
    short s = 1000;
    int widened = s;          /* Sign/zero extension */
    result += widened & 0xFF;
    
    /* Vector reinterpretation */
    short* as_shorts = (short*)&vec;
    result += as_shorts[0] + as_shorts[1];
    
    /* Mixed-size operations */
    unsigned long long big = 0x1122334455667788ULL;
    unsigned int lower = big;  /* Truncation */
    unsigned short upper = (big >> 32);  /* Extract with shift */
    result += lower + upper;
    
    /* Complex subregister access via inline asm */
    register int reg_var asm("eax") = 0x89ABCDEF;
    short subpart;
    asm volatile (
        "movw %%ax, %0\n\t"
        : "=r"(subpart)
        : "0"(reg_var)
        : "cc"
    );
    result += subpart;
    
    return result;
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[8];
    struct nested* next;
};

NOINLINE static unsigned test_memory_operand(void) {
    static VOLATILE_VAR int buffer[256];
    static struct nested nodes[4];
    VOLATILE_VAR struct nested* current;
    VOLATILE_VAR int*** triple_ptr;
    unsigned result = 0;
    int i;
    
    /* Initialize */
    for (i = 0; i < 256; i++) {
        buffer[i] = i * 3;
    }
    for (i = 0; i < 4; i++) {
        memset(nodes[i].data, i, sizeof(nodes[i].data));
        nodes[i].next = &nodes[(i + 1) % 4];
    }
    
    /* Complex pointer chasing */
    current = &nodes[0];
    for (i = 0; i < 8; i++) {
        result += current->data[get_index() % 8];
        current = current->next;
    }
    
    /* Multi-level indirection */
    int** ptr2 = (int**)malloc(3 * sizeof(int*));
    for (i = 0; i < 3; i++) {
        ptr2[i] = &buffer[i * 10];
    }
    triple_ptr = &ptr2;
    
    /* Triple dereference */
    result += ***triple_ptr;
    result += (*(*triple_ptr + 1))[2];
    
    /* Volatile array access with variable index */
    VOLATILE_VAR int idx = get_index() % 256;
    result += buffer[idx];
    result += buffer[idx + 1];
    result += buffer[idx * 2 % 256];
    
    /* Structure field with complex addressing */
    result += nodes[1].data[3] + nodes[2].data[5];
    
    /* Pointer arithmetic with dereference */
    int* ptr = buffer;
    ptr += get_index() % 128;
    result += *ptr;
    result += *(ptr + 4);
    result += *(ptr - 2);
    
    free(ptr2);
    return result;
}

/* ========== Main driver ========== */
/* Opaque functions to prevent optimization */
int get_index(void) {
    static VOLATILE_VAR int counter = 0;
    return (counter++ * 13 + 7) & 0xFF;
}

void* get_ptr(void) {
    static VOLATILE_VAR char buffer[100];
    return buffer + (get_index() % 80);
}

int main(void) {
    unsigned total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    total += test_zero_extract();
    printf("  zero_extract: %u\n", total & 0xFF);
    
    total += test_strict_low_part();
    printf("  strict_low_part: %u\n", total & 0xFF);
    
    total += test_subreg();
    printf("  subreg: %u\n", total & 0xFF);
    
    total += test_memory_operand();
    printf("  memory_operand: %u\n", total & 0xFF);
    
    printf("Final checksum: %u\n", total);
    
    return (int)(total & 0xFF);
}
