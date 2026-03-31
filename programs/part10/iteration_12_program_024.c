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
struct bitfield_s {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int extra:8;
    unsigned int pad:16;
} NOINLINE;

struct packed_bitfield {
    unsigned short a:4;
    unsigned short b:4;
    unsigned short c:4;
    unsigned short d:4;
} NOINLINE;

NOINLINE int test_zero_extract(void) {
    struct bitfield_s bf1 = {0};
    struct packed_bitfield bf2 = {0};
    VOLATILE_VAR unsigned int raw_val = 0xDEADBEEF;
    int result = 0;
    
    /* Bitfield assignments that may generate ZERO_EXTRACT */
    bf1.flag = (raw_val >> 3) & 0x7;      /* Extract 3 bits */
    bf1.value = (raw_val >> 8) & 0x1F;    /* Extract 5 bits */
    bf1.extra = (raw_val >> 13) & 0xFF;   /* Extract 8 bits */
    
    /* More complex extraction */
    bf2.a = (raw_val >> 0) & 0xF;
    bf2.b = (raw_val >> 4) & 0xF;
    bf2.c = (raw_val >> 8) & 0xF;
    bf2.d = (raw_val >> 12) & 0xF;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    unsigned int mask = 0x3F;  /* 6 bits */
    unsigned int shift = 10;
    unsigned int extracted = (raw_val >> shift) & mask;
    
    /* Combine results */
    result = bf1.flag + bf1.value + bf1.extra +
             bf2.a + bf2.b + bf2.c + bf2.d +
             extracted;
    
    return result;
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE int test_strict_low_part(void) {
    VOLATILE_VAR int wide_int = 0x12345678;
    VOLATILE_VAR short short_val = 0;
    VOLATILE_VAR char char_val = 0;
    int result = 0;
    
    /* Union for type punning - may generate low-part accesses */
    union {
        uint32_t full;
        uint8_t bytes[4];
    } pun NOINLINE;
    pun.full = 0xAABBCCDD;
    
    /* Byte-sized stores that may generate STRICT_LOW_PART */
    *(VOLATILE_VAR unsigned char*)&wide_int = 0xFF;  /* Store low byte */
    
    /* Explicit truncation preserving high bits in source */
    char_val = wide_int & 0xFF;  /* Only low byte */
    
    /* Multiple byte operations */
    pun.bytes[0] = (wide_int >> 0) & 0xFF;
    pun.bytes[1] = (wide_int >> 8) & 0xFF;
    pun.bytes[2] = (wide_int >> 16) & 0xFF;
    pun.bytes[3] = (wide_int >> 24) & 0xFF;
    
    /* Inline assembly forcing low-byte register access */
    #ifdef __x86_64__
    asm volatile (
        "movb %b1, %0\n\t"
        : "=r"(char_val)
        : "r"(wide_int)
        : "cc"
    );
    #elif defined(__i386__)
    asm volatile (
        "movb %%al, %0\n\t"
        : "=m"(char_val)
        : "a"(wide_int)
        : "cc"
    );
    #endif
    
    /* Arithmetic that operates on low parts */
    short_val = (wide_int * 2) & 0xFFFF;  /* Keep only low 16 bits */
    
    result = char_val + short_val + pun.bytes[0] + pun.bytes[3];
    return result;
}

/* ========== SUBREG patterns ========== */
NOINLINE int test_subreg(void) {
    /* Vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    int result = 0;
    
    /* Type punning between different sizes */
    float float_val = 3.14159f;
    int int_val;
    
    /* Bitcast float to int - may use SUBREG */
    memcpy(&int_val, &float_val, sizeof(float_val));
    
    /* Extract vector elements - often compiles to SUBREG */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    short selem1 = vec_short[1];
    short selem5 = vec_short[5];
    
    /* Cast between short and int */
    short sval = 0x1234;
    int ival_from_short = (int)sval;      /* Zero/sign extension */
    short sval_from_int = (short)int_val; /* Truncation */
    
    /* Complex type mixing */
    struct {
        int a;
        short b;
        char c;
    } mixed NOINLINE = {100, 200, 300};
    
    /* Access different-sized members */
    int a_val = mixed.a;
    short b_val = mixed.b;
    char c_val = mixed.c;
    
    /* Pointer casting for subregister access */
    int* int_ptr = &mixed.a;
    short* short_ptr = (short*)int_ptr;  /* Aliasing through different types */
    
    result = elem0 + elem2 + selem1 + selem5 +
             ival_from_short + sval_from_int +
             a_val + b_val + c_val + *short_ptr;
    
    return result;
}

/* ========== Memory operand patterns ========== */
NOINLINE int test_memory_operand(void) {
    /* Complex memory addressing structures */
    struct node {
        int value;
        struct node* next;
        struct node* prev;
    };
    
    /* Multi-level array */
    int array3d[3][3][3] NOINLINE;
    VOLATILE_VAR int*** ptr3d = (int***)array3d;
    
    /* Volatile indices to prevent constant propagation */
    VOLATILE_VAR int i = get_index() % 3;
    VOLATILE_VAR int j = (get_index() + 1) % 3;
    VOLATILE_VAR int k = (get_index() + 2) % 3;
    
    int result = 0;
    
    /* Initialize array */
    for (int a = 0; a < 3; a++) {
        for (int b = 0; b < 3; b++) {
            for (int c = 0; c < 3; c++) {
                array3d[a][b][c] = a * 100 + b * 10 + c;
            }
        }
    }
    
    /* Complex memory addressing with multiple dereferences */
    result += array3d[i][j][k];           /* 3D array access */
    result += *(*(array3d[i] + j) + k);   /* Pointer arithmetic version */
    
    /* Multi-level pointer chasing */
    int val1 = 42;
    int* p1 = &val1;
    int** p2 = &p1;
    int*** p3 = &p2;
    
    result += ***p3;  /* Triple dereference */
    
    /* Structure field access with pointer arithmetic */
    struct node nodes[5] NOINLINE;
    for (int idx = 0; idx < 5; idx++) {
        nodes[idx].value = idx * 10;
        nodes[idx].next = (idx < 4) ? &nodes[idx + 1] : NULL;
        nodes[idx].prev = (idx > 0) ? &nodes[idx - 1] : NULL;
    }
    
    /* Chase pointers through structure */
    struct node* current = &nodes[0];
    for (int idx = 0; idx < 3 && current != NULL; idx++) {
        result += current->value;
        current = current->next;
    }
    
    /* Volatile memory operations */
    VOLATILE_VAR int volatile_buffer[100];
    for (VOLATILE_VAR int idx = 0; idx < 50; idx++) {
        volatile_buffer[idx] = idx * 2;
        result += volatile_buffer[idx];
    }
    
    /* Indirect call through function pointer */
    int (*func_ptr)(void) = &test_zero_extract;
    result += func_ptr();
    
    return result;
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    printf("Testing resource pattern coverage...\n");
    
    /* Test each pattern */
    total += test_zero_extract();
    printf("Zero extract test completed\n");
    
    total += test_strict_low_part();
    printf("Strict low part test completed\n");
    
    total += test_subreg();
    printf("Subreg test completed\n");
    
    total += test_memory_operand();
    printf("Memory operand test completed\n");
    
    printf("Final checksum: %d\n", total);
    
    return total != 0 ? 0 : 1;
}

/* Dummy external functions */
int get_index(void) {
    static int counter = 0;
    return counter++ % 100;
}

void* get_ptr(void) {
    static char buffer[1024];
    return buffer;
}
