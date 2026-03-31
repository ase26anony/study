/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
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

NOINLINE void test_zero_extract(void) {
    volatile struct bitfield_struct bf;
    
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    bf.field1 = 5;          /* ZERO_EXTRACT: 3-bit field */
    bf.field2 = 31;         /* ZERO_EXTRACT: 5-bit field */
    bf.field3 = 255;        /* ZERO_EXTRACT: 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf.field1), "r"(bf.field2), "r"(bf.field3));
}

/* ===== STRICT_LOW_PART patterns ===== */
/* Partial register writes, especially on 32-bit x86 */
NOINLINE int test_strict_low_part(short src_short, char src_char) {
    int dest_int = 0;
    long dest_long = 0;
    
    /* These assignments may generate STRICT_LOW_PART when optimized */
    dest_int = src_short;   /* Possible STRICT_LOW_PART for 16->32 bit */
    dest_long = src_char;   /* Possible STRICT_LOW_PART for 8->64 bit */
    
    /* Mixed operations to encourage partial register usage */
    dest_int = (dest_int & 0xFFFF0000) | src_short;
    
    return dest_int + dest_long;
}

/* ===== SUBREG patterns ===== */
/* Vector types and type punning generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    int temp_int;
    float temp_float;
    
    /* Vector element extraction - often uses SUBREG */
    temp_int = vec_int[g_index & 3];    /* SUBREG for vector lane access */
    
    /* Type punning through union - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    temp_int = pun.i;                   /* SUBREG for type conversion */
    
    /* Vector store to scalar */
    vec_int[0] = temp_int;              /* SUBREG as SET_DEST */
    
    /* Use results */
    asm volatile("" : : "r"(temp_int), "r"(vec_int), "r"(vec_float));
}

/* ===== MEM_P patterns ===== */
/* Complex memory addresses */
NOINLINE void test_mem_p(int *base, int offset, int value) {
    int array[16];
    int *ptr;
    
    /* Complex addressing modes */
    base[offset] = value;               /* MEM with index */
    
    /* Pointer arithmetic */
    ptr = base + (offset * 2);
    *ptr = value;                       /* MEM with scaled index */
    
    /* Conditional address calculation */
    if (g_condition) {
        ptr = &array[g_index];
    } else {
        ptr = &array[offset];
    }
    *ptr = value;                       /* MEM with complex address */
    
    /* Multi-dimensional access */
    int matrix[4][4];
    matrix[g_index & 3][offset & 3] = value;  /* MEM with multiple indices */
}

/* ===== Combined test function ===== */
/* Function with multiple patterns to increase coverage chance */
NOINLINE void combined_test(int *mem_base) {
    /* ZERO_EXTRACT pattern */
    struct {
        volatile unsigned int flags : 4;
        volatile unsigned int mode : 2;
    } settings;
    settings.flags = 0xA;
    settings.mode = 0x3;
    
    /* STRICT_LOW_PART pattern */
    short src = 0x1234;
    int dest = 0;
    dest = src;                         /* Potential STRICT_LOW_PART */
    
    /* SUBREG pattern with vectors */
    typedef char v16qi __attribute__((vector_size(16)));
    v16qi v1, v2;
    memset(&v1, 0x11, sizeof(v1));
    memset(&v2, 0x22, sizeof(v2));
    char lane = v1[g_index & 15];       /* SUBREG access */
    
    /* MEM_P pattern with complex address */
    int *ptr = mem_base + (g_index * 2);
    *ptr = dest + lane;                 /* MEM store */
    
    /* Use all results */
    asm volatile("" : : "r"(settings.flags), "r"(dest), "r"(lane), "r"(*ptr));
}

/* ===== Main driver ===== */
int main(void) {
    int data_buffer[64];
    int i;
    
    /* Initialize buffer */
    for (i = 0; i < 64; i++) {
        data_buffer[i] = i;
    }
    
    /* Run all tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i, i * 2);
        test_subreg();
        test_mem_p(data_buffer, i & 15, i * 100);
        combined_test(data_buffer);
        
        /* Modify globals to vary behavior */
        g_index = (g_index * 13 + 7) & 31;
        g_condition = !g_condition;
    }
    
    /* Final validation */
    int sum = 0;
    for (i = 0; i < 64; i++) {
        sum += data_buffer[i];
    }
    
    return sum & 0xFF;  /* Non-zero exit to indicate execution */
}
