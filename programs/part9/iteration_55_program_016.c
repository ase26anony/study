/* test_resource.c - Program to exercise specific RTL patterns in GCC's resource tracking */

#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure functions generate separate RTL */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int global_index = 3;
volatile int global_value = 42;

/* ==================== ZERO_EXTRACT patterns ==================== */

/* Bit-field structure - assignments generate ZERO_EXTRACT in RTL */
struct bitfield_struct {
    unsigned int field3 : 3;    /* 3-bit field */
    unsigned int field5 : 5;    /* 5-bit field */
    unsigned int field8 : 8;    /* 8-bit field */
};

/* Volatile to prevent optimization */
volatile struct bitfield_struct bf;

NOINLINE void test_zero_extract(void) {
    /* These assignments should generate ZERO_EXTRACT as SET_DEST */
    bf.field3 = 5;      /* ZERO_EXTRACT of 3 bits */
    bf.field5 = 20;     /* ZERO_EXTRACT of 5 bits */
    bf.field8 = 100;    /* ZERO_EXTRACT of 8 bits */
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bf));
}

/* Another ZERO_EXTRACT pattern using unions */
union mixed_bf {
    struct {
        unsigned short low : 4;
        unsigned short high : 4;
        unsigned short pad : 8;
    } bits;
    unsigned short word;
};

volatile union mixed_bf ubf;

NOINLINE void test_zero_extract_union(void) {
    ubf.bits.low = 0xA;     /* ZERO_EXTRACT of 4 bits */
    ubf.bits.high = 0x5;    /* ZERO_EXTRACT of 4 bits */
    
    /* Force use */
    asm volatile("" : : "r"(ubf.word));
}

/* ==================== STRICT_LOW_PART patterns ==================== */

/* These patterns work best on 32-bit x86 with -m32 flag */
NOINLINE void test_strict_low_part(int x) {
    volatile short src_short = x & 0xFFFF;
    volatile char src_char = x & 0xFF;
    
    /* These assignments may generate STRICT_LOW_PART when writing
       to partial registers on x86 */
    int dest_int = src_short;    /* Possible STRICT_LOW_PART for lower 16 bits */
    long dest_long = src_char;   /* Possible STRICT_LOW_PART for lower 8 bits */
    
    /* Mix operations to prevent optimization */
    dest_int += dest_long;
    dest_long *= dest_int;
    
    /* Force use */
    asm volatile("" : : "r"(dest_int), "r"(dest_long));
}

/* Function with optimization attribute to encourage STRICT_LOW_PART */
__attribute__((optimize("O1")))
NOINLINE void test_strict_low_part_opt(short s, char c) {
    /* Multiple partial register writes */
    int a = s;      /* short -> int may use STRICT_LOW_PART */
    int b = c;      /* char -> int may use STRICT_LOW_PART */
    
    /* Use both results */
    a = a + b;
    b = b - a;
    
    asm volatile("" : : "r"(a), "r"(b));
}

/* ==================== SUBREG patterns ==================== */

/* GCC vector types often generate SUBREG operations */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE void test_subreg(void) {
    volatile v4si vec_a = {1, 2, 3, 4};
    volatile v4si vec_b = {5, 6, 7, 8};
    
    /* Vector operations generate SUBREG when extracting lanes */
    int lane0 = vec_a[0];    /* SUBREG extract from vector */
    int lane1 = vec_b[1];    /* SUBREG extract from vector */
    
    /* Cast between vector types */
    v8hi short_vec = (v8hi)vec_a;  /* SUBREG for type conversion */
    
    /* Mixed-size operations */
    short_vec[0] = (short)lane0;   /* SUBREG for store to vector lane */
    
    asm volatile("" : : "r"(lane0), "r"(lane1), "r"(short_vec));
}

/* Type punning through unions generates SUBREG */
NOINLINE void test_subreg_union(void) {
    union {
        float f;
        int i;
        uint32_t u;
    } pun;
    
    pun.f = 3.14159f;
    /* Access through different types generates SUBREG */
    int int_view = pun.i;          /* SUBREG for type reinterpretation */
    uint32_t uint_view = pun.u;    /* Another SUBREG */
    
    /* Force use */
    asm volatile("" : : "r"(int_view), "r"(uint_view));
}

/* ==================== MEM_P patterns ==================== */

/* Complex memory addressing */
NOINLINE void test_mem_dest(int *base, int index, int value) {
    /* Various memory destinations with complex addressing */
    base[index] = value;                     /* MEM with index */
    base[index + 1] = value * 2;             /* MEM with computed index */
    base[global_index] = global_value;       /* MEM with global index */
    
    /* Pointer arithmetic */
    int *ptr = base + index;
    *ptr = value + 1;                        /* MEM with pointer dereference */
    ptr[2] = value + 2;                      /* MEM with offset */
    
    /* Force memory writes to happen */
    asm volatile("" : : "r"(base));
}

/* Struct with pointer member access */
struct data {
    int array[10];
    int value;
};

NOINLINE void test_mem_struct(struct data *d, int idx) {
    /* Complex memory destinations through struct */
    d->array[idx] = global_value;            /* MEM with struct+array+index */
    d->array[idx + 1] = d->value;            /* MEM with multiple indirections */
    d->value = idx * 2;                      /* MEM with struct member */
    
    /* Pointer to member */
    int *member_ptr = &d->value;
    *member_ptr = idx + 1;                   /* MEM through pointer */
    
    asm volatile("" : : "r"(d));
}

/* ==================== Combined test function ==================== */

/* Function that combines multiple patterns */
NOINLINE void combined_test(int *arr, int idx) {
    /* ZERO_EXTRACT */
    struct bitfield_struct local_bf;
    local_bf.field3 = idx & 0x7;
    local_bf.field5 = (idx >> 3) & 0x1F;
    
    /* STRICT_LOW_PART (partial register write) */
    short partial = idx & 0x7FFF;
    int widened = partial;  /* May generate STRICT_LOW_PART */
    
    /* SUBREG */
    v4si vec = {idx, idx+1, idx+2, idx+3};
    int element = vec[0];  /* SUBREG extract */
    
    /* MEM_P with complex address */
    arr[idx] = widened + element;
    arr[idx + local_bf.field3] = partial;
    
    /* Use all results */
    asm volatile("" : : "r"(local_bf), "r"(widened), "r"(element), "r"(arr));
}

/* ==================== Main function ==================== */

int main(void) {
    int array[20] = {0};
    struct data d = {{0}, 0};
    
    /* Test ZERO_EXTRACT patterns */
    test_zero_extract();
    test_zero_extract_union();
    
    /* Test STRICT_LOW_PART patterns */
    test_strict_low_part(12345);
    test_strict_low_part_opt(100, 50);
    
    /* Test SUBREG patterns */
    test_subreg();
    test_subreg_union();
    
    /* Test MEM_P patterns */
    test_mem_dest(array, global_index, global_value);
    test_mem_struct(&d, 5);
    
    /* Combined test */
    for (int i = 0; i < 5; i++) {
        combined_test(array, i);
    }
    
    /* Prevent dead code elimination of entire program */
    asm volatile("" : : "r"(array), "r"(d));
    
    return 0;
}
