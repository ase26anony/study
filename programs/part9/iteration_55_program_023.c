/* test_resource.c - Cover specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int global_index = 3;
volatile int global_value = 42;

/* ===== ZERO_EXTRACT patterns (bit-field assignments) ===== */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Use volatile to prevent optimization */
volatile struct bitfield_struct volatile_bf;

NOINLINE void test_zero_extract(void) {
    /* These assignments should generate ZERO_EXTRACT in RTL */
    volatile_bf.field1 = 5;      /* 3-bit field */
    volatile_bf.field2 = 20;     /* 5-bit field */
    volatile_bf.field3 = 100;    /* 8-bit field */
    volatile_bf.field4 = 30000;  /* 16-bit field */
    
    /* Compound assignment to bit-field */
    volatile_bf.field1 |= 2;
    volatile_bf.field2 &= 0x0F;
}

/* ===== STRICT_LOW_PART patterns (partial register writes) ===== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size assignments that may generate STRICT_LOW_PART */
    short src_short = (short)x;
    char src_char = (char)x;
    
    /* These assignments may use partial register writes */
    int dest_int = src_short;    /* short -> int */
    long dest_long = src_char;   /* char -> long */
    
    /* Force use of results */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
    
    /* Pointer casting with smaller types */
    int value = x;
    char *ptr = (char *)&value;
    ptr[1] = (char)(x >> 8);  /* Partial write to integer */
}

/* ===== SUBREG patterns (register subset operations) ===== */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector element extraction - may use SUBREG */
    int lane0 = vec_int[0];
    float lane1 = vec_float[1];
    
    /* Type punning through union - often generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    int int_bits = pun.i;  /* float bits as int */
    
    /* Complex number operations */
    _Complex float comp = 1.0f + 2.0fi;
    __real__ comp = 3.0f;  /* May use SUBREG for complex part access */
    
    asm volatile("" : : "r"(lane0), "r"(lane1), "r"(int_bits));
}

/* ===== MEM_P patterns (complex memory destinations) ===== */
struct nested {
    int data[4];
    struct {
        int x, y;
    } point;
};

NOINLINE void test_mem_dest(struct nested *n, int idx) {
    /* Complex array indexing */
    n->data[idx] = global_value;
    n->data[idx + 1] = n->data[idx] * 2;
    
    /* Struct member through pointer with offset */
    n->point.x = idx;
    n->point.y = global_value;
    
    /* Pointer arithmetic with non-constant offset */
    int *ptr = &n->data[0];
    ptr[global_index] = idx * 3;
    
    /* Multi-dimensional array-like access */
    char buffer[16][16];
    buffer[idx][global_index] = (char)idx;
    
    /* Force memory barrier */
    asm volatile("" : : "m"(*n), "m"(buffer));
}

/* ===== Combined test with all patterns ===== */
NOINLINE __attribute__((optimize("O2"))) 
void combined_test(int param) {
    /* ZERO_EXTRACT */
    struct bitfield_struct local_bf;
    local_bf.field1 = param & 0x07;
    local_bf.field3 = (param >> 8) & 0xFF;
    
    /* STRICT_LOW_PART */
    unsigned char small = param & 0xFF;
    unsigned int medium = small;  /* Zero-extension or partial write */
    
    /* SUBREG */
    v4si vec = {param, param+1, param+2, param+3};
    int first = vec[0];
    
    /* MEM_P */
    int array[10];
    for (int i = 0; i < 10; i++) {
        array[i] = param + i;  /* Store with loop index */
    }
    
    /* Mix operations to prevent optimization */
    if (param > 100) {
        local_bf.field2 = medium & 0x1F;
        vec[1] = array[global_index];
    }
    
    /* Use all results */
    asm volatile("" : : "r"(local_bf.field1), "r"(first), "m"(array));
}

/* ===== Main test driver ===== */
int main(void) {
    /* Initialize */
    struct nested n;
    memset(&n, 0, sizeof(n));
    
    /* Test each pattern individually */
    test_zero_extract();
    test_strict_low_part(global_value);
    test_subreg();
    test_mem_dest(&n, global_index);
    
    /* Test combined patterns with different parameters */
    for (int i = 0; i < 10; i++) {
        combined_test(i * 50);
    }
    
    /* Additional stress test for MEM_P with complex addressing */
    int matrix[8][8];
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            matrix[i][j] = i * 8 + j;  /* 2D array store */
        }
    }
    
    /* Test with volatile pointer */
    volatile int *volatile_ptr = (volatile int *)matrix;
    volatile_ptr[global_index] = 999;
    
    return 0;
}
