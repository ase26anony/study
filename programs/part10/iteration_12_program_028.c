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

NOINLINE unsigned int test_zero_extract(void) {
    struct bitfield_pack bf1 = {0};
    struct bitfield_pack bf2 = {0};
    struct bitfield_large bf3 = {0};
    
    VOLATILE_VAR unsigned int raw_val = 0xDEADBEEF;
    
    /* Direct bitfield assignments - may generate ZERO_EXTRACT */
    bf1.flag = (raw_val >> 0) & 0x7;
    bf1.value = (raw_val >> 3) & 0x1F;
    bf1.mode = (raw_val >> 8) & 0xF;
    
    /* Bitfield to bitfield copy */
    bf2 = bf1;
    
    /* Large bitfield operations */
    bf3.high = (raw_val >> 16) & 0xFFFF;
    bf3.low = raw_val & 0xFFFF;
    
    /* Complex bit extraction */
    unsigned int extracted = 0;
    extracted |= (bf1.flag & 0x7) << 0;
    extracted |= (bf1.value & 0x1F) << 3;
    extracted |= (bf1.mode & 0xF) << 8;
    
    /* Manual bitfield-like extraction */
    unsigned int val = 0x12345678;
    unsigned int mask = 0x1F;  /* 5 bits */
    unsigned int shift = 7;
    unsigned int field = (val >> shift) & mask;
    
    return extracted + field + bf3.low;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE unsigned int test_strict_low_part(void) {
    VOLATILE_VAR unsigned int int_var = 0x12345678;
    VOLATILE_VAR unsigned short short_var = 0;
    VOLATILE_VAR unsigned char char_var = 0;
    
    /* Byte store into integer - may generate STRICT_LOW_PART */
    *(volatile unsigned char*)&int_var = 0xFF;
    
    /* Union for type punning */
    union {
        unsigned int full;
        unsigned char bytes[4];
    } pun;
    pun.full = 0xDEADBEEF;
    pun.bytes[0] = 0x11;  /* Low byte store */
    
    /* Truncation preserving high bits context */
    unsigned int temp = int_var;
    char_var = temp & 0xFF;  /* Explicit truncation */
    
    /* Multiple byte operations */
    for (int i = 0; i < 4; i++) {
        pun.bytes[i] = (temp >> (i * 8)) & 0xFF;
    }
    
    /* Inline assembly forcing low-part access */
    unsigned int in_asm = 0x87654321;
    unsigned char out_asm = 0;
    
    /* x86 low-byte register access */
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r" (out_asm)
        : "r" (in_asm)
        : "cc"
    );
    
    /* Another assembly pattern */
    unsigned short in2 = 0xABCD;
    unsigned char out2;
    asm volatile (
        "movw %w1, %0\n\t"
        : "=r" (out2)
        : "r" (in2)
        : "cc"
    );
    
    return pun.full + char_var + out_asm + out2;
}

/* ========== SUBREG patterns ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE unsigned int test_subreg(void) {
    /* Vector operations */
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Element extraction - may generate SUBREG */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    
    /* Type punning between float and int */
    float f = 3.14159f;
    unsigned int f_as_int;
    memcpy(&f_as_int, &f, sizeof(f));
    
    /* Cast between different integer sizes */
    unsigned long long big = 0x1122334455667788ULL;
    unsigned int small = (unsigned int)big;  /* Truncation */
    unsigned short smaller = (unsigned short)small;
    
    /* Mixed size operations */
    unsigned int a = 0x12345678;
    unsigned short b = 0x9ABC;
    unsigned int c = a + b;  /* b promoted, but SUBREG in intermediate */
    
    /* Pointer casting for subregister access */
    unsigned int *ptr_int = &a;
    unsigned short *ptr_short = (unsigned short*)ptr_int;
    unsigned short low_half = ptr_short[0];
    unsigned short high_half = ptr_short[1];
    
    /* Vector element manipulation */
    vec_int[1] = elem0 + 100;
    vec_short[3] = low_half;
    
    return elem0 + elem2 + f_as_int + small + c + low_half + vec_int[1];
}

/* ========== Memory operand patterns ========== */
struct nested {
    int data[4];
    struct nested *next;
};

NOINLINE unsigned int test_memory_operand(void) {
    /* Complex memory addressing */
    VOLATILE_VAR int array[256];
    VOLATILE_VAR int *ptr1 = array;
    VOLATILE_VAR int **ptr2 = &ptr1;
    VOLATILE_VAR int ***ptr3 = &ptr2;
    
    /* Initialize */
    for (int i = 0; i < 256; i++) {
        array[i] = i * 3;
    }
    
    /* Multi-level pointer dereference */
    int val1 = ***ptr3;
    int val2 = **(ptr2 + global_index % 64);
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx = get_index() % 256;
    int val3 = array[idx];
    int val4 = array[idx + 1];
    int val5 = array[idx * 2 % 256];
    
    /* Structure with pointer chasing */
    struct nested node1, node2, node3;
    node1.data[0] = 100;
    node1.data[1] = 200;
    node1.next = &node2;
    
    node2.data[0] = 300;
    node2.data[1] = 400;
    node2.next = &node3;
    
    node3.data[0] = 500;
    node3.data[1] = 600;
    node3.next = NULL;
    
    /* Chain dereference */
    int chain_val = node1.next->next->data[1];
    
    /* Volatile memory operations */
    VOLATILE_VAR int *volatile_ptr = (int*)global_ptr;
    if (volatile_ptr) {
        val1 += volatile_ptr[0];
    }
    
    /* Mixed offset calculation */
    int *base = array;
    int offset = idx * sizeof(int);
    int *addr = (int*)((char*)base + offset);
    int val6 = *addr;
    
    /* Pointer arithmetic with different scales */
    short *short_ptr = (short*)array;
    short val7 = short_ptr[idx * 2];  /* Different element size */
    
    return val1 + val2 + val3 + val4 + val5 + chain_val + val6 + val7;
}

/* ========== Main function ========== */
int main(void) {
    unsigned int result = 0;
    
    printf("Starting resource pattern tests...\n");
    
    /* Test each pattern */
    result += test_zero_extract();
    printf("Zero extract test completed\n");
    
    result += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    result += test_subreg();
    printf("Subreg test completed\n");
    
    result += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final result: %u (0x%08X)\n", result, result);
    
    return 0;
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 256;
}

void* get_ptr(void) {
    static char buffer[1024];
    return buffer;
}
