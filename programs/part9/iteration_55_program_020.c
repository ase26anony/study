/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_condition = 1;

/* ===== ZERO_EXTRACT patterns ===== */
NOINLINE void test_zero_extract(void) {
    /* Bit-field assignment should generate ZERO_EXTRACT in RTL */
    struct bitfield {
        unsigned int field1 : 3;
        unsigned int field2 : 5;
        unsigned int field3 : 8;
    };
    volatile struct bitfield bf;
    
    /* Multiple bit-field assignments */
    bf.field1 = 5;      /* Should generate ZERO_EXTRACT for 3-bit field */
    bf.field2 = 20;     /* Should generate ZERO_EXTRACT for 5-bit field */
    bf.field3 = 100;    /* Should generate ZERO_EXTRACT for 8-bit field */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf));
}

/* ===== STRICT_LOW_PART patterns ===== */
NOINLINE void test_strict_low_part(int x) {
    /* Mixed-size integer assignments on 32-bit x86 */
    short src_short = x & 0xFFFF;
    char src_char = x & 0xFF;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    int dest_int = src_short;      /* short -> int */
    long dest_long = src_char;     /* char -> long */
    
    /* Force use of partial registers */
    dest_int = dest_int + (src_short & 0xFF);
    dest_long = dest_long | (src_char << 8);
    
    /* Prevent optimization */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* ===== SUBREG patterns ===== */
NOINLINE void test_subreg(void) {
    /* GCC vector types often generate SUBREG */
    typedef int v4si __attribute__((vector_size(16)));
    typedef short v8hi __attribute__((vector_size(16)));
    
    v4si vec_a = {1, 2, 3, 4};
    v8hi vec_b = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Vector element extraction - may use SUBREG */
    int elem = vec_a[g_index & 3];
    short selem = vec_b[g_index & 7];
    
    /* Type punning through union - often generates SUBREG */
    union pun {
        float f;
        int i;
    } u;
    u.f = 3.14159f;
    int int_bits = u.i;  /* float bits -> int via SUBREG */
    
    /* Mixed vector operations */
    vec_a = vec_a + (v4si){elem, elem, elem, elem};
    vec_b = vec_b + (v8hi){selem, selem, selem, selem, selem, selem, selem, selem};
    
    asm volatile("" : : "r"(vec_a), "r"(vec_b), "r"(int_bits));
}

/* ===== MEM_P patterns ===== */
NOINLINE void test_mem_dest(int *arr, int size, int idx) {
    /* Complex memory destinations */
    if (idx >= 0 && idx < size) {
        /* Store with complex address calculation */
        arr[idx * 2 + 1] = g_value;           /* MEM with index calculation */
        arr[(idx ^ 0x0F) & (size-1)] = idx;   /* More complex address */
    }
    
    /* Pointer arithmetic with multiple bases */
    int *ptr = arr + (idx % 8);
    for (int i = 0; i < 4; i++) {
        ptr[i] = ptr[i] + g_value;  /* MEM destination in loop */
    }
    
    /* Struct member store through pointer */
    struct data {
        int a;
        int b;
        int c;
    };
    struct data d;
    struct data *dptr = &d;
    dptr->b = g_value;  /* MEM with field offset */
    
    asm volatile("" : : "r"(arr), "r"(dptr));
}

/* ===== Combined test with all patterns ===== */
__attribute__((optimize("O2")))
NOINLINE void combined_test(int x) {
    /* Bit-field (ZERO_EXTRACT) */
    struct {
        unsigned int flags : 4;
        unsigned int mode : 3;
    } cfg;
    volatile struct { unsigned int status : 2; } status_reg;
    
    cfg.flags = x & 0xF;
    cfg.mode = (x >> 4) & 0x7;
    status_reg.status = x & 0x3;
    
    /* STRICT_LOW_PART patterns */
    unsigned char byte_val = x & 0xFF;
    unsigned short word_val = x & 0xFFFF;
    unsigned int dword;
    
    dword = byte_val;      /* Possible STRICT_LOW_PART */
    dword = word_val;      /* Possible STRICT_LOW_PART */
    
    /* SUBREG patterns with vectors */
    typedef float v4sf __attribute__((vector_size(16)));
    v4sf v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf v3 = v1 + v2;
    
    /* Extract element - SUBREG */
    float elem = v3[g_index & 3];
    
    /* MEM destinations */
    int buffer[16];
    for (int i = 0; i < 16; i++) {
        /* Complex addressing */
        buffer[(i + x) & 15] = i * elem;  /* MEM destination */
    }
    
    /* Prevent optimization */
    asm volatile("" : : "r"(cfg), "r"(dword), "r"(v3), "r"(buffer));
}

/* Helper to create complex address expressions */
NOINLINE int* get_pointer(int *base, int offset) {
    return base + (offset * 2) + (g_condition ? 1 : -1);
}

/* Main test driver */
int main(void) {
    int test_array[32];
    int i;
    
    /* Initialize array */
    for (i = 0; i < 32; i++) {
        test_array[i] = i;
    }
    
    /* Run all tests multiple times with different inputs */
    for (i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i * 7);
        test_subreg();
        test_mem_dest(test_array, 32, i);
        combined_test(i * 13);
        
        /* Test with pointer from helper */
        int *ptr = get_pointer(test_array, i);
        *ptr = i * 100;  /* MEM destination with complex address */
    }
    
    /* Final check to prevent dead code elimination */
    volatile int sum = 0;
    for (i = 0; i < 32; i++) {
        sum += test_array[i];
    }
    
    return sum > 0 ? 0 : 1;
}
