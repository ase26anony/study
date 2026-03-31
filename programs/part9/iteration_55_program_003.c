/* test_resource.c - Cover specific SET_DEST patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure functions are compiled separately */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile char g_char = 'A';
volatile short g_short = 1234;

/* ===== ZERO_EXTRACT patterns (bit-field assignments) ===== */

/* Bit-field in struct - volatile to prevent optimization */
struct bitfield_struct {
    volatile unsigned int field3 : 3;
    volatile unsigned int field5 : 5;
    volatile unsigned int field8 : 8;
};

NOINLINE void test_zero_extract(void) {
    struct bitfield_struct s;
    volatile unsigned int v = 0;
    
    /* These assignments should generate ZERO_EXTRACT in RTL */
    s.field3 = 5;      /* 3-bit field */
    s.field5 = 20;     /* 5-bit field */
    s.field8 = 255;    /* 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(s.field3), "r"(s.field5), "r"(s.field8));
}

/* Another ZERO_EXTRACT pattern using direct bit operations */
NOINLINE void test_zero_extract2(int x) {
    volatile unsigned int buffer = 0;
    
    /* Extract and set bits 5-7 */
    unsigned int bits = (x & 0x7) << 5;
    buffer = (buffer & ~(0x7 << 5)) | bits;
    
    asm volatile("" : : "r"(buffer));
}

/* ===== STRICT_LOW_PART patterns (partial register writes) ===== */

/* Compile with -m32 for better STRICT_LOW_PART generation */
NOINLINE void test_strict_low_part(void) {
    volatile int dest_int;
    volatile short src_short = g_short;
    volatile char src_char = g_char;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest_int = src_short;  /* short to int - zero/sign extend */
    dest_int = src_char;   /* char to int */
    
    /* Mixed-size operations */
    int temp = dest_int;
    short *ptr = (short*)&temp;
    *ptr = src_short;      /* Partial write to int */
    
    asm volatile("" : : "r"(dest_int), "r"(temp));
}

/* More aggressive STRICT_LOW_PART generation */
NOINLINE int test_partial_reg_write(int a, short b, char c) {
    int result = a;
    
    /* Force partial register updates */
    *(short*)(&result) = b;      /* Write only lower 16 bits */
    *(char*)(&result + 2) = c;   /* Write a byte */
    
    /* Arithmetic that keeps partial result */
    result = (result & 0xFFFF0000) | (b & 0xFFFF);
    
    return result;
}

/* ===== SUBREG patterns ===== */

/* GCC vector extensions for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v8hi c = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that generate SUBREG */
    a = a + b;                     /* Full vector op */
    int lane = a[0];               /* Extract element - may use SUBREG */
    a[2] = lane;                   /* Insert element */
    
    /* Type punning through union */
    union {
        float f;
        int i;
    } u;
    u.f = 3.14f;
    int int_bits = u.i;           /* float->int bitcast via SUBREG */
    
    /* Mixed vector sizes */
    v8hi d = c + (v8hi){1, 1, 1, 1, 1, 1, 1, 1};
    short first = d[0];           /* Another SUBREG extract */
    
    asm volatile("" : : "r"(a), "r"(lane), "r"(int_bits), "r"(first));
}

/* Complex SUBREG patterns with floating point */
NOINLINE float test_float_subreg(float x, float y) {
    /* Type punning that generates SUBREG */
    int ix = *(int*)&x;
    int iy = *(int*)&y;
    
    /* Manipulate bits */
    int iz = (ix & 0x7F800000) | (iy & 0x007FFFFF);
    float z = *(float*)&iz;
    
    /* Mixed precision */
    double d = (double)x + y;
    float f = (float)d;           /* TRUNCATE/SUBREG pattern */
    
    return z + f;
}

/* ===== MEM_P patterns (complex memory addresses) ===== */

int global_array[100];
struct point {
    int x, y, z;
};
struct point global_points[10];

NOINLINE void test_mem_dest(int index, int value) {
    /* Complex array indexing */
    global_array[index * 2 + 1] = value;
    global_array[g_index] = value * 2;
    
    /* Struct member access through pointer */
    struct point *p = &global_points[index % 10];
    p->x = value;
    p->y = index;
    p->z = value + index;
    
    /* Pointer arithmetic with multiple bases */
    int *ptr = global_array;
    ptr += (index & 0x3F);        /* Complex address calculation */
    *ptr = value;
    
    /* Multi-dimensional access */
    int matrix[10][10];
    matrix[index/10][index%10] = value;
    
    asm volatile("" : : "r"(global_array[0]), "r"(p->x), "r"(matrix[0][0]));
}

/* More complex MEM patterns */
NOINLINE void test_complex_mem(int a, int b, int c) {
    /* Chain of calculations for address */
    int *base = global_array;
    int offset = (a * b + c) & 63;
    int *addr = base + offset;
    
    /* Store with complex address */
    *addr = a ^ b ^ c;
    
    /* Struct with bit-fields in memory */
    struct {
        volatile unsigned int a : 4;
        volatile unsigned int b : 4;
        volatile unsigned int c : 8;
    } __attribute__((packed)) packed_struct;
    
    packed_struct.a = a & 0xF;
    packed_struct.b = b & 0xF;
    packed_struct.c = c & 0xFF;
    
    /* Indirect store through function pointer */
    void (*store_func)(int*, int) = test_mem_dest;
    if (a > 0) {
        store_func(addr, b);
    }
}

/* ===== Combined test function ===== */

/* Function with all patterns mixed together */
NOINLINE void test_combined(int x, int y, int z) {
    /* ZERO_EXTRACT */
    struct bitfield_struct bs;
    bs.field3 = x & 0x7;
    bs.field8 = y & 0xFF;
    
    /* STRICT_LOW_PART */
    int dest = x;
    *(short*)&dest = y & 0xFFFF;
    
    /* SUBREG */
    v4si vec = {x, y, z, x+y};
    int elem = vec[1];
    
    /* MEM_P */
    global_array[(x + y) % 100] = elem;
    
    /* Control flow to prevent single basic block */
    if (dest > 100) {
        bs.field5 = z & 0x1F;
        vec[2] = dest;
    }
    
    for (int i = 0; i < 3; i++) {
        global_array[i] += x;
    }
    
    asm volatile("" : : "r"(bs.field3), "r"(dest), "r"(elem));
}

/* ===== Main driver ===== */

int main(void) {
    /* Initialize globals */
    memset(global_array, 0, sizeof(global_array));
    memset(global_points, 0, sizeof(global_points));
    
    /* Test all patterns */
    test_zero_extract();
    test_zero_extract2(5);
    
    test_strict_low_part();
    int partial = test_partial_reg_write(0x12345678, 0xABCD, 'X');
    
    test_subreg();
    float fval = test_float_subreg(1.0f, 2.0f);
    
    test_mem_dest(25, 100);
    test_complex_mem(1, 2, 3);
    
    test_combined(10, 20, 30);
    test_combined(100, 200, 300);
    test_combined(1000, 2000, 3000);
    
    /* Use results to prevent dead code elimination */
    asm volatile("" : : "r"(partial), "r"(fval));
    
    return 0;
}
