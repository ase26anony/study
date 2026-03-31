/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile char g_char = 'A';

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure - should generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field1 : 3;
    unsigned int field2 : 5;
    unsigned int field3 : 8;
};

NOINLINE void test_zero_extract(void) {
    /* Volatile to prevent optimization */
    volatile struct bitfield_struct bf;
    
    /* These assignments should generate SET with ZERO_EXTRACT destination */
    bf.field1 = 5;      /* ZERO_EXTRACT: 3-bit field assignment */
    bf.field2 = 20;     /* ZERO_EXTRACT: 5-bit field assignment */
    bf.field3 = 100;    /* ZERO_EXTRACT: 8-bit field assignment */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf));
}

/* Another ZERO_EXTRACT pattern using unions */
union mixed_bf {
    struct {
        unsigned short low : 4;
        unsigned short high : 12;
    } bits;
    unsigned short full;
};

NOINLINE void test_zero_extract_union(void) {
    volatile union mixed_bf u;
    u.bits.low = 7;     /* ZERO_EXTRACT: bit-field in union */
    u.bits.high = 255;  /* ZERO_EXTRACT: another bit-field */
    
    asm volatile("" : : "r"(u.full));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* Mixed-size assignments - common source of STRICT_LOW_PART */
NOINLINE void test_strict_low_part(void) {
    volatile short src16 = -12345;
    volatile char src8 = 127;
    
    /* These assignments may generate STRICT_LOW_PART on x86 */
    int dest32;
    
    /* short -> int assignment */
    dest32 = src16;     /* Potential STRICT_LOW_PART for lower 16 bits */
    
    /* char -> int assignment */
    dest32 = src8;      /* Potential STRICT_LOW_PART for lower 8 bits */
    
    /* Use inline assembly to force partial register writes */
    asm volatile (
        "movw %1, %0\n\t"  /* 16-bit move to 32-bit register */
        : "=r"(dest32)
        : "r"(src16)
    );
    
    asm volatile("" : : "r"(dest32));
}

/* Function with optimization to encourage STRICT_LOW_PART */
__attribute__((optimize("O1")))
NOINLINE void test_partial_reg_ops(int x) {
    volatile short s = x & 0xFFFF;
    int y = s;          /* short to int - potential STRICT_LOW_PART */
    
    /* Chain of operations to keep value live */
    y = y + (x >> 16);
    asm volatile("" : : "r"(y));
}

/* ==================== SUBREG patterns ==================== */

/* GCC vector types often generate SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    volatile v4si vec_a = {1, 2, 3, 4};
    volatile v4si vec_b = {5, 6, 7, 8};
    
    /* Vector operations generate SUBREG for lane access */
    vec_a = vec_b;      /* Vector assignment */
    
    /* Extract lane - generates SUBREG */
    int lane0 = vec_a[0];
    int lane1 = vec_a[1];
    
    /* Type punning through union - often uses SUBREG */
    union {
        float f;
        int i;
    } pun;
    pun.f = 3.14f;
    int int_bits = pun.i;  /* SUBREG for type conversion */
    
    asm volatile("" : : "r"(lane0), "r"(lane1), "r"(int_bits));
}

/* Mixed precision floating point */
NOINLINE void test_float_subreg(void) {
    volatile float f = 1.5f;
    volatile double d = 2.5;
    
    /* float to double conversion uses SUBREG */
    d = f;              /* SUBREG for float->double */
    
    /* Cast through pointer - generates SUBREG */
    int i = *(int*)&f;  /* SUBREG for bitcast */
    
    asm volatile("" : : "r"(d), "r"(i));
}

/* ==================== MEM_P patterns ==================== */

/* Complex memory addressing */
NOINLINE void test_mem_dest(int *base, int offset) {
    /* Complex address calculation for MEM destination */
    base[offset * 2 + 1] = g_value;           /* MEM with index */
    base[g_index] = offset;                   /* MEM with volatile index */
    
    /* Pointer arithmetic */
    int *ptr = base + offset;
    *ptr = g_value;                           /* MEM with computed address */
    
    /* Struct member through pointer */
    struct data {
        int a;
        int b;
        int c;
    };
    struct data *sptr = (struct data*)base;
    sptr->b = offset;                         /* MEM with struct offset */
    
    asm volatile("" : : "r"(base), "r"(offset));
}

/* Multi-dimensional array */
NOINLINE void test_array_mem(int n) {
    volatile int arr[10][10];
    
    /* Non-constant indices create complex MEM addresses */
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i][j] = i * j + n;            /* MEM with 2D indexing */
        }
    }
    
    /* Prevent loop elimination */
    asm volatile("" : : "r"(arr[5][5]));
}

/* ==================== Combined test function ==================== */

/* Function that combines multiple patterns */
NOINLINE void test_combined(int *mem, int idx) {
    /* ZERO_EXTRACT */
    struct {
        unsigned int flags : 4;
        unsigned int mode : 4;
    } opts;
    opts.flags = idx & 0xF;
    opts.mode = (idx >> 4) & 0xF;
    
    /* STRICT_LOW_PART */
    short s_val = idx;
    int i_val = s_val;
    
    /* SUBREG */
    typedef float v2f __attribute__((vector_size(8)));
    v2f v1 = {1.0f, 2.0f};
    v2f v2 = v1;
    float f = v2[0];
    
    /* MEM_P */
    mem[idx] = i_val;
    mem[idx + 1] = opts.flags;
    
    asm volatile("" : : "r"(opts), "r"(i_val), "r"(f), "r"(mem));
}

/* ==================== Main driver ==================== */

int main(void) {
    int buffer[100] = {0};
    int i;
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract();
    test_zero_extract_union();
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part();
    test_partial_reg_ops(0x12345678);
    
    /* Test SUBREG patterns */
    test_subreg();
    test_float_subreg();
    
    /* Test MEM_P patterns */
    test_mem_dest(buffer, g_index);
    test_array_mem(g_value);
    
    /* Combined test */
    for (i = 0; i < 10; i++) {
        test_combined(buffer, i);
    }
    
    /* Use results to prevent optimization */
    int sum = 0;
    for (i = 0; i < 100; i++) {
        sum += buffer[i];
    }
    
    return sum & 0xFF;  /* Return non-constant value */
}
