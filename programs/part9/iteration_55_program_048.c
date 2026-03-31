/* test_resource.c - Coverage test for GCC resource.cc mark_referenced_resources */
#include <stdint.h>
#include <string.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline))

/* Global variables to prevent constant propagation */
volatile int g_index = 3;
volatile int g_value = 42;
volatile int g_temp;

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
void test_zero_extract_and_mem(int *arr, int n) {
    struct bitfield_struct bs = {0};
    
    /* This should generate ZERO_EXTRACT for bit-field assignment */
    bs.field1 = 5;          /* ZERO_EXTRACT expected in SET_DEST */
    bs.field2 = 31;         /* Another bit-field assignment */
    
    /* Complex memory store - should generate MEM with address computation */
    int idx = g_index + n;
    if (idx >= 0 && idx < 10) {
        arr[idx] = g_value;  /* MEM_P expected in SET_DEST */
    }
    
    /* Another bit-field with volatile to prevent optimization */
    volatile struct bitfield_struct *vbs = &bs;
    vbs->field3 = 127;
    
    /* Use asm to prevent dead code elimination */
    asm volatile("" : : "r"(bs), "r"(arr));
}

/* Function 2: Focus on STRICT_LOW_PART and SUBREG */
NOINLINE __attribute__((optimize("O1")))
void test_strict_low_part_and_subreg(short src_short, char src_char) {
    int dest_int = 0;
    long dest_long = 0;
    
    /* These assignments may generate STRICT_LOW_PART for partial register writes */
    dest_int = src_short;   /* Possible STRICT_LOW_PART for 16->32 bit */
    dest_long = src_char;   /* Possible STRICT_LOW_PART for 8->64 bit */
    
    /* Vector operations for SUBREG generation */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    
    /* Vector store to memory - may generate SUBREG in SET_DEST */
    int temp[4];
    memcpy(temp, &vec_a, sizeof(vec_a));
    
    /* Vector lane extraction - generates SUBREG */
    int lane0 = vec_a[0];   /* SUBREG expected */
    int lane2 = vec_a[2];   /* Another SUBREG */
    
    /* Mixed-type assignment via pointer casting (bit casting) */
    float f = 3.14f;
    int float_as_int;
    memcpy(&float_as_int, &f, sizeof(float));  /* May generate SUBREG */
    
    /* Use all results to prevent optimization */
    g_temp = dest_int + dest_long + lane0 + lane2 + float_as_int;
}

/* Function 3: Complex memory addressing patterns */
NOINLINE __attribute__((optimize("O3")))
void test_complex_mem_addressing(struct bitfield_struct *bs_array, 
                                 int count, int base) {
    /* Complex addressing with multiple computations */
    for (int i = 0; i < count && i < 5; i++) {
        /* Array of structs with bit-field assignment */
        bs_array[i].field1 = i & 0x7;      /* ZERO_EXTRACT */
        
        /* Pointer arithmetic with scaling */
        int *ptr = (int *)(bs_array + i);
        *ptr = base + i;                   /* MEM_P with complex address */
        
        /* Nested addressing */
        if (i > 0) {
            bs_array[i-1].field2 = *ptr & 0x1F;  /* ZERO_EXTRACT with MEM source */
        }
    }
    
    /* Additional volatile store to force MEM generation */
    volatile int *volatile_ptr = (volatile int *)&g_value;
    *volatile_ptr = base;
}

/* Function 4: Mixed operations emphasizing 32-bit partial writes */
NOINLINE __attribute__((optimize("O1")))
void test_mixed_32bit(volatile short *sptr, volatile int *iptr) {
    /* Multiple partial register writes */
    for (int i = 0; i < 4; i++) {
        short s = *sptr + i;
        int val = s;                     /* Potential STRICT_LOW_PART */
        *iptr = val;                     /* MEM_P store */
        
        /* Bit-field in loop */
        struct bitfield_struct bs;
        bs.field3 = val & 0xFF;          /* ZERO_EXTRACT */
        
        /* Use the result */
        g_temp += bs.field3;
    }
}

/* Helper to generate varying addresses */
NOINLINE
int* get_address(int *base, int offset) {
    return base + (offset * 2) / 2;  /* Non-trivial computation */
}

/* Main function that exercises all patterns */
int main(void) {
    int array[10] = {0};
    struct bitfield_struct bs_array[5];
    
    /* Test 1: ZERO_EXTRACT and MEM_P */
    test_zero_extract_and_mem(array, 2);
    
    /* Test 2: STRICT_LOW_PART and SUBREG */
    test_strict_low_part_and_subreg(1000, 50);
    
    /* Test 3: Complex memory addressing */
    test_complex_mem_addressing(bs_array, 5, 100);
    
    /* Test 4: Mixed operations with volatile */
    volatile short s = 500;
    volatile int i = 0;
    test_mixed_32bit(&s, &i);
    
    /* Additional test with computed addresses */
    int *ptr = get_address(array, g_index);
    *ptr = 999;                         /* MEM_P with computed address */
    
    /* Bit-field through pointer */
    struct bitfield_struct *bs_ptr = &bs_array[0];
    bs_ptr->field2 = 20;                /* ZERO_EXTRACT through pointer */
    
    /* Force use of all results */
    asm volatile("" : : "r"(array), "r"(bs_array), "r"(g_temp));
    
    return g_temp > 0 ? 0 : 1;
}
