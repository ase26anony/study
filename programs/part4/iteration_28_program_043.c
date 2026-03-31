/* test_expr_cc.c
 * 
 * This program is designed to trigger the uncovered lines in expr.cc
 * related to constant-bounded array/aggregate initialization.
 * It creates various scenarios to exercise the condition:
 *   if (const_bounds_p
 *       && tree_fits_shwi_p (lo_index)
 *       && tree_fits_shwi_p (hi_index)
 *       && (lo = tree_to_shwi (lo_index),
 *           hi = tree_to_shwi (hi_index),
 *           count = hi - lo + 1,
 *           (!MEM_P (target)
 *            || count <= 2
 *            || (tree_fits_uhwi_p (TYPE_SIZE (elttype))
 *                && (tree_to_uhwi (TYPE_SIZE (elttype)) * count ...
 */

#include <stdio.h>

/* Use enum to define constant bounds */
enum { L = 2, H = 5 };
enum { SMALL_COUNT = 2 };

/* Packed struct with constant bitfield size */
struct __attribute__((packed)) PackedStruct {
    int a : 7;
    int b : 9;
    char c;
};

/* Struct containing an array */
struct WithArray {
    int x;
    int arr[10];
    struct PackedStruct ps;
};

/* Global/static initializations (likely MEM_P targets) */

/* Large static array with wide constant range - triggers count > 2, MEM_P(target) */
static int big_array[100] = { [10 ... 90] = 99 };  /* count = 81 */

/* Multi-dimensional array with nested constant range */
static int md_array[5][6] = { [1 ... 3][2 ... 4] = 7 };  /* 3x3 subarray */

/* Packed struct array with constant range */
static struct PackedStruct packed_arr[8] = { [1 ... 6] = { .a = 1, .b = 2, .c = 3 } };

int main(void) {
    /* 1. Register target with count <= 2: !MEM_P(target) path */
    register struct PackedStruct reg_target = { .a = 1, .b = 2, .c = 3 };
    
    /* 2. Automatic array with small constant range (count <= 2) */
    int small_range[10] = { [3] = 42, [4] = 43 };  /* count = 2 if considered as range? 
                                                     Actually individual designators, 
                                                     but we'll create explicit range */
    int small_range2[10] = { [5 ... 6] = 99 };     /* count = 2 */
    
    /* 3. Automatic array with constant range using enum bounds */
    int auto_array[20] = { [L ... H] = 255 };  /* count = 4 (H-L+1) */
    
    /* 4. Volatile array (definitely MEM_P) with constant range */
    volatile int volatile_array[15] = { [2 ... 8] = 777 };  /* count = 7 */
    
    /* 5. Nested struct with array initialization using constant range */
    struct WithArray wa = { 
        .x = 1, 
        .arr = { [1 ... 3] = 7 },  /* count = 3 */
        .ps = { .a = 3, .b = 4, .c = 5 }
    };
    
    /* 6. Compound literal assignment (creates initialization context) */
    int *ptr = (int[8]){ [0 ... 7] = 1 };  /* count = 8 */
    
    /* 7. Multi-dimensional automatic array with constant range */
    int local_md[4][5] = { [0 ... 2][1 ... 3] = 9 };  /* 3x3 subarray */
    
    /* 8. Array with exactly 1-element range (count = 1) */
    char single_range[100] = { [50] = 'X' };  /* Single element designated */
    char single_range_explicit[100] = { [25 ... 25] = 'Y' };  /* Explicit 1-element range */
    
    /* 9. Mixed initializers with constant bounds */
    struct Mixed {
        int a;
        int b[5];
        int c;
    } mixed = { 
        .a = 1, 
        .b = { [0 ... 4] = 2 },  /* count = 5 */
        .c = 3 
    };
    
    /* 10. Use __builtin_constant_p to assert constant-ness (optional) */
    if (__builtin_constant_p(L) && __builtin_constant_p(H)) {
        /* This branch should be taken since L and H are enum constants */
        int verify_array[10] = { [L ... H] = 123 };
        (void)verify_array;
    }
    
    /* Prevent dead code elimination by using values */
    int sum = 0;
    sum += reg_target.a;
    sum += small_range2[5];
    sum += auto_array[L];
    sum += volatile_array[2];
    sum += wa.arr[1];
    sum += ptr[0];
    sum += local_md[0][1];
    sum += single_range[50];
    sum += single_range_explicit[25];
    sum += mixed.b[0];
    sum += big_array[10];
    sum += md_array[1][2];
    sum += packed_arr[1].a;
    
    printf("Sum = %d\n", sum);
    printf("All constant-bounded initializations executed.\n");
    
    return 0;
}
