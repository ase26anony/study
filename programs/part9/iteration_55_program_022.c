/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure - should generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

NOINLINE void test_zero_extract(void) {
    /* Volatile to prevent optimization */
    volatile struct bitfield_struct bfs;
    
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    bfs.field1 = 5;      /* ZERO_EXTRACT for bit-field store */
    bfs.field2 = 0x1F;   /* Another bit-field assignment */
    bfs.field3 = 0xFF;   /* 8-bit field assignment */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bfs));
}

/* Another ZERO_EXTRACT pattern using unions */
union mixed_bf {
    struct {
        unsigned int low : 10;
        unsigned int high : 22;
    } bits;
    unsigned int full;
};

NOINLINE void test_zero_extract_union(void) {
    volatile union mixed_bf u;
    u.bits.low = 0x3FF;    /* Should generate ZERO_EXTRACT */
    u.bits.high = 0x3FFFFF >> 2;
    
    /* Force usage */
    asm volatile("" : : "r"(u.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Mixed-size integer assignments - may generate STRICT_LOW_PART */
NOINLINE void test_strict_low_part(void) {
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    volatile int dest32;
    volatile long dest64;
    
    /* These assignments might generate STRICT_LOW_PART on x86 */
    dest32 = src16;      /* short -> int, preserving low 16 bits */
    dest32 = src8;       /* char -> int, preserving low 8 bits */
    
    /* 64-bit version */
    dest64 = src16;      /* short -> long */
    dest64 = src8;       /* char -> long */
    
    /* Use inline assembly that explicitly writes partial registers */
    asm volatile(
        "movw %1, %%ax\n\t"
        "movl %%eax, %0\n\t"
        : "=r"(dest32)
        : "r"(src16)
        : "%eax"
    );
    
    asm volatile("" : : "r"(dest32), "r"(dest64));
}

/* ==================== SUBREG patterns ==================== */

/* GCC vector types - often generate SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    volatile v4si vec_a = {1, 2, 3, 4};
    volatile v4si vec_b = {5, 6, 7, 8};
    volatile v8hi vec_short;
    volatile int lane;
    volatile float f;
    volatile int i;
    
    /* Vector assignment - may involve SUBREG */
    vec_a = vec_b;
    
    /* Extract lane - generates SUBREG */
    lane = vec_a[0];      /* Extracting element via SUBREG */
    lane = vec_a[g_index & 3];  /* Variable index */
    
    /* Type punning through union - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    i = pun.i;            /* float bits -> int via SUBREG */
    
    /* Vector type conversion */
    vec_short = (v8hi)vec_a;  /* Cast between vector types */
    
    asm volatile("" : : "r"(vec_a), "r"(lane), "r"(i), "r"(vec_short));
}

/* ==================== MEM_P patterns ==================== */

/* Complex memory addressing patterns */
NOINLINE void test_mem_dest(int *base, int offset) {
    volatile int array[16];
    volatile int *ptr;
    
    /* Store with complex address calculation */
    array[g_index] = g_value;                     /* MEM with index */
    array[g_index + offset] = g_value * 2;        /* More complex index */
    
    /* Pointer arithmetic store */
    ptr = &array[0];
    ptr[offset * 2] = g_value;                    /* MEM with scaled index */
    
    /* Conditional pointer selection */
    int *dest_ptr = g_condition ? &array[8] : &array[0];
    *dest_ptr = g_value;                          /* MEM with conditional address */
    
    /* Struct member store through pointer */
    struct {
        int a;
        int b;
        int c;
    } s;
    
    int *member_ptr = g_condition ? &s.b : &s.c;
    *member_ptr = g_value;                        /* MEM to struct member */
    
    asm volatile("" : : "r"(array[0]), "r"(s.a));
}

/* ==================== Combined test function ==================== */

/* Function with all patterns mixed together */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int arg) {
    /* ZERO_EXTRACT */
    volatile struct {
        unsigned int bf1 : 4;
        unsigned int bf2 : 12;
    } s;
    s.bf1 = arg & 0xF;
    s.bf2 = (arg >> 4) & 0xFFF;
    
    /* STRICT_LOW_PART */
    volatile short sval = arg & 0xFFFF;
    volatile int ival;
    ival = sval;  /* Potential STRICT_LOW_PART */
    
    /* SUBREG */
    volatile v4si vec = {arg, arg+1, arg+2, arg+3};
    volatile int elem = vec[arg & 3];
    
    /* MEM_P */
    volatile int mem_array[8];
    int idx = (arg * 7) & 7;
    mem_array[idx] = elem;  /* Store with computed index */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(s), "r"(ival), "r"(elem), "r"(mem_array[0]));
}

/* ==================== Main driver ==================== */

int main(void) {
    int test_data[] = {1, 2, 3, 4, 5};
    int array[32];
    
    /* Call all test functions multiple times with different data */
    for (int i = 0; i < 5; i++) {
        test_zero_extract();
        test_zero_extract_union();
        test_strict_low_part();
        test_subreg();
        test_mem_dest(array, i);
        test_combined(test_data[i]);
    }
    
    /* Additional calls with different optimization hints */
    __attribute__((optimize("O3"))) void (*opt_func)(int) = test_combined;
    for (int i = 0; i < 3; i++) {
        opt_func(i * 10);
    }
    
    return 0;
}
