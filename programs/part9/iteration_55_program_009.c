/* test_resource.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * resource.cc file, particularly in mark_referenced_resources() function.
 * The goal is to generate RTL with SET destinations that are:
 * 1. ZERO_EXTRACT    (via bit-field assignments)
 * 2. STRICT_LOW_PART (via partial register writes)
 * 3. SUBREG          (via vector/type conversions)
 * 4. MEM_P           (via complex memory stores)
 */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
int g_array[100];

/* ==================== ZERO_EXTRACT ==================== */
/* Bit-field assignments often generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

/* Use volatile to prevent optimization of bit-field accesses */
volatile struct bitfield_struct g_bitfield;

NOINLINE void test_zero_extract(int a, int b, int c) {
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    g_bitfield.field1 = a & 0x7;      /* 3-bit field */
    g_bitfield.field2 = b & 0x1F;     /* 5-bit field */
    g_bitfield.field3 = c & 0xFF;     /* 8-bit field */
    
    /* Mix with control flow to prevent over-optimization */
    if (a > b) {
        g_bitfield.field1 = b & 0x7;
    }
    
    /* Nested bit-field in local volatile struct */
    volatile struct bitfield_struct local_bf;
    local_bf.field2 = (a + b) & 0x1F;
    local_bf.field3 = local_bf.field2 << 2;
    
    /* Force use of local_bf to prevent dead store elimination */
    asm volatile("" : : "r"(local_bf.field3));
}

/* ==================== STRICT_LOW_PART ==================== */
/* Partial register writes (e.g., short to int) can generate STRICT_LOW_PART */
/* Compile with -m32 for better chance of STRICT_LOW_PART on x86 */
NOINLINE void test_strict_low_part(short s_val, char c_val, int i_val) {
    int dest_int;
    long dest_long;
    
    /* short -> int assignment may use STRICT_LOW_PART */
    dest_int = s_val;                 /* Potential STRICT_LOW_PART */
    
    /* char -> long assignment */
    dest_long = c_val;                /* Another potential */
    
    /* Mix with computation to prevent optimization */
    dest_int = dest_int + (i_val & 0xFFFF);
    
    /* Use inline assembly hint for partial register */
    asm volatile("# STRICT_LOW_PART hint" : "+r"(dest_int));
    
    /* Chain of partial writes */
    short temp = s_val + 1;
    dest_int = temp;                  /* Another candidate */
    
    /* Force use of results */
    g_array[0] = dest_int;
    g_array[1] = (int)dest_long;
}

/* ==================== SUBREG ==================== */
/* Type conversions and vector operations generate SUBREG */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(float f_val, int i_val) {
    /* Vector operations often involve SUBREG */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Vector assignment - may have SUBREG in RTL representation */
    vec_c = vec_a + vec_b;
    
    /* Extract element - generates SUBREG */
    int element = vec_c[g_index & 3];  /* SUBREG to extract lane */
    
    /* Type punning through union - can generate SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = f_val;
    element = pun.i;                   /* Potential SUBREG */
    
    /* Mixed-size vector conversion */
    v8hi short_vec = {1, 2, 3, 4, 5, 6, 7, 8};
    v4si int_vec;
    
    /* This cast may involve SUBREG */
    memcpy(&int_vec, &short_vec, sizeof(short_vec));
    
    /* Complex number can also generate SUBREG */
    _Complex float comp = f_val + f_val * 2.0f * I;
    float real_part = __real__ comp;   /* May use SUBREG */
    
    /* Store results to prevent elimination */
    g_array[2] = element;
    g_array[3] = (int)real_part;
    g_array[4] = vec_c[0];
}

/* ==================== MEM_P ==================== */
/* Complex memory addresses generate MEM with address expressions */
NOINLINE void test_mem_p(int *base, int offset, int value) {
    /* Array with variable index - complex address calculation */
    g_array[g_index] = value;                     /* MEM with index */
    
    /* Pointer arithmetic with multiple operations */
    int *ptr = base + offset * 2;
    ptr[g_index & 7] = value + 1;                 /* More complex MEM */
    
    /* Struct pointer with field access */
    struct data {
        int x;
        int y;
        int z;
    };
    struct data *data_ptr = (struct data *)base;
    data_ptr[offset & 3].y = value * 2;           /* MEM with struct field */
    
    /* Two-dimensional access simulation */
    int matrix[10][10];
    int i = (offset * 17) % 10;
    int j = (offset * 23) % 10;
    matrix[i][j] = value;                         /* MEM with 2D index */
    
    /* Pointer through function call */
    int *get_ptr(int idx) { return &g_array[idx * 2]; }
    *get_ptr(offset) = value - 1;                 /* MEM with function result */
    
    /* Volatile pointer dereference */
    volatile int *vol_ptr = (volatile int *)base;
    vol_ptr[5] = value;                           /* Volatile MEM */
}

/* ==================== COMBINED TEST ==================== */
/* Function that combines all patterns */
NOINLINE __attribute__((optimize("O2"))) 
void test_combined(int a, short b, float c, int *ptr) {
    /* ZERO_EXTRACT via bit-field */
    volatile struct {
        unsigned int low : 4;
        unsigned int high : 12;
    } comb_bf;
    comb_bf.low = a & 0xF;
    comb_bf.high = (a >> 4) & 0xFFF;
    
    /* STRICT_LOW_PART via partial write */
    int combined_int = b;              /* short to int */
    
    /* SUBREG via vector */
    v4sf vec = {c, c+1, c+2, c+3};
    float first = vec[0];              /* Element extraction */
    
    /* MEM_P via complex store */
    ptr[g_index % 10] = combined_int + (int)first;
    
    /* Loop to increase RTL complexity */
    for (int i = 0; i < 3; i++) {
        comb_bf.low = (comb_bf.low + i) & 0xF;
        combined_int += ptr[i];
    }
    
    /* Use results */
    asm volatile("" : : "r"(comb_bf.low), "r"(combined_int), "r"(first));
}

/* ==================== MAIN ==================== */
int main(int argc, char **argv) {
    /* Initialize with non-constant values */
    int base_val = argc > 1 ? atoi(argv[1]) : 100;
    
    /* Test each pattern multiple times with different values */
    for (int i = 0; i < 10; i++) {
        test_zero_extract(i, i*2, i*3);
        test_strict_low_part(i*10, i*5, i*20);
        test_subreg(i*1.5f, i*7);
        test_mem_p(g_array, i, base_val + i);
        test_combined(i, i*2, i*0.5f, g_array);
    }
    
    /* Verify some results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += g_array[i];
    }
    
    return sum > 0 ? 0 : 1;
}
