/* test_resource.c - Cover specific RTL patterns in GCC's resource.cc */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ========== ZERO_EXTRACT patterns ========== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment should generate ZERO_EXTRACT in RTL */
    struct bitfield_s {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    };
    volatile struct bitfield_s bf;
    
    /* Multiple bit-field assignments */
    bf.field1 = 5;      /* Should generate ZERO_EXTRACT for 3-bit field */
    bf.field2 = 31;     /* Should generate ZERO_EXTRACT for 5-bit field */
    bf.field3 = 255;    /* Should generate ZERO_EXTRACT for 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf));
}

/* ========== STRICT_LOW_PART patterns ========== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size integer assignments on 32-bit x86 often use STRICT_LOW_PART */
    short src16 = x & 0xFFFF;
    char src8 = x & 0xFF;
    int dest32;
    
    /* These assignments may generate STRICT_LOW_PART for partial register writes */
    dest32 = src16;     /* 16-bit to 32-bit assignment */
    g_temp = dest32;
    
    dest32 = src8;      /* 8-bit to 32-bit assignment */
    g_temp = dest32;
    
    /* Pointer casting with smaller types */
    int32_t val32 = x;
    int16_t *ptr16 = (int16_t*)&val32;
    *ptr16 = 0x1234;    /* Partial write to 32-bit variable */
    
    asm volatile("" : : "r"(val32));
}

/* ========== SUBREG patterns ========== */
NOINLINE void test_subreg(float f, double d) {
    /* Type punning through unions often generates SUBREG */
    union {
        float f;
        uint32_t i;
    } u;
    
    u.f = f;
    u.i = u.i ^ 0x80000000;  /* Flip sign bit via integer operation */
    
    /* GCC vector extensions for SUBREG generation */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v8hi vec_b;
    
    /* Vector lane extraction generates SUBREG */
    int lane = vec_a[g_index & 3];
    g_temp = lane;
    
    /* Vector conversion through memory */
    memcpy(&vec_b, &vec_a, sizeof(vec_b));
    asm volatile("" : : "r"(vec_b));
    
    /* Double to float conversion */
    float f2 = (float)d;
    asm volatile("" : : "r"(f2));
}

/* ========== MEM_P with complex addressing ========== */
NOINLINE void test_mem_complex(int *base, int offset) {
    /* Complex memory addressing modes */
    int array[16];
    
    /* Store with index computation */
    array[g_index] = g_value;
    
    /* Store with pointer arithmetic */
    *(base + offset) = g_value;
    
    /* Store with conditional addressing */
    int *ptr = (offset > 0) ? &array[5] : &array[10];
    *ptr = g_value;
    
    /* Nested structure with pointer */
    struct nested {
        int a[4];
        struct {
            int x;
            int y;
        } inner;
    } nst;
    
    nst.inner.x = g_value;
    nst.a[g_index & 3] = offset;
    
    asm volatile("" : : "r"(array[0]), "r"(nst.inner.x));
}

/* ========== Combined test with all patterns ========== */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int x, float f, int *ptr) {
    /* Bit-field (ZERO_EXTRACT) */
    struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } s;
    s.bf1 = x & 0xF;
    s.bf2 = x & 0xFFF;
    
    /* Partial register write (STRICT_LOW_PART) */
    short sval = x;
    int ival = sval;
    
    /* Type conversion (SUBREG) */
    float fval = f;
    int ival_from_float = *(int*)&fval;
    
    /* Memory store with addressing (MEM_P) */
    ptr[g_index] = ival;
    
    /* Vector operation (more SUBREG) */
    typedef int v2si __attribute__((vector_size(8)));
    v2si v1 = {x, x + 1};
    v2si v2 = {x * 2, x * 3};
    v2si v3 = v1 + v2;
    
    g_temp = s.bf1 + ival + ival_from_float + ptr[0] + v3[0];
}

/* ========== Main driver ========== */
int main(void) {
    int array[32] = {0};
    float fvalues[] = {1.0f, 2.5f, 3.14f};
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i);
        test_subreg(fvalues[i % 3], (double)i);
        test_mem_complex(array, i);
        test_combined(i, fvalues[i % 3], array);
        
        /* Modify globals to vary behavior */
        g_index = (g_index * 13 + 7) & 31;
        g_value = (g_value * 3 + 1) & 255;
    }
    
    return g_temp != 0;
}
