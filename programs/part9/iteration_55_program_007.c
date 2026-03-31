/* test_resource.c - Coverage test for GCC's resource.cc mark_referenced_resources */
/* Compile with: gcc -O2 -fprofile-arcs -ftest-coverage -fdump-rtl-all -c test_resource.c */
/* Also try: gcc -m32 -O1 -fno-omit-frame-pointer -fprofile-arcs -ftest-coverage test_resource.c -o test */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile char g_char = 'A';
volatile short g_short = 1234;

/* Structure with bit-fields to generate ZERO_EXTRACT */
struct bitfield_struct {
    volatile unsigned int field1 : 3;
    volatile unsigned int field2 : 5;
    volatile unsigned int field3 : 8;
};

/* Vector types to generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Function 1: Focus on ZERO_EXTRACT and MEM_P */
NOINLINE void test_zero_extract_and_mem(int *arr, int n) {
    struct bitfield_struct bfs = {0};
    
    /* This should generate ZERO_EXTRACT as SET_DEST */
    bfs.field1 = g_value & 0x7;          /* ZERO_EXTRACT destination */
    bfs.field2 = (g_value >> 3) & 0x1F;  /* Another ZERO_EXTRACT */
    
    /* Complex memory store - MEM_P destination with non-trivial address */
    int idx = g_index * 2 + 1;
    arr[idx] = g_value;                  /* MEM destination */
    
    /* Nested array access */
    int *ptr = &arr[n % 8];
    *ptr = bfs.field1;                   /* Another MEM destination */
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(bfs.field1), "r"(bfs.field2));
}

/* Function 2: Focus on STRICT_LOW_PART and SUBREG */
NOINLINE void test_strict_low_part_and_subreg(int x, long y) {
    volatile short s_val;
    volatile char c_val;
    volatile int i_val;
    
    /* Mixed-size assignments that may generate STRICT_LOW_PART */
    s_val = g_short;
    i_val = s_val;                       /* Possible STRICT_LOW_PART destination */
    
    c_val = g_char;
    i_val = c_val;                       /* Another possible STRICT_LOW_PART */
    
    /* Vector operations for SUBREG */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    vec_c = vec_a + vec_b;               /* Vector operation */
    
    /* Extract element - may generate SUBREG as destination */
    int lane = vec_c[g_index & 3];       /* SUBREG destination */
    
    /* Type punning through union for SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    i_val = pun.i;                       /* Possible SUBREG destination */
    
    /* Prevent optimization */
    asm volatile("" : : "r"(s_val), "r"(i_val), "r"(lane), "r"(vec_c));
}

/* Function 3: Mixed patterns with control flow */
NOINLINE int test_mixed_patterns(int *arr, int flag) {
    volatile int result = 0;
    struct bitfield_struct bfs = {0};
    
    for (int i = 0; i < 4; i++) {
        if (flag & (1 << i)) {
            /* Bit-field assignment - ZERO_EXTRACT */
            bfs.field3 = (g_value + i) & 0xFF;
            
            /* Memory store with complex addressing - MEM_P */
            arr[i * 2] = bfs.field3;
            
            /* Mixed-size assignment - STRICT_LOW_PART */
            short temp = (short)(g_value + i);
            result = temp;               /* Possible STRICT_LOW_PART destination */
        } else {
            /* Different memory pattern */
            int idx = (i + g_index) % 8;
            arr[idx] = result;           /* MEM destination */
        }
    }
    
    /* Vector extract - SUBREG */
    v8hi v_short = {1, 2, 3, 4, 5, 6, 7, 8};
    short s_elem = v_short[g_index & 7]; /* SUBREG destination */
    result += s_elem;
    
    return result;
}

/* Function 4: Emphasize STRICT_LOW_PART on 32-bit architectures */
__attribute__((optimize("O1"))) 
NOINLINE void test_partial_registers(void) {
    volatile int i;
    volatile short s;
    volatile char c;
    
    /* Multiple partial register writes */
    s = 0xABCD;
    i = s;                               /* Likely STRICT_LOW_PART on x86-32 */
    
    c = 0xEF;
    i = c;                               /* Another STRICT_LOW_PART candidate */
    
    /* Chain of assignments */
    short s2 = i & 0xFFFF;
    int i2 = s2;                         /* Another candidate */
    
    /* Prevent elimination */
    asm volatile("" : : "r"(i), "r"(s), "r"(i2));
}

/* Helper to create complex address expressions */
NOINLINE int* get_complex_address(int *base, int offset) {
    /* Complex address calculation */
    return base + (offset * 3) / 2 + (offset % 4);
}

/* Main driver */
int main(void) {
    int array[32] = {0};
    int i;
    
    /* Initialize array with non-zero values */
    for (i = 0; i < 32; i++) {
        array[i] = i * 3;
    }
    
    /* Call test functions multiple times with different arguments */
    for (i = 0; i < 8; i++) {
        test_zero_extract_and_mem(array, i);
        test_strict_low_part_and_subreg(i, i * 100L);
        test_mixed_patterns(array, i);
        test_partial_registers();
        
        /* Test with complex address */
        int *ptr = get_complex_address(array, i);
        *ptr = i * 2;                    /* MEM destination */
    }
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < 32; i++) {
        sum += array[i];
    }
    
    return sum > 0 ? 0 : 1;
}
