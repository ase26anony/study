/* test_resources.c - Generate RTL patterns for GCC resource tracking coverage */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* Prevent optimization and inlining */
#define NOINLINE __attribute__((noinline, optimize("O0")))
#define VOLATILE_VAR volatile

/* External function to prevent constant propagation */
extern int opaque(int x);
extern void* opaque_ptr(void* p);

/* ========== ZERO_EXTRACT patterns ========== */

/* Bitfield structure for ZERO_EXTRACT */
struct bitfields {
    unsigned int flag:3;
    unsigned int value:5;
    unsigned int mode:4;
    unsigned int pad:20;
} NOINLINE;

/* Complex bitfield operations */
NOINLINE static int test_zero_extract(void) {
    VOLATILE_VAR struct bitfields bf1, bf2;
    VOLATILE_VAR unsigned int raw1, raw2;
    int result = 0;
    
    /* Initialize with opaque values */
    raw1 = opaque(0x12345678);
    raw2 = opaque(0x9ABCDEF0);
    
    /* Direct bitfield assignments (may generate ZERO_EXTRACT) */
    bf1.flag = (raw1 >> 0) & 0x7;
    bf1.value = (raw1 >> 3) & 0x1F;
    bf1.mode = (raw1 >> 8) & 0xF;
    
    /* Bitfield to bitfield copy */
    bf2 = bf1;
    
    /* Extract bitfields through masking */
    result += bf2.flag;
    result += bf2.value << 3;
    result += bf2.mode << 8;
    
    /* Manual bit extraction that may compile to ZERO_EXTRACT */
    VOLATILE_VAR unsigned int x = raw1;
    VOLATILE_VAR unsigned int y = raw2;
    
    /* Multiple extractions with different widths */
    unsigned int ext1 = (x >> 5) & 0x3FF;      /* 10-bit extract */
    unsigned int ext2 = (y >> 2) & 0x7FFF;     /* 15-bit extract */
    unsigned int ext3 = (x >> 15) & 0x1FF;     /* 9-bit extract */
    
    result += ext1 + ext2 + ext3;
    
    /* Nested extractions */
    unsigned int tmp = (x & 0xFF00) >> 8;
    unsigned int final = (tmp >> 2) & 0x3F;    /* 6-bit extract from middle */
    
    return result + final;
}

/* ========== STRICT_LOW_PART patterns ========== */

/* Union for type punning */
union reg_pun {
    uint32_t full;
    uint8_t bytes[4];
    uint16_t halves[2];
} NOINLINE;

NOINLINE static int test_strict_low_part(void) {
    VOLATILE_VAR uint32_t reg = opaque(0xDEADBEEF);
    VOLATILE_VAR union reg_pun pun;
    int result = 0;
    
    /* Byte store into integer (may generate STRICT_LOW_PART) */
    pun.full = reg;
    
    /* Store to low byte only */
    pun.bytes[0] = 0xAA;
    result += pun.full;
    
    /* Store to second byte */
    pun.bytes[1] = 0xBB;
    result += pun.full;
    
    /* Truncation to low part with arithmetic */
    VOLATILE_VAR uint32_t big = opaque(0x12345678);
    uint8_t low_byte = big & 0xFF;          /* Explicit truncation */
    uint16_t low_half = big & 0xFFFF;       /* Low 16-bit truncation */
    
    result += low_byte;
    result += low_half;
    
    /* Inline assembly forcing low-part access */
    uint32_t in_val = opaque(0x87654321);
    uint32_t out_val;
    
    /* x86 byte register access */
    asm volatile (
        "movb %b1, %b0\n\t"
        : "=r"(out_val)
        : "r"(in_val)
        : "cc"
    );
    
    result += out_val;
    
    /* More complex: preserve high bits while modifying low */
    VOLATILE_VAR uint32_t preserve = 0xA5A5A5A5;
    preserve = (preserve & 0xFFFFFF00) | 0x77;  /* Replace only low byte */
    
    result += preserve;
    
    return result;
}

/* ========== SUBREG patterns ========== */

/* Vector types for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE static int test_subreg(void) {
    VOLATILE_VAR v4si vec_int = { opaque(1), opaque(2), opaque(3), opaque(4) };
    VOLATILE_VAR v4sf vec_float = { 1.0f, 2.0f, 3.0f, 4.0f };
    VOLATILE_VAR int scalar = opaque(42);
    int result = 0;
    
    /* Extract vector elements (generates SUBREG) */
    int elem0 = vec_int[0];
    int elem2 = vec_int[2];
    result += elem0 + elem2;
    
    /* Type punning between int and float */
    float f = vec_float[1];
    int i;
    memcpy(&i, &f, sizeof(int));  /* Type punning via memcpy */
    result += i;
    
    /* Cast between different integer sizes */
    VOLATILE_VAR int32_t i32 = opaque(0x12345678);
    int16_t i16 = (int16_t)i32;      /* Truncating cast */
    int8_t i8 = (int8_t)i32;         /* Another truncation */
    
    result += i16;
    result += i8;
    
    /* Union for subregister access */
    union {
        double d;
        uint32_t parts[2];
    } u;
    u.d = 3.14159;
    result += u.parts[0];  /* Access low 32 bits of double */
    
    /* Vector lane extraction with shuffle */
    v8hi short_vec = { 1, 2, 3, 4, 5, 6, 7, 8 };
    short s = short_vec[3];  /* SUBREG access to vector element */
    result += s;
    
    /* Mixed-size operations */
    VOLATILE_VAR long long ll = 0x1122334455667788LL;
    int low_part = (int)ll;          /* Extract low 32 bits */
    int high_part = (int)(ll >> 32); /* Extract high 32 bits */
    
    result += low_part + high_part;
    
    return result;
}

/* ========== Complex Memory Operands ========== */

/* Complex structure for memory addressing */
struct nested {
    int data[4];
    struct nested *next;
};

NOINLINE static int test_memory_operand(void) {
    VOLATILE_VAR int buffer[100];
    VOLATILE_VAR struct nested nodes[5];
    VOLATILE_VAR int ***triple_ptr;
    int result = 0;
    
    /* Initialize with opaque values */
    for (int i = 0; i < 100; i++) {
        buffer[i] = opaque(i);
    }
    
    /* Complex array indexing with volatile index */
    VOLATILE_VAR int idx1 = opaque(10) % 50;
    VOLATILE_VAR int idx2 = opaque(20) % 50;
    
    result += buffer[idx1];      /* Non-constant index */
    result += buffer[idx1 + 5];  /* Index with offset */
    result += buffer[idx2 * 2];  /* Index with scaling */
    
    /* Multi-level pointer dereference */
    int **ptr2 = (int**)opaque_ptr(&buffer[0]);
    int *ptr1 = (int*)opaque_ptr(&buffer[10]);
    int val = **ptr2;            /* Double dereference */
    result += val;
    
    /* Structure field access with pointer chasing */
    for (int i = 0; i < 4; i++) {
        nodes[i].data[0] = opaque(i * 10);
        nodes[i].data[1] = opaque(i * 20);
        nodes[i].next = &nodes[i + 1];
    }
    nodes[4].next = NULL;
    
    /* Chain of structure accesses */
    struct nested *current = &nodes[0];
    result += current->data[1];              /* Direct field */
    result += current->next->data[0];        /* Indirect field */
    result += current->next->next->data[2];  /* Double indirect */
    
    /* Volatile memory operations */
    VOLATILE_VAR int *volatile vol_ptr = &buffer[50];
    result += *vol_ptr;          /* Volatile read */
    *vol_ptr = result;           /* Volatile write */
    
    /* Complex address calculation */
    int *addr = &buffer[0] + idx1 * 2 + idx2;
    result += *addr;
    
    /* Pointer arithmetic with different scales */
    char *char_ptr = (char*)buffer;
    int *int_ptr = (int*)buffer;
    
    char_ptr += idx1 * sizeof(int);  /* Byte offset */
    int_ptr += idx2;                 /* Int offset */
    
    result += *char_ptr;
    result += *int_ptr;
    
    return result;
}

/* ========== Main test driver ========== */

/* Opaque functions to prevent optimization */
int opaque(int x) {
    static VOLATILE_VAR int seed = 0x1234;
    seed = seed * 1103515245 + 12345;
    return x ^ (seed & 0x7FFF);
}

void* opaque_ptr(void* p) {
    static VOLATILE_VAR int counter = 0;
    counter++;
    return (void*)((uintptr_t)p + (counter & 0xF));
}

int main(void) {
    int total = 0;
    
    printf("Testing GCC resource tracking patterns...\n");
    
    /* Run all pattern tests */
    total += test_zero_extract();
    total += test_strict_low_part();
    total += test_subreg();
    total += test_memory_operand();
    
    printf("Result checksum: %d\n", total);
    printf("(Non-zero indicates all tests executed)\n");
    
    return total == 0 ? 1 : 0;
}
