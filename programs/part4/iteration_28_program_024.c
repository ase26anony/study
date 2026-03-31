/* test_expr_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in expr.cc
 * related to constant-bounded array/aggregate initialization.
 * It creates various initialization patterns to exercise different
 * paths in the bounds checking logic.
 */

#include <stdio.h>
#include <stddef.h>

/* ==================== PART 1: Helper Types and Constants ==================== */

/* Use enum to define constant bounds that will be folded by the front-end */
enum ConstantBounds {
    L1 = 0,
    H1 = 1,      /* count = 2 */
    L2 = 10,
    H2 = 90,     /* count = 81 > 2 */
    L3 = 5,
    H3 = 5,      /* count = 1 */
    L4 = 0,
    H4 = 2       /* count = 3 > 2 */
};

/* Small packed struct with constant bitfield size */
struct Packed7_9 {
    unsigned int a : 7;
    unsigned int b : 9;
} __attribute__((packed));

/* Struct containing an array for nested initialization */
struct WithArray {
    int header;
    int data[8];
    struct Packed7_9 packed;
};

/* ==================== PART 2: Static Initializations (MEM_P targets) ==================== */

/* Large static array with wide constant range -> count > 2, MEM_P(target) true */
static int big_array[100] = { [L2 ... H2] = 99 };

/* Static struct with array member initialized with constant range */
static struct WithArray static_struct = {
    .header = 0xABCD,
    .data = { [2 ... 5] = 42 },   /* count = 4 > 2 */
    .packed = { .a = 0x7F, .b = 0x1FF }
};

/* ==================== PART 3: Automatic Variables in Functions ==================== */

/* Function to trigger various initialization patterns */
void test_initializations(void) {
    /* ----- Path: count <= 2 (regardless of MEM_P) ----- */
    /* Automatic array with exactly 2 elements in range */
    int small_range[10] = { [L1 ... H1] = 37 };  /* count = 2 */
    
    /* Single element range */
    int single_elem[20] = { [L3] = 123 };        /* count = 1 */
    
    /* ----- Path: !MEM_P(target) with register variable ----- */
    /* Try to force register target for a small struct */
    register struct Packed7_9 reg_target = { .a = 0x3F, .b = 0xFF };
    
    /* Compound literal assignment to register variable */
    register int reg_int = (int){ [0] = 1, [1] = 2 };  /* Designated init for scalar */
    
    /* ----- Path: MEM_P(target) && count > 2 && constant element size ----- */
    /* Volatile ensures MEM_P classification */
    volatile int volatile_array[50] = { [10 ... 40] = 999 };  /* count = 31 > 2 */
    
    /* Automatic large array with constant range */
    char char_big[200] = { [50 ... 150] = 'X' };  /* count = 101 > 2, eltsize = 1 */
    
    /* ----- Multi-dimensional array with nested constant ranges ----- */
    int md_array[5][6] = { [1 ... 3][2 ... 4] = 88 };  /* 3x3 subarray */
    
    /* ----- Nested struct with array initialization ----- */
    struct WithArray auto_struct = {
        .data = { [1 ... 3] = 7 },  /* count = 3 > 2 */
        .packed = { .a = 0x40, .b = 0x100 }
    };
    
    /* ==================== PART 4: Use Variables to Prevent Dead Code Elimination ==================== */
    
    int sum = 0;
    
    /* Use all initialized values in computations */
    sum += small_range[0] + small_range[1];
    sum += single_elem[L3];
    sum += reg_target.a + reg_target.b;
    sum += reg_int;
    sum += volatile_array[20];
    sum += char_big[100];
    sum += md_array[2][3];
    sum += auto_struct.data[2];
    sum += static_struct.data[3];
    sum += big_array[50];
    
    /* Print something to ensure side effects */
    printf("Initialization test sum: %d\n", sum);
    printf("Static big_array[50] = %d\n", big_array[50]);
    printf("Static struct data[3] = %d\n", static_struct.data[3]);
}

/* ==================== PART 5: Additional Contexts ==================== */

/* Function with conditional constant initialization */
void conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Array with constant bounds inside conditional block */
        int cond_array[30] = { [5 ... 15] = selector };  /* count = 11 > 2 */
        printf("Conditional array[10] = %d\n", cond_array[10]);
    } else {
        /* Different range */
        int cond_array[30] = { [20 ... 25] = selector };  /* count = 6 > 2 */
        printf("Conditional array[22] = %d\n", cond_array[22]);
    }
}

/* ==================== PART 6: Main Function ==================== */

int main(void) {
    printf("=== Testing expr.cc constant bounds initialization coverage ===\n");
    
    /* Trigger the main initialization test */
    test_initializations();
    
    /* Test conditional initialization paths */
    conditional_init(1);
    conditional_init(0);
    
    /* Additional test with attribute aligned */
    int __attribute__((aligned(32))) aligned_array[64] = { [16 ... 48] = 255 };  /* count = 33 > 2 */
    printf("Aligned array[32] = %d\n", aligned_array[32]);
    
    /* Test with different element types */
    short short_array[100] = { [20 ... 80] = -1 };  /* count = 61 > 2, eltsize = 2 */
    printf("Short array[50] = %d\n", short_array[50]);
    
    /* Packed struct array */
    struct Packed7_9 packed_array[10] = { [2 ... 7] = { .a = 0x7F, .b = 0x1FF } };  /* count = 6 > 2 */
    printf("Packed array[5].a = %u\n", packed_array[5].a);
    
    return 0;
}
