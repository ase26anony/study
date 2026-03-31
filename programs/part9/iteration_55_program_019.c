/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp = 0;

/* Structure with bit-fields for ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
};

/* Vector types for SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Function 1: Focus on ZERO_EXTRACT and MEM_P */
NOINLINE __attribute__((optimize("O2")))
void test_zero_extract_mem(int idx, int val) {
    struct bitfield_struct s = {0};
    int array[16];
    
    /* This should generate ZERO_EXTRACT in SET_DEST */
    s.field1 = val & 0x7;          /* ZERO_EXTRACT destination */
    s.field2 = (val >> 3) & 0x1F;  /* Another ZERO_EXTRACT */
    
    /* Complex memory store - MEM_P destination */
    array[idx * 2 + 1] = val;      /* MEM with complex address */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(s.field1), "r"(array[idx]));
}

/* Function 2: Focus on STRICT_LOW_PART and SUBREG */
NOINLINE __attribute__((optimize("O1")))
void test_strict_low_part_subreg(short src_short, char src_char) {
    int dest_int = 0;
    long dest_long = 0;
    v4si vec_a, vec_b;
    int lane;
    
    /* These may generate STRICT_LOW_PART on x86 */
    dest_int = src_short;          /* Possible STRICT_LOW_PART */
    dest_long = src_char;          /* Another possible STRICT_LOW_PART */
    
    /* Vector operations for SUBREG */
    vec_a = (v4si){1, 2, 3, 4};
    vec_b = (v4si){5, 6, 7, 8};
    
    /* Vector lane extract - generates SUBREG */
    lane = vec_a[2];               /* SUBREG extraction */
    
    /* Type punning through union for SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    dest_int = pun.i;              /* Possible SUBREG through memory */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(dest_int), "r"(dest_long), "r"(lane));
}

/* Function 3: Mixed patterns with control flow */
NOINLINE __attribute__((optimize("O3")))
int test_mixed_patterns(int *ptr, int count) {
    volatile struct bitfield_struct bs = {0};
    v8hi vec_short;
    int result = 0;
    
    for (int i = 0; i < count && i < 8; i++) {
        if (i & 1) {
            /* ZERO_EXTRACT pattern */
            bs.field3 = ptr[i] & 0xFF;
            
            /* MEM_P with pointer arithmetic */
            *(ptr + i + 1) = bs.field3;
        } else {
            /* SUBREG through vector operations */
            vec_short = (v8hi){0};
            vec_short[i] = ptr[i] & 0xFFFF;  /* Possible STRICT_LOW_PART */
            
            /* Extract through SUBREG */
            result += vec_short[i];
        }
    }
    
    /* Complex memory address calculation */
    int *complex_ptr = ptr + (count & 3);
    *complex_ptr = result;          /* MEM_P with computed address */
    
    return result;
}

/* Function 4: Emphasize STRICT_LOW_PART for 32-bit x86 */
NOINLINE __attribute__((optimize("O1")))
void test_partial_registers(void) {
    /* These operations are more likely to generate STRICT_LOW_PART on x86 */
    volatile short s16;
    volatile char c8;
    volatile int i32;
    volatile long long ll64;
    
    s16 = g_value;
    c8 = g_value;
    
    /* Multiple partial register writes */
    i32 = s16;                     /* Likely STRICT_LOW_PART on x86-32 */
    i32 = c8;                      /* Another possible STRICT_LOW_PART */
    
    /* Mixed-size operations */
    ll64 = i32;                    /* Sign extension may use SUBREG */
    i32 = ll64;                    /* Truncation may use SUBREG */
    
    /* Use asm to force register allocation */
    asm volatile("" : : "r"(i32), "r"(ll64));
}

/* Helper to create complex address expressions */
NOINLINE int* get_complex_ptr(int *base, int offset) {
    return base + (offset * 2) + (g_index & 3);
}

/* Main driver that calls all test functions */
int main(void) {
    int data[32];
    int i;
    
    /* Initialize data */
    for (i = 0; i < 32; i++) {
        data[i] = i * 3 + 1;
    }
    
    /* Test 1: ZERO_EXTRACT and MEM_P */
    for (i = 0; i < 8; i++) {
        test_zero_extract_mem(i, data[i]);
    }
    
    /* Test 2: STRICT_LOW_PART and SUBREG */
    test_strict_low_part_subreg(1000, 50);
    test_strict_low_part_subreg(-200, 100);
    
    /* Test 3: Mixed patterns */
    int *ptr = get_complex_ptr(data, 4);
    int res = test_mixed_patterns(ptr, 10);
    
    /* Test 4: Partial register emphasis */
    test_partial_registers();
    
    /* Additional MEM_P tests with complex addresses */
    for (i = 0; i < 5; i++) {
        int *addr = get_complex_ptr(data, i);
        *addr = res + i;            /* MEM_P with function-call address */
    }
    
    /* Ensure everything is used */
    g_temp = res + data[0];
    
    return g_temp > 0 ? 0 : 1;
}
