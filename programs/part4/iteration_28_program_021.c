/* test_expr_bounds.c
 * 
 * This program is designed to trigger the constant bounds checking logic
 * in GCC's expr.cc, specifically lines 7691-7700.
 * It uses GNU C extensions for designated initializers with constant ranges.
 *
 * Compile with:
 *   gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_bounds.c -o test_expr_bounds
 *   gcc -O2 -ftree-vectorize -std=gnu11 -fextended-identifiers test_expr_bounds.c -o test_expr_bounds_opt
 */

#include <stdio.h>
#include <stddef.h>

/* ==================== 1. Constant bounds via enum ==================== */
enum { L = 2, H = 5 };  /* Constant integer expressions */

/* ==================== 2. Small packed struct with constant size ==================== */
struct Packed7_9 {
    unsigned int a : 7;
    unsigned int b : 9;
} __attribute__((packed));

/* ==================== 3. Struct containing an array ==================== */
struct WithArray {
    int x;
    int arr[10];
};

/* ==================== 4. Multi-dimensional array type ==================== */
typedef int MDArray[3][4];

/* ==================== 5. Volatile array type (ensures MEM_P) ==================== */
typedef volatile int VolArray[20];

/* ==================== Global/static initializations ==================== */

/* Large static array with wide constant range -> count > 2, MEM_P(target) true */
static int big_array[100] = { [10 ... 90] = 99 };  /* count = 81 */

/* Packed struct with constant size */
static struct Packed7_9 packed_global = { .a = 0x7F, .b = 0x1FF };

/* ==================== Functions to force initialization in different contexts ==================== */

/* Function to trigger register-target initialization (!MEM_P(target)) */
static void test_register_target(void) {
    /* Use 'register' keyword to hint at register storage */
    register int reg_target = { [0] = 42 };  /* count = 1, !MEM_P likely */
    
    /* Small struct that might be initialized in registers */
    register struct { short a; short b; } reg_struct = { .a = 1, .b = 2 };
    
    /* Use the values to prevent elimination */
    printf("reg_target = %d, reg_struct = {%d, %d}\n", 
           reg_target, reg_struct.a, reg_struct.b);
}

/* Function with automatic variables and nested blocks */
static void test_automatic_vars(void) {
    /* Automatic array with constant range using enum bounds */
    int auto_arr[] = { [L ... H] = 42 };  /* count = 4, MEM_P likely */
    
    /* Nested block with another initialization */
    {
        /* Exactly 2 elements -> count <= 2 */
        int two_elems[10] = { [3] = 100, [4] = 200 };  /* count = 2 */
        printf("two_elems[3]=%d, two_elems[4]=%d\n", two_elems[3], two_elems[4]);
    }
    
    /* Use volatile to ensure MEM_P classification */
    VolArray volatile_arr = { [5 ... 9] = 777 };  /* count = 5 > 2, MEM_P true */
    
    printf("auto_arr[%d]=%d, volatile_arr[5]=%d\n", L, auto_arr[L], volatile_arr[5]);
}

/* Function with multi-dimensional array initialization */
static void test_multi_dim(void) {
    /* Multi-dimensional array with nested constant range */
    MDArray md = { [0 ... 1][2 ... 3] = 5 };  /* 2x2 block = 4 elements */
    
    /* Use the values */
    printf("md[0][2]=%d, md[1][3]=%d\n", md[0][2], md[1][3]);
}

/* Function with struct containing array initialized with constant range */
static void test_struct_with_array(void) {
    struct WithArray s = { 
        .x = 10, 
        .arr = { [1 ... 3] = 7 }  /* count = 3 > 2, MEM_P likely */
    };
    
    printf("s.x=%d, s.arr[2]=%d\n", s.x, s.arr[2]);
}

/* Function with conditional constant initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Array with single element -> count = 1 */
        int single[100] = { [42] = 999 };
        printf("single[42]=%d\n", single[42]);
    } else {
        /* Array with exactly 2 elements -> count = 2 */
        int double_arr[50] = { [10] = 111, [11] = 222 };
        printf("double_arr[10]=%d, double_arr[11]=%d\n", 
               double_arr[10], double_arr[11]);
    }
}

/* Function using compound literals */
static void test_compound_literal(void) {
    /* Compound literal creates an initialization context */
    struct Packed7_9 *ptr = &(struct Packed7_9){ .a = 63, .b = 511 };
    printf("compound literal: a=%u, b=%u\n", ptr->a, ptr->b);
}

/* ==================== main ==================== */
int main(void) {
    printf("Testing constant bounds initialization paths in expr.cc\n\n");
    
    /* 1. Register target with count <= 2 */
    test_register_target();
    
    /* 2. Large static array (count > 2, MEM_P true) */
    printf("big_array[50]=%d\n", big_array[50]);
    
    /* 3. Automatic variables with different counts */
    test_automatic_vars();
    
    /* 4. Multi-dimensional array */
    test_multi_dim();
    
    /* 5. Struct with array member */
    test_struct_with_array();
    
    /* 6. Conditional initialization */
    test_conditional_init(1);
    test_conditional_init(0);
    
    /* 7. Compound literal */
    test_compound_literal();
    
    /* 8. Global packed struct */
    printf("packed_global: a=%u, b=%u\n", packed_global.a, packed_global.b);
    
    /* 9. Array with constant bounds via enum */
    int enum_arr[] = { [L ... H] = 123 };
    printf("enum_arr[%d]=%d\n", H, enum_arr[H]);
    
    /* 10. Very small element type with large count */
    char char_big[1000] = { [100 ... 900] = 'A' };  /* count=801, element size=1 */
    printf("char_big[500]=%c\n", char_big[500]);
    
    return 0;
}
