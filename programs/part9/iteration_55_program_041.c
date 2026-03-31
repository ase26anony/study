/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure - assignments generate ZERO_EXTRACT */
struct bitfields {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

NOINLINE void test_zero_extract(void) {
    volatile struct bitfields bf;
    
    /* These assignments should generate ZERO_EXTRACT in SET_DEST */
    bf.field1 = 5;          /* ZERO_EXTRACT for 3-bit field */
    bf.field2 = 0x1F;       /* ZERO_EXTRACT for 5-bit field */
    bf.field3 = g_value;    /* ZERO_EXTRACT with non-constant value */
    bf.field4 = bf.field3;  /* ZERO_EXTRACT from another bit-field */
    
    /* Force use of bit-fields */
    asm volatile("" : : "r"(bf));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE void test_strict_low_part(void) {
    volatile short src16 = 0x1234;
    volatile char src8 = 0x56;
    volatile int dest32;
    volatile long dest64;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest32 = src16;         /* short -> int, preserving low 16 bits */
    dest32 = src8;          /* char -> int, preserving low 8 bits */
    
    /* Mixed-size operations */
    dest32 = dest32 + src16;  /* Operation with mixed sizes */
    
    /* Use inline assembly hint for partial register */
    asm volatile("movw %1, %%ax\n\t"
                 "movl %%eax, %0"
                 : "=r"(dest32)
                 : "r"(src16)
                 : "%eax");
    
    /* Force use of variables */
    asm volatile("" : : "r"(dest32), "r"(dest64));
}

/* ==================== SUBREG patterns ==================== */

/* Vector types for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    v8hi vec_short = {1, 2, 3, 4, 5, 6, 7, 8};
    
    volatile int lane;
    volatile float flane;
    
    /* Vector lane extraction - generates SUBREG */
    lane = vec_int[g_index & 3];      /* SUBREG for vector element */
    flane = vec_float[g_index & 3];   /* SUBREG for float vector */
    
    /* Type punning through union - may generate SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = flane;
    lane = pun.i;  /* SUBREG for type conversion */
    
    /* Vector conversion */
    vec_short = __builtin_convertvector(vec_int, v8hi);  /* SUBREG in conversion */
    
    /* Force use */
    asm volatile("" : : "r"(lane), "r"(flane), "r"(vec_short));
}

/* ==================== MEM_P patterns ==================== */

NOINLINE void test_mem_dest(void) {
    int array[16];
    int *ptr;
    
    /* Complex address calculation for MEM destination */
    ptr = &array[g_index];
    
    /* Store with complex address - generates MEM in SET_DEST */
    ptr[0] = g_value;                     /* MEM with pointer */
    ptr[g_index & 7] = g_value + 1;       /* MEM with indexed address */
    
    /* Struct with pointer member */
    struct {
        int data[8];
        int *extra;
    } s;
    s.extra = &array[8];
    
    /* Nested MEM access */
    s.extra[g_index & 3] = g_value * 2;   /* MEM through struct pointer */
    
    /* Pointer arithmetic */
    int *p = array + (g_index * 2);
    *p = g_value - 1;                     /* MEM with computed address */
    
    /* Force memory operations */
    asm volatile("" : : "m"(array), "m"(s));
}

/* ==================== Combined test function ==================== */

/* Function with optimization attribute to force specific RTL generation */
__attribute__((optimize("O2")))
NOINLINE void combined_test(int seed) {
    volatile struct bitfields bf_combined;
    volatile int temp_int;
    volatile short temp_short;
    int local_array[10];
    
    /* Mix of all patterns in one function */
    
    /* ZERO_EXTRACT */
    bf_combined.field2 = seed & 0x1F;
    
    /* STRICT_LOW_PART (on x86 with -m32) */
    temp_short = seed & 0xFFFF;
    temp_int = temp_short;
    
    /* SUBREG */
    v4si vec = {seed, seed+1, seed+2, seed+3};
    temp_int = vec[seed & 3];
    
    /* MEM_P with complex address */
    local_array[(seed * 7) % 10] = temp_int;
    
    /* Another MEM with pointer chain */
    int *p = &local_array[5];
    p[(seed & 1) ? 1 : -1] = temp_int * 2;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bf_combined), "r"(temp_int), "m"(local_array));
}

/* ==================== Main driver ==================== */

int main(void) {
    int i;
    
    /* Call test functions multiple times with different values */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_dest();
        combined_test(i);
        
        /* Modify globals to vary behavior */
        g_index = (g_index * 13 + 7) & 15;
        g_value = g_value + i;
    }
    
    return 0;
}
