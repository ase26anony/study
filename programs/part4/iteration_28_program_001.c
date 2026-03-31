/* test_expr_coverage.c
 * 
 * This program is designed to trigger the uncovered lines in expr.cc
 * related to constant-bounded array/aggregate initialization.
 * It uses GNU C extensions for designated initializers with ranges.
 */

#include <stdio.h>
#include <stddef.h>

/* Use enum to define constant bounds that will be folded by the front-end */
enum { L = 2, H = 5 };
enum { SMALL_COUNT = 2 };
enum { BIG_START = 10, BIG_END = 90 };

/* A small packed struct with constant bitfield sizes */
struct Packed {
    int a:7;
    int b:9;
} __attribute__((packed));

/* Struct containing an array for nested initialization */
struct WithArray {
    int x;
    int arr[10];
};

/* Global/static initializations (MEM_P(target) likely true) */

/* Large array with wide constant range - triggers count > 2 path */
static int big_array[100] = { [BIG_START ... BIG_END] = 99 };

/* Multi-dimensional array with nested constant range */
static int md_array[3][4] = { [0 ... 1][2 ... 3] = 5 };

/* Packed struct array with constant range */
static struct Packed packed_arr[10] = { [1 ... 3] = { .a = 1, .b = 2 } };

int main(void) {
    /* 1. Register target scenario (!MEM_P(target)) 
     * Use a small struct that might be initialized in registers */
    register struct Packed reg_target = { .a = 3, .b = 4 };
    
    /* 2. Small count scenario (count <= 2) with automatic variable */
    int small_range[10] = { [0] = 1, [1] = 2 };  /* count = 2 */
    
    /* 3. Automatic array with constant range - may be register or memory */
    int auto_array[10] = { [L ... H] = 42 };
    
    /* 4. Volatile ensures MEM_P(target) == true */
    volatile int volatile_array[20] = { [5 ... 15] = 77 };
    
    /* 5. Nested block with struct containing array */
    {
        struct WithArray s = { .x = 1, .arr = { [1 ... 3] = 7 } };
        printf("s.arr[2] = %d\n", s.arr[2]);
    }
    
    /* 6. Compound literal assignment - creates initialization context */
    struct Packed *ptr = &(struct Packed){ .a = 5, .b = 6 };
    
    /* 7. Mixed element types with constant sizes */
    char char_array[50] = { [10 ... 20] = 'A' };
    short short_array[30] = { [5 ... 10] = 255 };
    
    /* 8. Designated initializer with single element (count = 1) */
    int single[100] = { [42] = 314 };
    
    /* 9. Array with alignment attribute */
    int aligned_array[64] __attribute__((aligned(64))) = { [0 ... 31] = 111 };
    
    /* Perform computations to prevent dead code elimination */
    int sum = 0;
    sum += reg_target.a + reg_target.b;
    sum += small_range[0];
    sum += auto_array[L];
    sum += volatile_array[10];
    sum += big_array[BIG_START];
    sum += md_array[0][2];
    sum += packed_arr[2].a;
    sum += char_array[15];
    sum += short_array[7];
    sum += single[42];
    sum += aligned_array[16];
    
    printf("Sum = %d\n", sum);
    printf("All initializations performed.\n");
    
    return 0;
}
