/* test_expr_bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700.
 * It uses GNU C extensions for designated initializers with constant ranges.
 */

#include <stdio.h>
#include <stddef.h>

/* Use enum to define constant bounds */
enum { L = 2, H = 5 };
enum { START = 10, END = 90 };

/* Small packed struct with constant bitfield sizes */
struct Packed {
    int a : 7;
    int b : 9;
} __attribute__((packed));

/* Struct containing an array */
struct WithArray {
    int x;
    int arr[8];
};

/* Global/static initializations (MEM_P(target) likely true) */
static int global_arr[100] = { [START ... END] = 99 };  /* count > 2, constant size */

/* Multi-dimensional array with constant range */
static int md_arr[4][5] = { [0 ... 2][1 ... 3] = 7 };

/* Packed struct array */
static struct Packed packed_arr[10] = { [1 ... 4] = { .a = 3, .b = 5 } };

int main(void) {
    /* 1. Register target with count <= 2 (trigger !MEM_P(target) path) */
    register int reg_target = (int){ [0] = 1, [1] = 2 };  /* count = 2 */
    
    /* 2. Automatic array with constant range (count > 2, MEM_P(target) true) */
    int auto_arr[20] = { [L ... H] = 42 };  /* count = 4 */
    
    /* 3. Small range (count <= 2) with MEM_P target */
    volatile int volatile_arr[5] = { [3] = 100 };  /* count = 1, volatile forces MEM_P */
    
    /* 4. Nested block with struct containing array initialization */
    {
        struct WithArray s = { .x = 1, .arr = { [1 ... 3] = 77 } };
        printf("s.arr[2] = %d\n", s.arr[2]);
    }
    
    /* 5. Compound literal assignment (creates initialization context) */
    int *ptr = (int[6]){ [0 ... 5] = 9 };
    
    /* 6. Mixed initialization in conditional with constant condition */
    if (1) {  /* Always true to ensure initialization is parsed */
        int cond_arr[8] = { [0 ... 7] = 0 };
        cond_arr[0] = reg_target;
    }
    
    /* 7. Use __builtin_constant_p to assert constant-ness (optional) */
    if (__builtin_constant_p(L) && __builtin_constant_p(H)) {
        printf("Bounds are constant: L=%d, H=%d\n", L, H);
    }
    
    /* Prevent dead code elimination */
    printf("Results: %d, %d, %d, %d\n", 
           reg_target, 
           auto_arr[L], 
           volatile_arr[3], 
           global_arr[START]);
    
    /* Use multi-dimensional array */
    printf("md_arr[1][2] = %d\n", md_arr[1][2]);
    
    /* Use packed struct array */
    printf("packed_arr[2].a = %d\n", packed_arr[2].a);
    
    return 0;
}
