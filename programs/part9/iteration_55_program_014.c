/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in RTL */
    struct {
        volatile unsigned int field1 : 3;
        volatile unsigned int field2 : 5;
        volatile unsigned int field3 : 8;
    } bit_struct;
    
    bit_struct.field1 = 5;      /* ZERO_EXTRACT for 3-bit field */
    bit_struct.field2 = 31;     /* ZERO_EXTRACT for 5-bit field */
    bit_struct.field3 = 255;    /* ZERO_EXTRACT for 8-bit field */
    
    /* Volatile bit-field in union */
    union {
        volatile unsigned int full;
        struct {
            volatile unsigned int low : 16;
            volatile unsigned int high : 16;
        } parts;
    } bit_union;
    
    bit_union.parts.low = 0xABCD;   /* ZERO_EXTRACT for 16-bit field */
    bit_union.parts.high = 0x1234;  /* Another ZERO_EXTRACT */
    
    /* Force use to prevent optimization */
    asm volatile("" : : "r"(bit_struct.field1), "r"(bit_union.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE __attribute__((optimize("O2"))) 
void test_strict_low_part(short src_short, char src_char) {
    int dest_int;
    long dest_long;
    
    /* Mixed-size assignments - may generate STRICT_LOW_PART */
    dest_int = src_short;      /* short -> int, possible STRICT_LOW_PART */
    dest_long = src_char;      /* char -> long, possible STRICT_LOW_PART */
    
    /* Explicit truncation */
    unsigned int wide = 0x12345678;
    unsigned short narrow = (unsigned short)wide;
    dest_int = narrow;         /* Another potential STRICT_LOW_PART */
    
    /* Use volatile to prevent optimization */
    volatile int use_int = dest_int;
    volatile long use_long = dest_long;
    
    /* Inline assembly that might generate partial register writes */
    asm volatile(
        "movw %w1, %0\n\t"    /* 16-bit move to 32/64-bit reg */
        : "=r"(dest_int)
        : "r"(src_short)
    );
}

/* ==================== SUBREG patterns ==================== */
NOINLINE __attribute__((optimize("O3")))
void test_subreg(void) {
    /* GCC vector types - SUBREG common with vectors */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that generate SUBREG */
    vec_a = vec_a + vec_b;          /* Vector operation */
    int lane = vec_a[g_index & 3];  /* Lane extract - SUBREG as destination */
    
    /* Type punning through unions - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    
    pun.f = 3.14159f;
    int int_bits = pun.i;           /* SUBREG for type conversion */
    
    /* Complex number assignment */
    __complex__ float cf = 1.0f + 2.0fi;
    __complex__ double cd;
    cd = cf;                        /* Possible SUBREG for conversion */
    
    /* Use results */
    asm volatile("" : : "r"(lane), "r"(int_bits), "r"(cd));
}

/* ==================== MEM_P with complex addressing ==================== */
NOINLINE __attribute__((optimize("O2")))
void test_mem_dest(int *base, int offset, int value) {
    int array[100];
    int *ptr;
    
    /* Complex addressing modes */
    array[g_index * 2 + offset] = value;      /* MEM with index computation */
    
    /* Pointer arithmetic */
    ptr = base + (g_index * 3);               /* Non-constant offset */
    *ptr = value * 2;                         /* MEM store */
    
    /* Struct with pointer member */
    struct {
        int data[10];
        int *next;
    } s;
    
    s.data[offset % 10] = value;              /* MEM with struct member */
    s.next = &array[g_index];
    *s.next = value + 1;                      /* MEM through pointer member */
    
    /* Multi-dimensional array */
    int matrix[10][10];
    matrix[offset % 10][g_index % 10] = value; /* MEM with 2D indexing */
    
    /* Prevent dead store elimination */
    asm volatile("" : : "m"(array[0]), "m"(matrix[0][0]), "r"(ptr));
}

/* ==================== Combined test function ==================== */
NOINLINE __attribute__((optimize("O1")))
void combined_test(int arg) {
    /* Mix all patterns in one function */
    volatile struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } bits;
    
    bits.bf1 = arg & 0xF;          /* ZERO_EXTRACT */
    bits.bf2 = (arg >> 4) & 0xFFF; /* Another ZERO_EXTRACT */
    
    short s = arg & 0xFFFF;
    int i = s;                     /* Possible STRICT_LOW_PART */
    
    /* Vector extract */
    typedef float v4f __attribute__((vector_size(16)));
    v4f v = {1.0f, 2.0f, 3.0f, 4.0f};
    float f = v[arg & 3];          /* SUBREG extract */
    
    /* Memory store with addressing */
    int buffer[50];
    buffer[(arg * 7) % 50] = i;    /* MEM store */
    
    /* Use all results */
    asm volatile("" : : "r"(bits.bf1), "r"(i), "r"(f), "m"(buffer[0]));
}

/* ==================== Main driver ==================== */
int main(void) {
    int test_array[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        test_array[i] = i * i;
    }
    
    /* Call test functions with different arguments */
    test_zero_extract();
    
    for (int i = 0; i < 10; i++) {
        test_strict_low_part(i * 100, i * 10);
        test_subreg();
        test_mem_dest(test_array, i, g_value + i);
        combined_test(i * 12345);
    }
    
    /* Additional stress test */
    __attribute__((optimize("O3"))) {
        volatile int x = 0;
        for (int i = 0; i < 1000; i++) {
            x += test_array[i % 100];
        }
        asm volatile("" : : "r"(x));
    }
    
    return 0;
}
