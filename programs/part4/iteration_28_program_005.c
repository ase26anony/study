/* test_expr_cc.c - Test program to cover constant bounds checking in GCC's expr.cc */
/* Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_cc.c -o test_expr_cc */
/* Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test_expr_cc.c -o test_expr_cc_opt */

#include <stdio.h>
#include <string.h>

/* ==================== PART 1: Constant bounds definitions ==================== */
enum ConstBounds {
    L1 = 2,
    H1 = 5,        /* count = 4 (>2) */
    L2 = 0,
    H2 = 1,        /* count = 2 (<=2) */
    L3 = 10,
    H3 = 90        /* count = 81 (>2) */
};

/* Small packed struct with constant bitfield size */
struct Packed7_9 {
    unsigned int a : 7;
    unsigned int b : 9;
} __attribute__((packed));

/* Struct containing array for nested initialization */
struct WithArray {
    int x;
    int arr[10];
    struct Packed7_9 p;
};

/* ==================== PART 2: Static initializations (MEM_P(target) likely true) ==================== */
/* Large static array with wide constant range -> count > 2, MEM_P true */
static int big_array[100] = { [L3 ... H3] = 99 };  /* 81 elements initialized */

/* Static struct with array member initialization using constant range */
static struct WithArray s1 = { .x = 1, .arr = { [2 ... 6] = 42 } };

/* ==================== PART 3: Register-target scenarios (!MEM_P(target)) ==================== */
/* Use register keyword to hint register storage */
static int test_register_target(void) {
    /* Small struct that might fit in registers */
    register struct { int a; int b; } reg_struct = { .a = 10, .b = 20 };
    
    /* Scalar with designated init (count=1) -> count <= 2 path */
    register int reg_scalar = { [0] = 100 };
    
    return reg_struct.a + reg_scalar;
}

/* ==================== PART 4: Automatic variables with various conditions ==================== */
static void test_automatic_vars(void) {
    /* Automatic array with small constant range -> count <= 2 */
    int small[5] = { [L2 ... H2] = 3 };  /* count = 2 */
    
    /* Automatic array with medium range -> count > 2, MEM_P likely true */
    int medium[20] = { [5 ... 15] = 7 }; /* count = 11 */
    
    /* Volatile array to force MEM_P(target) = true */
    volatile int volatile_arr[10] = { [2 ... 7] = 9 }; /* count = 6 */
    
    /* Packed struct initialization */
    struct Packed7_9 ps = { .a = 127, .b = 511 };
    
    /* Compound literal assignment (creates initialization context) */
    struct WithArray *ptr = &s1;
    *ptr = (struct WithArray){ .arr = { [1 ... 8] = 77 } };
    
    /* Use variables to prevent elimination */
    printf("small[0]=%d, medium[10]=%d, volatile_arr[5]=%d, ps.a=%d\n",
           small[0], medium[10], volatile_arr[5], ps.a);
}

/* ==================== PART 5: Multi-dimensional array with constant range ==================== */
static void test_multi_dim(void) {
    /* GCC extended designated initializer for 2D array */
    int md[4][5] = { [0 ... 2][1 ... 3] = 55 };  /* 3x3 subarray */
    
    /* Nested struct with array initialization */
    struct { struct WithArray inner; } nested = {
        .inner.arr = { [3 ... 7] = 123 }
    };
    
    printf("md[1][2]=%d, nested.inner.arr[5]=%d\n", md[1][2], nested.inner.arr[5]);
}

/* ==================== PART 6: Conditional initialization with constant bounds ==================== */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* This initialization should still be processed */
        int cond_arr[8] = { [1 ... 4] = selector }; /* count = 4 */
        printf("cond_arr[2]=%d\n", cond_arr[2]);
    } else {
        /* Different range with count = 1 */
        int cond_arr2[8] = { [5] = 999 }; /* count = 1 */
        printf("cond_arr2[5]=%d\n", cond_arr2[5]);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 1: {
            int switch_arr[6] = { [0 ... 2] = 111 }; /* count = 3 */
            printf("switch_arr[1]=%d\n", switch_arr[1]);
            break;
        }
        case 2: {
            int switch_arr[6] = { [3 ... 5] = 222 }; /* count = 3 */
            printf("switch_arr[4]=%d\n", switch_arr[4]);
            break;
        }
    }
}

/* ==================== MAIN: Orchestrate all test cases ==================== */
int main(void) {
    int result;
    
    printf("=== Testing expr.cc constant bounds checking ===\n");
    
    /* 1. Register target with count <= 2 */
    result = test_register_target();
    printf("Register test result: %d\n", result);
    
    /* 2. Static large array (count > 2, MEM_P true) */
    printf("big_array[50]=%d (should be 99)\n", big_array[50]);
    printf("big_array[0]=%d (should be 0)\n", big_array[0]);
    
    /* 3. Automatic variables with various conditions */
    test_automatic_vars();
    
    /* 4. Multi-dimensional arrays */
    test_multi_dim();
    
    /* 5. Conditional initializations */
    test_conditional_init(1);
    test_conditional_init(0);
    test_conditional_init(2);
    
    /* 6. Additional test: Array with exactly 2 elements in designated init */
    int exact_two[10] = { [3 ... 4] = 888 }; /* count = 2 */
    printf("exact_two[3]=%d, exact_two[4]=%d\n", exact_two[3], exact_two[4]);
    
    /* 7. Test with different element types */
    char char_arr[32] = { [8 ... 15] = 'A' }; /* count = 8 */
    short short_arr[32] = { [10 ... 20] = 32767 }; /* count = 11 */
    printf("char_arr[10]=%c, short_arr[15]=%d\n", char_arr[10], short_arr[15]);
    
    /* 8. Struct with bitfield array */
    struct BitfieldStruct {
        unsigned int flags[4];
    } bfs = { .flags = { [0 ... 2] = 0xFU } }; /* count = 3 */
    printf("bfs.flags[1]=%u\n", bfs.flags[1]);
    
    return 0;
}
