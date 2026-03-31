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

/* Volatile bit-field to prevent optimization */
volatile struct bitfield_struct g_bf;

NOINLINE void test_zero_extract(void) {
    /* Assignment to bit-field member -> ZERO_EXTRACT destination */
    struct bitfield_struct local_bf;
    local_bf.field1 = 5;      /* Should generate ZERO_EXTRACT */
    local_bf.field2 = 31;     /* Another bit-field assignment */
    
    /* Volatile assignment ensures RTL generation */
    g_bf.field3 = g_value & 0xFF;
    
    /* Complex bit-field expression */
    unsigned int temp = g_value;
    local_bf.field1 = (temp >> 2) & 0x7;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(local_bf.field1), "r"(local_bf.field2));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE void test_strict_low_part(void) {
    /* Mixed-size integer assignments - common source of STRICT_LOW_PART */
    short src_short = 12345;
    int dest_int;
    
    /* short -> int assignment may generate STRICT_LOW_PART */
    dest_int = src_short;           /* Potential STRICT_LOW_PART destination */
    
    /* char -> long assignment */
    char src_char = 'A';
    long dest_long;
    dest_long = src_char;           /* Another potential STRICT_LOW_PART */
    
    /* 16-bit operation on 32-bit variable */
    uint32_t var32 = 0x12345678;
    uint16_t var16 = 0xABCD;
    var32 = var16;                  /* Should generate STRICT_LOW_PART */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(dest_int), "r"(dest_long), "r"(var32));
}

/* ==================== SUBREG patterns ==================== */

/* GCC vector types for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    /* Vector operations that generate SUBREG */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector assignment */
    vec_a = vec_b;                  /* May involve SUBREG in RTL */
    
    /* Extract lane from vector -> SUBREG destination */
    int lane = vec_a[g_index & 3];  /* SUBREG for lane extraction */
    
    /* Type punning through union - generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14159f;
    int int_bits = pun.i;           /* SUBREG for type conversion */
    
    /* Mixed vector operations */
    v4sf float_vec = {1.0f, 2.0f, 3.0f, 4.0f};
    v4si int_vec;
    
    /* Memory cast that might generate SUBREG */
    memcpy(&int_vec, &float_vec, sizeof(float_vec));
    
    /* Prevent dead code */
    asm volatile("" : : "r"(lane), "r"(int_bits), "r"(int_vec[0]));
}

/* ==================== MEM_P patterns ==================== */

/* Complex addressing helper */
NOINLINE int* get_pointer(int *base, int offset) {
    return base + (offset * g_condition);
}

NOINLINE void test_mem_dest(void) {
    int array[16];
    int *ptr_array = array;
    
    /* Store with complex address calculation -> MEM destination */
    array[g_index] = g_value;       /* MEM with index */
    
    /* Pointer arithmetic store */
    *(ptr_array + g_index + 2) = g_value * 2;
    
    /* Store through function-returned pointer */
    int *dynamic_ptr = get_pointer(array, g_index);
    *dynamic_ptr = g_value + 1;     /* MEM with complex address */
    
    /* Struct member store */
    struct {
        int a;
        int b;
        int c[4];
    } s;
    
    s.b = g_value;                  /* MEM to struct member */
    s.c[g_index & 3] = g_value * 3; /* MEM with array index */
    
    /* Prevent optimization */
    asm volatile("" : : "m"(array[0]), "m"(s.b));
}

/* ==================== COMBINED TEST FUNCTION ==================== */

/* Force O2 optimization on this function */
__attribute__((optimize("O2")))
NOINLINE void test_combined(void) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT via bit-field */
    struct {
        unsigned int flags : 4;
        unsigned int mode : 3;
    } settings;
    settings.flags = g_value & 0xF;
    settings.mode = (g_value >> 4) & 0x7;
    
    /* STRICT_LOW_PART via mixed-size assignment */
    int32_t big_var;
    int16_t small_var = g_value;
    big_var = small_var;            /* Potential STRICT_LOW_PART */
    
    /* SUBREG via vector operation */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    short_vec[g_index & 7] = g_value & 0xFFFF;
    
    /* MEM destination with complex address */
    int buffer[8];
    for (int i = 0; i < 8; i++) {
        buffer[i] = g_value + i;    /* MEM in loop */
    }
    
    /* Control flow to create multiple basic blocks */
    if (g_condition) {
        settings.mode = 3;
        buffer[0] = 99;
    } else {
        big_var = 0;
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(settings.flags), "r"(big_var), "m"(buffer[0]));
}

/* ==================== MAIN DRIVER ==================== */

int main(void) {
    /* Call test functions multiple times with different conditions */
    for (int i = 0; i < 10; i++) {
        g_index = i;
        g_value = i * 10;
        g_condition = i & 1;
        
        test_zero_extract();
        test_strict_low_part();
        test_subreg();
        test_mem_dest();
        test_combined();
    }
    
    return 0;
}
