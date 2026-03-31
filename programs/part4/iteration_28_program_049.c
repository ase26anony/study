/* test_expr_constant_bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700.
 * It creates various initialization patterns with constant array/struct bounds
 * to exercise different paths in the uncovered block.
 */

#include <stdio.h>
#include <stddef.h>

/* ==================== PART 1: Helper types and constants ==================== */

/* Use enum to define constant bounds that will be folded by the front-end */
enum ConstBounds {
    L1 = 0,
    H1 = 1,      /* count = 2 */
    L2 = 10,
    H2 = 90,     /* count = 81 > 2 */
    L3 = 5,
    H3 = 5,      /* count = 1 */
    L4 = 2,
    H4 = 5       /* count = 4 > 2 */
};

/* Small packed struct with constant bitfield size */
struct Packed7_9 {
    unsigned int a : 7;
    unsigned int b : 9;
} __attribute__((packed));

/* Struct containing an array for nested designated initializers */
struct WithArray {
    int header;
    int data[8];
    struct Packed7_9 packed;
};

/* ==================== PART 2: Static (data section) initializations ==================== */

/* This will be MEM_P(target) with count > 2 and constant element size */
static int big_array[100] = { [L2 ... H2] = 99 };

/* Multi-dimensional array with constant nested range */
static int md_array[4][6] = { [1 ... 2][2 ... 4] = 77 };

/* Packed struct array with constant range (count > 2) */
static struct Packed7_9 packed_arr[10] = { [2 ... 7] = { .a = 3, .b = 5 } };

/* ==================== PART 3: Function with various automatic initializations ==================== */

int main(void) {
    int result = 0;
    
    /* -------------------- Scenario 1: Possibly !MEM_P(target) with count <= 2 -------------------- */
    /* Try to force a register target: small struct initialized with constant range of 2 elements */
    {
        register struct Packed7_9 reg_target = { .a = 1, .b = 2 };
        /* Simulate initialization with a constant range of 1 element via compound literal */
        reg_target = (struct Packed7_9){ .a = 5, .b = 10 };
        result += reg_target.a + reg_target.b;
    }
    
    /* -------------------- Scenario 2: MEM_P(target) with count <= 2 -------------------- */
    /* Automatic array with exactly 2 elements in the designated range */
    int small_range[10] = { [L1 ... H1] = 42 };
    result += small_range[0] + small_range[1];
    
    /* Single element range */
    int single_range[20] = { [L3] = 123 };
    result += single_range[5];
    
    /* -------------------- Scenario 3: MEM_P(target) with count > 2, constant element size -------------------- */
    /* Automatic large array with wide constant range */
    int auto_big[200] = { [30 ... 150] = 88 };
    result += auto_big[30] + auto_big[150];
    
    /* Use volatile to ensure MEM_P classification */
    volatile int volatile_arr[50] = { [10 ... 40] = 66 };
    result += volatile_arr[10];
    
    /* -------------------- Scenario 4: Nested aggregates with constant bounds -------------------- */
    struct WithArray nested = {
        .header = 1000,
        .data = { [2 ... 6] = 9 },
        .packed = { .a = 127, .b = 511 }
    };
    result += nested.data[3] + nested.packed.a;
    
    /* Multi-dimensional automatic array */
    int local_md[3][5] = { [0 ... 1][1 ... 3] = 13 };
    result += local_md[0][1];
    
    /* -------------------- Scenario 5: Conditional constant initialization -------------------- */
    if (1) { /* Always true to keep initialization but allow optimizer to see constant condition */
        int cond_arr[10] = { [L4 ... H4] = 55 };
        result += cond_arr[3];
    }
    
    /* Switch with constant case to create another initialization context */
    switch (5) {
        case 5: {
            int switch_arr[8] = { [1 ... 3] = 22 };
            result += switch_arr[2];
            break;
        }
        default:
            break;
    }
    
    /* -------------------- Use static arrays to prevent dead code elimination -------------------- */
    result += big_array[10] + big_array[90];
    result += md_array[1][2] + md_array[2][4];
    result += packed_arr[3].a + packed_arr[6].b;
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
