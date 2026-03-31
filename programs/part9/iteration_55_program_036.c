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
    } s;
    
    s.field1 = 5;      /* ZERO_EXTRACT destination */
    s.field2 = 31;     /* Another bit-field write */
    s.field3 = s.field1 + s.field2;  /* Bit-field read and write */
    
    /* Volatile bit-field in union */
    union {
        volatile unsigned int full;
        struct {
            volatile unsigned int low : 16;
            volatile unsigned int high : 16;
        } parts;
    } u;
    
    u.parts.low = 0xABCD;   /* ZERO_EXTRACT */
    u.parts.high = 0x1234;  /* ZERO_EXTRACT */
    
    /* Force use to prevent elimination */
    asm volatile("" : : "r"(s.field1), "r"(u.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size assignments - may generate STRICT_LOW_PART on x86 */
    short src_short = x & 0xFFFF;
    int dest_int = 0;
    
    /* This assignment might use STRICT_LOW_PART to write only lower 16 bits */
    dest_int = src_short;  /* Potential STRICT_LOW_PART destination */
    
    /* Character to integer assignment */
    char src_char = x & 0xFF;
    long dest_long = 0;
    dest_long = src_char;  /* Another potential STRICT_LOW_PART */
    
    /* Byte operations with masking */
    uint32_t val32 = 0x12345678;
    uint8_t byte = 0xAA;
    val32 = (val32 & 0xFFFFFF00) | byte;  /* Low byte write */
    
    /* Force register usage */
    asm volatile("" : : "r"(dest_int), "r"(dest_long), "r"(val32));
}

/* ==================== SUBREG patterns ==================== */
NOINLINE void test_subreg(void) {
    /* GCC vector types - often generate SUBREG operations */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    /* Vector operations that might use SUBREG */
    vec_a = vec_a + vec_b;  /* Vector operation */
    
    /* Extract lane to scalar - may use SUBREG */
    int lane0 = vec_a[0];   /* SUBREG for extraction */
    int lane1 = vec_a[1];
    
    /* Type punning through union - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    int int_bits = pun.i;  /* SUBREG for type conversion */
    
    /* Complex number assignment */
    __complex__ double c1 = 1.0 + 2.0i;
    __complex__ double c2 = 3.0 + 4.0i;
    __complex__ double c3 = c1 * c2;  /* Complex operations use SUBREG */
    
    asm volatile("" : : "r"(lane0), "r"(int_bits), "r"(c3));
}

/* ==================== MEM_P patterns ==================== */
NOINLINE void test_mem_dest(int *base, int index, int value) {
    /* Complex memory destinations */
    int array[16];
    
    /* Store with complex addressing - MEM destination */
    array[index] = value;                     /* MEM with index */
    array[index + g_index] = value * 2;       /* More complex address */
    array[g_index * 2] = array[index];        /* Load and store */
    
    /* Pointer arithmetic store */
    int *ptr = &array[8];
    ptr[g_condition ? 1 : -1] = value;        /* Conditional offset */
    
    /* Struct member store through pointer */
    struct Point {
        int x;
        int y;
        int z;
    } points[4];
    
    points[g_index & 3].x = value;            /* Struct member store */
    points[g_index & 3].y = value + 1;
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[g_index & 3][index & 3] = value;   /* 2D array store */
    
    /* Force memory operations */
    asm volatile("" : : "m"(array), "m"(points), "m"(matrix));
}

/* ==================== Combined test function ==================== */
/* Use O2 optimization specifically for this function */
__attribute__((optimize("O2")))
NOINLINE void combined_test(int x) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        volatile unsigned bits : 4;
    } bf;
    bf.bits = x & 0xF;
    
    /* STRICT_LOW_PART via byte assignment */
    int dest = 0;
    char src_byte = x & 0xFF;
    dest = src_byte;  /* May generate STRICT_LOW_PART */
    
    /* SUBREG via vector extraction */
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf v = {1.0f, 2.0f, 3.0f, 4.0f};
    float element = v[0];  /* SUBREG extraction */
    
    /* MEM destination with addressing */
    int buffer[8];
    buffer[x & 7] = dest + (int)element;
    
    /* Loop with memory stores */
    for (int i = 0; i < 4; i++) {
        buffer[i] = buffer[i + 1] + x;
    }
    
    asm volatile("" : : "r"(bf.bits), "r"(dest), "m"(buffer));
}

/* Helper to generate varying addresses */
NOINLINE int* get_pointer(int *base, int offset) {
    return base + (offset & 7);
}

/* ==================== Main test driver ==================== */
int main(void) {
    int test_array[32];
    int i;
    
    /* Initialize with non-constant values */
    for (i = 0; i < 32; i++) {
        test_array[i] = i * 3 + 1;
    }
    
    /* Call test functions multiple times with varying inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i);
        test_subreg();
        test_mem_dest(test_array, i & 15, i * 10);
        combined_test(i);
        
        /* Additional MEM_P test with pointer helper */
        int *ptr = get_pointer(test_array, i);
        *ptr = i * 100;  /* MEM store through computed pointer */
    }
    
    /* Final validation to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < 32; i++) {
        sum += test_array[i];
    }
    
    return sum & 0xFF;  /* Non-zero return */
}
