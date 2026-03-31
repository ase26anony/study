/* Test program to trigger uncovered lines in resource.cc */
#include <stdint.h>
#include <string.h>

/* Prevent optimizations from removing our test patterns */
#define NOOPT __attribute__((noinline, noipa))

/* Volatile to force actual operations */
volatile int global_counter = 0;

/* ===== ZERO_EXTRACT Pattern ===== */
/* Using bit-fields to generate ZERO_EXTRACT RTL */
struct bitfield_struct {
    unsigned int full : 32;
    unsigned int part1 : 8;
    unsigned int part2 : 4;
    unsigned int part3 : 12;
    unsigned int part4 : 8;
} __attribute__((packed));

/* Also test with volatile bit-fields */
struct volatile_bitfield {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 24;
};

NOOPT void test_zero_extract(void) {
    struct bitfield_struct bf;
    struct volatile_bitfield vbf;
    
    /* Initialize to prevent dead code elimination */
    bf.full = 0x12345678;
    bf.part1 = 0xAB;
    bf.part2 = 0x7;
    bf.part3 = 0xABC;
    bf.part4 = 0xCD;
    
    /* Operations that should generate ZERO_EXTRACT */
    bf.part1 = (bf.part2 << 1) | 0x1;      /* Bit-field assignment */
    bf.part3 = bf.part1 ^ bf.part4;        /* Another bit-field op */
    
    /* Volatile bit-field operations */
    vbf.field1 = 0x3;
    vbf.field2 = vbf.field1 << 2;
    vbf.field3 = vbf.field2 * 17;
    
    /* Complex bit-field expression */
    bf.part4 = ((bf.part1 & 0xF) << 4) | (bf.part2 & 0xF);
    
    global_counter += bf.part1 + bf.part2 + bf.part3 + bf.part4;
    global_counter += vbf.field1 + vbf.field2 + vbf.field3;
}

/* ===== STRICT_LOW_PART Pattern ===== */
NOOPT void test_strict_low_part(void) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val = 0x12345678;
    
    /* These assignments to partial registers may generate STRICT_LOW_PART */
    s_val = (short)(i_val & 0xFFFF);      /* Low 16-bit assignment */
    c_val = (char)(i_val & 0xFF);         /* Low 8-bit assignment */
    
    /* More complex low-part operations */
    i_val = (i_val & 0xFFFFFF00) | (c_val + 1);
    i_val = (i_val & 0xFFFF0000) | (s_val * 2);
    
    /* Inline assembly that explicitly uses low-part modifiers */
    int x = 42;
    short y;
    
    asm volatile (
        "movw %w1, %0\n\t"          /* %w1 for word (16-bit) operand */
        : "=r" (y)
        : "r" (x)
        : "cc"
    );
    
    /* Another assembly with byte operation */
    char z;
    asm volatile (
        "movb %b1, %0\n\t"          /* %b1 for byte operand */
        : "=r" (z)
        : "r" (x)
        : "cc"
    );
    
    global_counter += s_val + c_val + y + z;
}

/* ===== SUBREG Pattern ===== */
NOOPT void test_subreg(void) {
    /* Use unions for type-punning to generate SUBREG */
    union {
        uint32_t full;
        uint16_t halves[2];
        uint8_t bytes[4];
    } data;
    
    data.full = 0x89ABCDEF;
    
    /* Operations on sub-parts that may generate SUBREG */
    data.halves[0] = data.halves[1] ^ 0x55AA;
    data.bytes[2] = data.bytes[0] + data.bytes[1];
    
    /* Vector types can also generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Extract and operate on elements */
    int elem = vec1[2];              /* May generate SUBREG */
    vec1[3] = elem * 2;              /* Another SUBREG possibility */
    
    /* Packed structure operations */
    struct packed {
        int16_t a;
        int16_t b;
        int32_t c;
    } __attribute__((packed)) packed_struct;
    
    packed_struct.a = 100;
    packed_struct.b = 200;
    packed_struct.c = packed_struct.a * packed_struct.b;
    
    global_counter += data.full + elem + packed_struct.c;
}

/* ===== MEM_P with Complex Addressing Pattern ===== */
NOOPT void test_complex_mem(void) {
    /* Multi-dimensional array with complex indexing */
    volatile int arr[10][10][10];
    int i, j, k;
    
    /* Initialize array */
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            for (k = 0; k < 10; k++) {
                arr[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Complex memory accesses that should generate non-trivial addresses */
    int sum = 0;
    volatile int *ptr;
    
    /* Chain of pointer arithmetic */
    ptr = &arr[0][0][0];
    for (i = 0; i < 5; i++) {
        for (j = 0; j < 5; j++) {
            /* Complex address calculation */
            sum += *(ptr + i * 100 + j * 10 + (i ^ j));
            
            /* More complex: nested array access with computation */
            sum += arr[i][j][(i * j) % 10];
            
            /* Pointer chain */
            int ***ptr3 = (int***)arr;
            sum += ptr3[i][j][(i + j) % 10];
        }
    }
    
    /* Structure with pointer chains */
    struct node {
        int value;
        struct node *next;
        struct node *prev;
    };
    
    struct node nodes[10];
    for (i = 0; i < 9; i++) {
        nodes[i].value = i * 10;
        nodes[i].next = &nodes[i + 1];
        nodes[i].prev = (i > 0) ? &nodes[i - 1] : 0;
    }
    nodes[9].value = 90;
    nodes[9].next = 0;
    nodes[9].prev = &nodes[8];
    
    /* Complex structure access chain */
    sum += nodes[0].next->next->next->value;
    sum += nodes[5].prev->prev->value;
    
    /* Inline assembly with memory operand */
    int x = 0;
    asm volatile (
        "addl $1, %0\n\t"
        : "+m" (arr[3][4][5])   /* Memory operand */
        :
        : "cc"
    );
    
    global_counter += sum + arr[0][0][0] + x;
}

/* ===== Combined Test Function ===== */
NOOPT void test_combined(void) {
    /* Try to combine multiple patterns in one function */
    volatile struct {
        unsigned int bitfield : 10;
        unsigned int full;
        short low_part;
        char byte_part;
    } combined;
    
    /* ZERO_EXTRACT pattern */
    combined.bitfield = (combined.full >> 5) & 0x3FF;
    
    /* STRICT_LOW_PART pattern */
    combined.low_part = (short)(combined.full & 0xFFFF);
    combined.byte_part = (char)(combined.full & 0xFF);
    
    /* Complex memory access */
    volatile int *ptr = &combined.full;
    *(ptr + 1) = combined.low_part;  /* Access with offset */
    
    global_counter += combined.bitfield + combined.low_part + combined.byte_part;
}

/* Main function that calls all tests */
int main(void) {
    /* Initialize global to prevent optimization */
    global_counter = 1;
    
    /* Run all test patterns */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    test_complex_mem();
    test_combined();
    
    /* Additional iterations to increase coverage chances */
    for (int i = 0; i < 3; i++) {
        test_zero_extract();
        test_strict_low_part();
    }
    
    /* Return something based on computations to prevent dead code elimination */
    return global_counter == 0 ? 0 : 1;
}
