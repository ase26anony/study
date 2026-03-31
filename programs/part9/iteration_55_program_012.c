/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ===== ZERO_EXTRACT patterns (bit-field assignments) ===== */
struct bitfields {
    unsigned int field3 : 3;
    unsigned int field5 : 5;
    unsigned int field8 : 8;
    volatile unsigned int field10 : 10; /* volatile ensures actual extraction */
};

NOINLINE void test_zero_extract(void) {
    /* These assignments should generate ZERO_EXTRACT in SET_DEST */
    volatile struct bitfields bf;
    
    /* Multiple bit-field assignments to increase coverage probability */
    bf.field3 = 5;      /* Should generate ZERO_EXTRACT for 3-bit field */
    bf.field5 = 31;     /* Should generate ZERO_EXTRACT for 5-bit field */
    bf.field8 = 255;    /* Should generate ZERO_EXTRACT for 8-bit field */
    bf.field10 = 1023;  /* Should generate ZERO_EXTRACT for 10-bit field */
    
    /* Use the values to prevent dead code elimination */
    g_temp = bf.field3 + bf.field5;
}

/* ===== STRICT_LOW_PART patterns (partial register writes) ===== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size assignments that may generate STRICT_LOW_PART */
    int dest32;
    short src16 = x & 0xFFFF;
    char src8 = x & 0xFF;
    
    /* These assignments might generate STRICT_LOW_PART on x86 */
    dest32 = src16;     /* 16-bit to 32-bit assignment */
    dest32 = src8;      /* 8-bit to 32-bit assignment */
    
    /* Force use of the result */
    g_temp = dest32;
    
    /* Inline assembly that explicitly writes partial registers */
    asm volatile (
        "movw %w1, %0\n\t"  /* Write 16-bit (word) to 32/64-bit register */
        : "=r" (dest32)
        : "r" (src16)
    );
    
    /* Another partial write pattern */
    int32_t val32 = 0x12345678;
    int16_t val16 = 0xABCD;
    val32 = val16;  /* This might use STRICT_LOW_PART */
    
    g_temp = val32;
}

/* ===== SUBREG patterns (register subset operations) ===== */
typedef int v4si __attribute__((vector_size(16)));  /* 4x int vector */
typedef float v4sf __attribute__((vector_size(16))); /* 4x float vector */

NOINLINE void test_subreg(void) {
    /* Vector operations that generate SUBREG */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector assignment - may involve SUBREG for lane extraction */
    int lane0 = vec_a[0];  /* Lane extraction */
    int lane1 = vec_b[1];
    
    /* Type punning through union - often uses SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    int int_bits = pun.i;  /* This generates SUBREG */
    
    /* Mixed vector operations */
    v4sf fvec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4si ivec;
    memcpy(&ivec, &fvec, sizeof(fvec));  /* Type conversion via memcpy */
    
    g_temp = lane0 + lane1 + int_bits + ivec[0];
}

/* ===== MEM_P patterns (complex memory destinations) ===== */
NOINLINE void test_mem_dest(int index, int value) {
    int array[16];
    int *ptr;
    
    /* Complex addressing modes for MEM destinations */
    array[index] = value;                     /* Indexed store */
    array[index + 1] = value * 2;             /* More complex index */
    array[g_index] = value + g_value;         /* Volatile global in address */
    
    /* Pointer arithmetic with stores */
    ptr = &array[0];
    ptr += index;
    *ptr = value;                             /* Pointer-based store */
    
    *(ptr + 2) = value * 3;                   /* Offset pointer store */
    
    /* Struct with array member */
    struct {
        int data[8];
        int count;
    } s;
    
    s.data[index % 8] = value;                /* Struct member array store */
    s.count = index;
    
    /* Multi-dimensional array */
    int matrix[4][4];
    matrix[index % 4][(index + 1) % 4] = value;
    
    g_temp = array[0] + s.data[0] + matrix[0][0];
}

/* ===== COMBINED TEST FUNCTION ===== */
/* Use O2 optimization specifically for this function */
__attribute__((optimize("O2")))
NOINLINE void test_combined(void) {
    /* Mix all patterns in one function */
    volatile struct bitfields bf;
    bf.field5 = 20;                    /* ZERO_EXTRACT */
    
    short s = 1000;
    int i = s;                         /* Possible STRICT_LOW_PART */
    
    v4si vec = {0};
    int lane = vec[2];                 /* SUBREG */
    
    int arr[10];
    arr[g_index] = i;                  /* MEM_P with complex address */
    
    /* Force all values to be used */
    g_temp = bf.field5 + i + lane + arr[0];
}

/* Helper to get variable pointer for complex addressing */
NOINLINE int* get_pointer(int *base, int offset) {
    return base + (offset * 2);  /* Non-trivial address calculation */
}

/* Main driver that calls all test functions */
int main(void) {
    int i;
    
    /* Call test functions multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i);
        test_subreg();
        test_mem_dest(i % 8, i * 10);
        test_combined();
        
        /* Additional calls with pointer arithmetic */
        int buffer[20];
        int *ptr = get_pointer(buffer, i);
        *ptr = i;  /* MEM store with computed address */
    }
    
    return g_temp != 0 ? 0 : 1;
}
