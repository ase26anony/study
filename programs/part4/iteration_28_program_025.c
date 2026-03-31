/* test_expr_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in expr.cc
 * related to constant-bounded array/aggregate initialization.
 * It uses various GNU C extensions and initialization patterns to
 * exercise different paths in the bounds checking logic.
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

/* ==================== PART 2: Static Initializations (MEM_P(target) likely true) ==================== */

/* Large static array with wide constant range -> count > 2, MEM_P true */
static int big_array[100] = { [L2 ... H2] = 99 };

/* Static struct with array member initialized with constant range */
static struct WithArray static_struct = {
    .header = 42,
    .data = { [2 ... 5] = 7 },   /* count = 4 > 2 */
    .packed = { .a = 0x7F, .b = 0x1FF }
};

/* ==================== PART 3: Main Function with Various Initializations ==================== */

int main(void) {
    int result = 0;
    
    /* ----- Case 1: Register target with count <= 2 ----- */
    /* Use 'register' keyword to hint at register storage, small struct */
    register struct Packed7_9 reg_target = { .a = 1, .b = 2 };
    /* Designated initializer with constant range of 1 element -> count = 1 */
    int reg_array[10] = { [L3 ... H3] = 123 };
    result += reg_target.a + reg_array[5];
    
    /* ----- Case 2: Automatic array with constant range, count > 2 ----- */
    /* Automatic variable on stack -> likely MEM_P(target) true */
    int auto_array[20] = { [L4 ... H4] = 456 };   /* count = 3 > 2 */
    result += auto_array[0] + auto_array[1] + auto_array[2];
    
    /* ----- Case 3: Volatile array (definitely memory operand) ----- */
    volatile int volatile_array[30] = { [15 ... 25] = 789 };   /* count = 11 > 2 */
    result += volatile_array[20];
    
    /* ----- Case 4: Multi-dimensional array with nested constant range ----- */
    int md_array[4][5] = { [0 ... 2][1 ... 3] = 135 };   /* 3 rows, 3 cols each */
    result += md_array[1][2];
    
    /* ----- Case 5: Compound literal assignment (creates initialization context) ----- */
    struct WithArray *ptr = &static_struct;
    *ptr = (struct WithArray){
        .header = 100,
        .data = { [1 ... 6] = 222 },   /* count = 6 > 2 */
        .packed = { .a = 0x3F, .b = 0x100 }
    };
    result += ptr->data[3];
    
    /* ----- Case 6: Mixed element types with constant sizes ----- */
    char char_array[50] = { [10 ... 40] = 'X' };   /* count = 31 > 2, eltsize = 1 */
    short short_array[60] = { [20 ... 50] = 9999 }; /* count = 31 > 2, eltsize = 2 */
    result += char_array[25] + short_array[35];
    
    /* ----- Case 7: Using __builtin_constant_p to assert constant bounds ----- */
    if (__builtin_constant_p(L1) && __builtin_constant_p(H1)) {
        int verified_const[5] = { [L1 ... H1] = 333 };   /* count = 2 */
        result += verified_const[0];
    }
    
    /* ----- Case 8: Nested block with automatic struct ----- */
    {
        struct WithArray local_struct = {
            .data = { [0 ... 7] = 444 },   /* count = 8 > 2 */
            .packed = { .a = 0, .b = 0 }
        };
        result += local_struct.data[7];
    }
    
    /* ----- Case 9: Array with alignment attribute ----- */
    int aligned_array[64] __attribute__((aligned(64))) = { [16 ... 48] = 555 };
    result += aligned_array[32];
    
    /* ----- Case 10: Bitfield struct array ----- */
    struct Packed7_9 bf_array[10] = { [2 ... 8] = { .a = 0x7F, .b = 0x1FF } }; /* count = 7 > 2 */
    result += bf_array[5].a;
    
    /* ----- Use static array to prevent elimination ----- */
    result += big_array[50];
    
    /* ----- Print result to prevent dead code elimination ----- */
    printf("Result: %d\n", result);
    
    return 0;
}
