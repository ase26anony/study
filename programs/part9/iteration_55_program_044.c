/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure for ZERO_EXTRACT */
struct bitfields {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
    unsigned int field4 : 16;
};

/* Volatile bit-field to force actual memory operations */
volatile struct bitfields g_bf;

NOINLINE void test_zero_extract(void) {
    /* These assignments should generate ZERO_EXTRACT in SET_DEST */
    struct bitfields local_bf;
    
    /* Assignment to bit-field members */
    local_bf.field1 = 5;          /* Should generate ZERO_EXTRACT */
    local_bf.field2 = 0x1F;       /* Max value for 5-bit field */
    local_bf.field3 = 0xFF;       /* Max value for 8-bit field */
    local_bf.field4 = 0xABCD;     /* 16-bit value */
    
    /* Volatile assignment to prevent optimization */
    g_bf.field1 = local_bf.field1;
    g_bf.field3 = local_bf.field3;
    
    /* Use asm to mark as used */
    asm volatile("" : : "r"(local_bf));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

NOINLINE void test_strict_low_part(short s, char c) {
    int dest1, dest2;
    long dest3;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    dest1 = s;      /* short to int - may use STRICT_LOW_PART */
    dest2 = c;      /* char to int - may use STRICT_LOW_PART */
    dest3 = s;      /* short to long - may use STRICT_LOW_PART */
    
    /* Force use of the results */
    g_temp = dest1 + dest2 + (int)dest3;
    
    /* Mixed operations that might generate partial register writes */
    unsigned int x = 0x12345678;
    unsigned short y = 0xABCD;
    
    /* This might generate STRICT_LOW_PART when writing back */
    x = (x & 0xFFFF0000) | y;
    
    g_temp = x;
}

/* ==================== SUBREG patterns ==================== */

/* Vector types for SUBREG generation */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    v4si vec_int = {1, 2, 3, 4};
    v8hi vec_short = {10, 20, 30, 40, 50, 60, 70, 80};
    v4sf vec_float = {1.0f, 2.0f, 3.0f, 4.0f};
    
    /* Vector operations that generate SUBREG */
    int lane0 = vec_int[0];          /* May generate SUBREG for extraction */
    short lane3 = vec_short[3];      /* Another SUBREG extraction */
    
    /* Type punning through union - often generates SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    int int_bits = pun.i;            /* May use SUBREG for bitcast */
    
    /* Vector store to scalar */
    vec_int[2] = lane0;              /* SUBREG in SET_DEST */
    vec_short[5] = lane3;            /* Another SUBREG destination */
    
    /* Use results */
    g_temp = lane0 + lane3 + int_bits;
}

/* ==================== MEM_P patterns ==================== */

NOINLINE void test_mem_p(int *base, int offset, int value) {
    int array[16];
    int *ptr;
    
    /* Complex address computation for MEM destination */
    ptr = &array[g_index];           /* Non-constant index */
    *ptr = value;                    /* MEM destination */
    
    /* More complex addressing mode */
    base[offset * 2 + 1] = value;    /* Complex address expression */
    
    /* Pointer arithmetic */
    int *p = base + (offset & 0x3);  /* Masked offset */
    *p = value * 2;                  /* Another MEM destination */
    
    /* Struct with pointer member */
    struct {
        int data[8];
        int *next;
    } s;
    
    s.data[offset % 8] = value;      /* MEM with struct member access */
    
    /* Force memory operations to not be optimized away */
    asm volatile("" : : "m"(array), "m"(s));
}

/* ==================== Combined test function ==================== */

/* Use O2 optimization specifically for this function */
__attribute__((optimize("O2")))
NOINLINE void combined_test(int *arr, int n) {
    /* Mix all patterns in one function */
    
    /* ZERO_EXTRACT */
    struct bitfields bf;
    bf.field2 = n & 0x1F;
    
    /* STRICT_LOW_PART */
    short s = n & 0xFFFF;
    int dest = s;
    
    /* SUBREG */
    v4si vec = {n, n+1, n+2, n+3};
    int elem = vec[n % 4];
    
    /* MEM_P with complex addressing */
    for (int i = 0; i < 4; i++) {
        arr[i * 2 + (n & 1)] = dest + elem + bf.field2;
    }
    
    /* Use all results */
    g_temp = dest + elem;
}

/* ==================== Main driver ==================== */

int main(void) {
    int array[32] = {0};
    
    /* Call test functions multiple times with different inputs */
    for (int i = 0; i < 10; i++) {
        test_zero_extract();
        test_strict_low_part(i * 100, i * 10);
        test_subreg();
        test_mem_p(array, i, i * 1000);
        combined_test(array, i);
        
        /* Modify global to change behavior */
        g_index = (g_index + 1) & 0xF;
    }
    
    /* Final validation - compute checksum */
    int sum = 0;
    for (int i = 0; i < 32; i++) {
        sum += array[i];
    }
    
    return sum == 0 ? 0 : 1;
}
