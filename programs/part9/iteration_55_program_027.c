/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ===== ZERO_EXTRACT patterns ===== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment - should generate ZERO_EXTRACT in SET_DEST */
    struct bitfield_s {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    };
    volatile struct bitfield_s bf;
    
    /* Multiple bit-field assignments */
    bf.field1 = 5;          /* Should generate ZERO_EXTRACT for 3-bit field */
    bf.field2 = 0x1F;       /* Should generate ZERO_EXTRACT for 5-bit field */
    bf.field3 = 0xFF;       /* Should generate ZERO_EXTRACT for 8-bit field */
    
    /* Use asm to prevent optimization */
    asm volatile("" : : "r"(bf));
}

/* ===== STRICT_LOW_PART patterns ===== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size integer assignments - may generate STRICT_LOW_PART */
    short src_short = (short)x;
    char src_char = (char)x;
    int dest_int = 0;
    long dest_long = 0;
    
    /* These assignments might generate STRICT_LOW_PART on x86-32 */
    dest_int = src_short;   /* 16-bit to 32-bit */
    dest_long = src_char;   /* 8-bit to 64-bit */
    
    /* Force use of results */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* Compile with -m32 for better STRICT_LOW_PART generation */
NOINLINE void __attribute__((optimize("O1"))) test_strict_low_part_m32(void) {
    /* Explicit 32-bit targeting */
    volatile short s = 0x1234;
    volatile int i;
    
    /* This often generates STRICT_LOW_PART on x86-32 at O1 */
    i = s;  /* SET_DEST is likely STRICT_LOW_PART */
    
    asm volatile("" : : "r"(i));
}

/* ===== SUBREG patterns ===== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - often generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v8hi vec_b = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that might use SUBREG */
    int lane = vec_a[2];            /* Vector extract */
    vec_a[1] = lane;                /* Vector insert - SET_DEST with SUBREG */
    
    /* Type punning through union - can generate SUBREG */
    union {
        float f;
        int i;
    } u;
    u.f = 3.14f;
    int int_bits = u.i;             /* Might use SUBREG for type conversion */
    
    /* Complex number assignment */
    __complex__ float cf = 1.0f + 2.0fi;
    __real__ cf = 3.0f;             /* Real part assignment */
    
    asm volatile("" : : "r"(vec_a), "r"(vec_b), "r"(int_bits), "r"(cf));
}

/* ===== MEM_P patterns ===== */
NOINLINE void test_mem_p(int *base, int offset) {
    /* Complex memory addresses - should generate MEM with non-trivial address */
    int array[100];
    
    /* Store with complex address computation */
    array[g_index * 2 + offset] = g_value;      /* MEM with index computation */
    
    /* Pointer arithmetic store */
    int *ptr = base + (offset & 0xF);
    *ptr = g_value;                            /* MEM with pointer arithmetic */
    
    /* Struct member through pointer */
    struct point {
        int x;
        int y;
        int z;
    } points[10];
    
    points[offset % 10].y = g_value;           /* MEM with struct offset */
    
    /* Multi-dimensional array */
    int matrix[10][10];
    matrix[offset % 10][g_index] = g_value;    /* MEM with 2D indexing */
    
    asm volatile("" : : "r"(array), "r"(ptr), "r"(points), "r"(matrix));
}

/* ===== Combined test function ===== */
NOINLINE void test_combined(int arg) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct {
        unsigned int flags : 4;
        unsigned int mode : 4;
    } settings;
    settings.flags = arg & 0xF;
    settings.mode = (arg >> 4) & 0xF;
    
    /* STRICT_LOW_PART */
    unsigned char byte_val = (unsigned char)arg;
    unsigned int word_val;
    word_val = byte_val;  /* Potential STRICT_LOW_PART */
    
    /* SUBREG */
    typedef float v2f __attribute__((vector_size(8)));
    v2f v = {1.0f, 2.0f};
    float f = v[0];
    v[1] = f * 2.0f;
    
    /* MEM_P */
    int buffer[50];
    int idx = (arg * 17) % 50;
    buffer[idx] = word_val;
    
    /* Control flow to prevent single basic block */
    if (arg & 1) {
        buffer[idx + 1] = settings.flags;
    } else {
        buffer[idx - 1] = settings.mode;
    }
    
    asm volatile("" : : "r"(settings), "r"(word_val), "r"(v), "r"(buffer));
}

/* Helper to return pointer based on condition */
int* get_pointer(int *a, int *b, int condition) {
    return condition ? a : b;
}

/* Main test driver */
int main(void) {
    int data[100];
    int i;
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i * 3;
    }
    
    /* Run all tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i);
        test_subreg();
        test_mem_p(data, i);
        test_combined(i);
        
        /* Call 32-bit optimized version occasionally */
        if (i % 3 == 0) {
            test_strict_low_part_m32();
        }
        
        /* Complex pointer usage for MEM_P */
        int *ptr = get_pointer(&data[0], &data[50], g_condition);
        *ptr = i * 7;
    }
    
    return 0;
}
