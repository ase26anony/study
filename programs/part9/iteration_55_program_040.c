/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ===== ZERO_EXTRACT patterns ===== */
/* Bit-field assignments often generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

/* Use volatile to prevent optimization */
volatile struct bitfield_struct g_bf;

NOINLINE void test_zero_extract(int x) {
    /* This should generate ZERO_EXTRACT for SET_DEST */
    struct bitfield_struct local_bf;
    local_bf.field1 = x & 0x7;          /* 3-bit field */
    local_bf.field2 = (x >> 3) & 0x1F;  /* 5-bit field */
    local_bf.field3 = (x >> 8) & 0xFF;  /* 8-bit field */
    
    /* Force use of volatile global */
    g_bf.field1 = local_bf.field1;
    g_bf.field2 = local_bf.field2;
    g_bf.field3 = local_bf.field3;
    
    /* Complex bit-field with volatile */
    volatile struct {
        unsigned int a : 4;
        unsigned int b : 12;
        unsigned int c : 16;
    } vbf;
    
    vbf.a = x & 0xF;
    vbf.b = (x * 2) & 0xFFF;
    vbf.c = (x + 100) & 0xFFFF;
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes, especially on 32-bit x86 */
NOINLINE void test_strict_low_part(short s, char c, int i) {
    /* These assignments may generate STRICT_LOW_PART */
    int dest1, dest2, dest3;
    
    /* short to int assignment */
    dest1 = s;  /* May use STRICT_LOW_PART for 16-bit write */
    
    /* char to int assignment */
    dest2 = c;  /* May use STRICT_LOW_PART for 8-bit write */
    
    /* Mixed operations that force partial register updates */
    dest3 = i;
    dest3 = (dest3 & 0xFFFF0000) | (s & 0xFFFF);
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(dest1), "r"(dest2), "r"(dest3));
    
    /* Pointer casting with different sizes */
    int *ptr = &i;
    short *sptr = (short *)ptr;
    *sptr = s;  /* Partial store through pointer */
}

/* ===== SUBREG patterns ===== */
/* Using GCC vector extensions to generate SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(int a, int b, int c, int d) {
    v4si vec1 = {a, b, c, d};
    v4si vec2 = {d, c, b, a};
    
    /* Vector operations that may use SUBREG */
    v4si result = vec1 + vec2;
    
    /* Extract lane - may use SUBREG */
    int lane0 = result[0];
    int lane1 = result[1];
    
    /* Type punning through union */
    union {
        float f;
        int i;
    } pun;
    pun.f = (float)a;
    int int_from_float = pun.i;  /* May involve SUBREG */
    
    /* Mixed-size vector operations */
    v8hi short_vec = {a, b, c, d, a*2, b*2, c*2, d*2};
    short_vec[3] = (short)(a + b);
    
    /* Prevent optimization */
    asm volatile("" : : "r"(lane0), "r"(lane1), "r"(int_from_float));
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses for MEM destinations */
int global_array[100];

NOINLINE int* get_pointer(int idx) {
    return &global_array[idx * 2 + (g_condition ? 1 : 0)];
}

NOINLINE void test_mem_dest(int idx, int val) {
    /* Complex array indexing */
    global_array[idx * 3 + g_index] = val;
    
    /* Pointer arithmetic with multiple operations */
    int *ptr = get_pointer(idx);
    *ptr = val * 2;
    
    /* Struct with pointer member */
    struct data {
        int values[10];
        int count;
    };
    
    struct data d;
    d.values[idx % 10] = val;
    d.count = idx;
    
    /* Multi-dimensional array */
    int matrix[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            matrix[i][j] = idx + i * j;
        }
    }
    
    /* Indirect store through double pointer */
    int target = val;
    int *direct_ptr = &target;
    int **indirect_ptr = &direct_ptr;
    **indirect_ptr = val + 1;
}

/* ===== Combined test function ===== */
/* Function that mixes all patterns */
NOINLINE void test_combined(int x, short s, char c, int idx) {
    /* ZERO_EXTRACT pattern */
    volatile struct {
        unsigned int low : 10;
        unsigned int high : 22;
    } comb_bf;
    comb_bf.low = x & 0x3FF;
    comb_bf.high = (x >> 10) & 0x3FFFFF;
    
    /* STRICT_LOW_PART pattern */
    int partial_dest;
    partial_dest = s;  /* short to int */
    
    /* SUBREG pattern with vectors */
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf fvec = {x * 0.1f, x * 0.2f, x * 0.3f, x * 0.4f};
    float first = fvec[0];
    
    /* MEM_P pattern with complex address */
    int local_arr[20];
    local_arr[(idx * 7 + 3) % 20] = x;
    
    /* Use all results to prevent elimination */
    asm volatile("" : : "r"(comb_bf.low), "r"(partial_dest), "r"(first));
}

/* Main function to drive all tests */
int main(int argc, char **argv) {
    /* Initialize with non-constant values */
    int base = argc > 1 ? atoi(argv[1]) : 12345;
    
    /* Test each pattern multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract(base + i);
        test_strict_low_part((short)(base + i), (char)(base + i), base + i * 2);
        test_subreg(base + i, base + i + 1, base + i + 2, base + i + 3);
        test_mem_dest(i, base + i * 10);
        test_combined(base + i, (short)(base + i), (char)(base + i), i);
    }
    
    /* Additional stress test with larger inputs */
    for (int i = 0; i < 100; i++) {
        int idx = i % 50;
        global_array[idx] = test_zero_extract(base + idx);
    }
    
    return 0;
}
