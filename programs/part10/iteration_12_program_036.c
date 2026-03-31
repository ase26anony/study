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
extern void escape(void*);

/* Global volatile variables to force memory operations */
VOLATILE_VAR int global_index = 0;
VOLATILE_VAR char* global_ptr = NULL;

/* ========== ZERO_EXTRACT patterns ========== */
struct bitfield_pack {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int pad:20;
} NOINLINE;

struct bitfield_large {
    unsigned long long high:32;
    unsigned long long low:32;
} NOINLINE;

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_pack bf1 = {0};
    struct bitfield_large bf2 = {0};
    VOLATILE_VAR unsigned int temp = 0x12345678;
    
    /* Direct bitfield assignments */
    bf1.flag = 5;
    bf1.value = 20;  /* Will be truncated to 5 bits */
    bf1.mode = 9;
    
    /* Bitfield extraction via masking */
    unsigned int extracted = (temp >> 8) & 0xFFF;  /* Should generate ZERO_EXTRACT */
    
    /* Complex bitfield manipulation */
    bf2.high = (temp >> 16) & 0xFFFF;
    bf2.low = temp & 0xFFFF;
    
    /* Extract from bitfield struct */
    unsigned int combined = (bf1.value << 3) | bf1.flag;
    
    /* Another masking operation likely to generate ZERO_EXTRACT */
    unsigned int masked = (temp & 0xF0F0F0F0) >> 4;
    
    return extracted + combined + masked + (unsigned int)bf2.high;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int wide_reg = 0xDEADBEEF;
    VOLATILE_VAR unsigned short half_reg = 0;
    VOLATILE_VAR unsigned char byte_reg = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&wide_reg = 0x42;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0x12345678;
    pun.bytes[1] = 0xAA;  /* Modify only one byte */
    
    /* Truncation preserving high bits context */
    byte_reg = wide_reg & 0xFF;
    
    /* Inline assembly forcing low-part access */
    unsigned int result;
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (result)
        : "r" (wide_reg)
        : "cc"
    );
    
    /* Another low-part operation */
    half_reg = wide_reg & 0xFFFF;
    
    /* Complex expression with low-part access */
    result = (wide_reg & 0xFF) + (pun.full & 0xFF00) >> 8;
    
    return result + byte_reg + half_reg + pun.full;
}

/* ========== SUBREG patterns ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    /* Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    
    /* Extract vector element - may generate SUBREG */
    int elem = vec3[global_index % 4];
    
    /* Type punning between different sizes */
    unsigned int int_val = 0x12345678;
    unsigned short short_val = *(unsigned short*)&int_val;  /* SUBREG access */
    
    /* Float/int reinterpretation */
    float f = 3.14159f;
    int int_from_float = *(int*)&f;  /* Bitcast via SUBREG */
    
    /* Mixed size operations */
    long long big = 0x1122334455667788LL;
    int lower = (int)big;  /* Truncation via SUBREG */
    int upper = (int)(big >> 32);
    
    /* Vector element manipulation */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec[3] = elem;  /* SUBREG store */
    
    return elem + short_val + int_from_float + lower + upper + short_vec[0];
}

/* ========== Complex Memory Operands ========== */
struct nested {
    int data[4];
    struct nested* next;
};

NOINLINE unsigned int test_memory_operand(void) {
    /* Complex pointer chasing */
    VOLATILE_VAR struct nested* ptr = malloc(sizeof(struct nested));
    VOLATILE_VAR struct nested** pptr = &ptr;
    VOLATILE_VAR struct nested*** ppptr = &pptr;
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        ptr->data[i] = i * 100;
    }
    ptr->next = malloc(sizeof(struct nested));
    for (int i = 0; i < 4; i++) {
        ptr->next->data[i] = i * 200;
    }
    
    /* Multi-level dereference - forces address walking */
    int val1 = (**ppptr)->data[global_index % 4];
    int val2 = (*(**ppptr)->next).data[(global_index + 1) % 4];
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = global_index;
    int* array = ptr->data;
    int val3 = array[idx % 4] + array[(idx + 1) % 4];
    
    /* Volatile memory operations */
    VOLATILE_VAR int mem_buffer[16];
    for (VOLATILE_VAR int i = 0; i < 16; i++) {
        mem_buffer[i] = i * 50;
    }
    
    /* Complex addressing mode */
    int val4 = mem_buffer[mem_buffer[idx % 16] % 16];
    
    /* Structure field access with pointer arithmetic */
    int* data_ptr = &ptr->next->data[0];
    int val5 = *(data_ptr + (idx % 4));
    
    /* Cleanup */
    free(ptr->next);
    free(ptr);
    
    return val1 + val2 + val3 + val4 + val5;
}

/* ========== Main test driver ========== */
int main(void) {
    unsigned int total = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Initialize global index */
    global_index = 7;
    
    /* Run all tests */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Total checksum: %u\n", total);
    
    return (int)(total % 256);
}

/* Dummy external functions */
int get_index(void) {
    return global_index;
}

void escape(void* p) {
    asm volatile ("" : : "r"(p) : "memory");
}
