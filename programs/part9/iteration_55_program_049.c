/* test_resource.c - Cover specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 0;
volatile int g_value = 42;
volatile char g_char = 'A';

/* ===== ZERO_EXTRACT patterns (bit-field assignments) ===== */

/* Struct with bit-fields - volatile to prevent optimization */
struct bitfield_struct {
    volatile unsigned int field3 : 3;
    volatile unsigned int field5 : 5;
    volatile unsigned int field8 : 8;
};

NOINLINE void test_zero_extract(void) {
    struct bitfield_struct s = {0};
    volatile struct bitfield_struct *ps = &s;
    
    /* These assignments should generate ZERO_EXTRACT in RTL */
    s.field3 = 5;           /* ZERO_EXTRACT for 3-bit field */
    s.field5 = 31;          /* ZERO_EXTRACT for 5-bit field */
    ps->field8 = 255;       /* ZERO_EXTRACT through pointer */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(s.field3), "r"(s.field5), "r"(ps->field8));
}

/* ===== STRICT_LOW_PART patterns (partial register writes) ===== */

NOINLINE void test_strict_low_part(void) {
    volatile short src_short = -12345;
    volatile char src_char = 100;
    volatile int dest_int = 0;
    volatile long dest_long = 0;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest_int = src_short;   /* 16-bit to 32-bit - potential STRICT_LOW_PART */
    dest_long = src_char;   /* 8-bit to 64-bit - potential STRICT_LOW_PART */
    
    /* Mixed operations to force partial register updates */
    dest_int = (dest_int & 0xFFFF0000) | src_short;
    
    /* Use inline asm to force partial register constraints */
    asm volatile (
        "movw %1, %%ax\n\t"
        "movl %%eax, %0"
        : "=r"(dest_int)
        : "r"(src_short)
        : "%eax"
    );
    
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* ===== SUBREG patterns (type conversions/extractions) ===== */

/* Vector types for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile int lane_int;
    volatile float lane_float;
    
    /* Vector lane extraction - generates SUBREG */
    lane_int = vec_int[0];      /* SUBREG for vector element access */
    lane_float = vec_float[2];  /* SUBREG for vector element access */
    
    /* Type punning through unions - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    lane_int = pun.i;  /* SUBREG for type punning */
    
    /* Complex number operations */
    _Complex float c1 = 1.0f + 2.0fi;
    _Complex float c2 = 3.0f + 4.0fi;
    _Complex float c3 = c1 + c2;  /* May involve SUBREG operations */
    
    asm volatile("" : : "r"(lane_int), "r"(lane_float), "r"(c3));
}

/* ===== MEM_P patterns (complex memory addresses) ===== */

struct nested {
    int data[4];
    struct nested *next;
};

NOINLINE void test_mem_p(int idx) {
    volatile int array[100];
    volatile struct nested ns[10];
    volatile int *ptr;
    
    /* Complex array indexing - generates MEM with address expression */
    array[idx * 2 + 1] = g_value;           /* MEM with arithmetic address */
    array[g_index % 50] = idx;              /* MEM with global variable index */
    
    /* Struct member access through pointer arithmetic */
    ptr = &array[0];
    ptr[idx + 10] = g_value * 2;            /* MEM with computed pointer */
    
    /* Nested struct access */
    ns[idx % 5].data[(idx + 1) % 4] = g_value;
    
    /* Pointer chasing */
    volatile struct nested *current = &ns[0];
    for (int i = 0; i < 3 && i < idx; i++) {
        current->data[i] = i * 100;
    }
    
    asm volatile("" : : "r"(array[0]), "r"(ns[0].data[0]), "r"(ptr[0]));
}

/* ===== Combined test function with all patterns ===== */

NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int iterations) {
    struct bitfield_struct bs = {0};
    volatile int buffer[50];
    volatile short short_buffer[50];
    
    for (int i = 0; i < iterations && i < 10; i++) {
        /* ZERO_EXTRACT: Bit-field assignment */
        bs.field3 = i & 0x7;
        bs.field8 = (i * 37) & 0xFF;
        
        /* STRICT_LOW_PART: Partial register write */
        short_buffer[i] = i * 100;
        buffer[i] = short_buffer[i];  /* short to int conversion */
        
        /* MEM_P: Complex memory store */
        buffer[(i * 7) % 50] = bs.field8 + g_value;
        
        /* SUBREG: Type conversion */
        float temp_float = (float)i / 2.0f;
        int temp_int;
        memcpy(&temp_int, &temp_float, sizeof(int));  /* Bit-cast via memcpy */
        buffer[i + 10] = temp_int;
    }
    
    /* Force usage of all variables */
    asm volatile("" : : 
        "r"(bs.field3), 
        "r"(buffer[0]), 
        "r"(short_buffer[0]),
        "r"(buffer[10])
    );
}

/* ===== Main driver ===== */

int main(void) {
    /* Test each pattern individually */
    test_zero_extract();
    test_strict_low_part();
    test_subreg();
    
    /* Test with different indices to vary address computations */
    for (int i = 0; i < 5; i++) {
        test_mem_p(i);
    }
    
    /* Combined test with optimization */
    test_combined(8);
    
    /* Additional test with different optimization hints */
    __attribute__((optimize("O3"))) void test_optimized(void) {
        volatile struct { unsigned f:4; } bf = {0};
        bf.f = 15;
        volatile int arr[4] = {0};
        arr[g_index & 3] = bf.f;
        asm volatile("" : : "r"(bf.f), "r"(arr[0]));
    }
    
    test_optimized();
    
    return 0;
}
