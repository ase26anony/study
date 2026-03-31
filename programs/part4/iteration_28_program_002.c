/* Test program for GCC expr.cc constant bounds initialization coverage */
#include <stdio.h>

/* GNU C extensions required for designated range initializers */
#pragma GCC diagnostic ignored "-Wpedantic"

/* 1. Register target with count <= 2 */
static void test_register_target(void)
{
    /* Small struct that fits in register */
    struct SmallReg {
        int a;
        int b;
    } __attribute__((packed));
    
    /* Use register keyword to hint register allocation */
    register struct SmallReg reg_target = { .a = 1, .b = 2 };
    /* This should trigger !MEM_P(target) path */
    
    /* Also test with designated range of 2 elements */
    int reg_arr[10] = { [0 ... 1] = 42 };  /* count = 2 */
    
    printf("Register test: %d %d\n", reg_target.a, reg_arr[0]);
}

/* 2. Memory target with count > 2 and constant element size */
static void test_memory_target_large(void)
{
    /* Large static array with wide constant range */
    static int big_array[100] = { [10 ... 90] = 99 };  /* count = 81 > 2 */
    /* This should trigger MEM_P(target) && count > 2 path */
    
    /* Another with different element type */
    static char char_array[256] = { [32 ... 127] = 'A' };  /* count = 96 > 2 */
    
    printf("Large array: %d %c\n", big_array[50], char_array[64]);
}

/* 3. Mixed scenarios with different element types */
static void test_mixed_types(void)
{
    /* Struct with bitfields - constant odd size */
    struct PackedBits {
        unsigned int a : 7;
        unsigned int b : 9;
        unsigned int c : 3;
    } __attribute__((packed));
    
    /* Array of packed structs with constant range */
    struct PackedBits bits_arr[10] = { [2 ... 5] = { .a = 1, .b = 2, .c = 3 } };
    
    /* Volatile ensures MEM_P(target) */
    volatile int volatile_arr[20] = { [5 ... 10] = 123 };  /* count = 6 > 2 */
    
    printf("Mixed: %u %d\n", bits_arr[3].b, volatile_arr[7]);
}

/* 4. Multi-dimensional arrays with nested constant ranges */
static void test_multi_dimensional(void)
{
    /* 2D array with constant range in both dimensions */
    int matrix[5][5] = { 
        [0 ... 2][1 ... 3] = 7  /* Creates 3x3 submatrix */
    };
    
    /* 3D array with partial initialization */
    int cube[3][3][3] = {
        [0 ... 1][0 ... 1][0 ... 1] = 42  /* 2x2x2 subcube */
    };
    
    printf("Matrix: %d Cube: %d\n", matrix[1][2], cube[0][1][1]);
}

/* 5. Complex nested structures */
static void test_nested_aggregates(void)
{
    struct Inner {
        int data[4];
    };
    
    struct Outer {
        int id;
        struct Inner inner;
        int more[3];
    };
    
    /* Nested designated initializers with constant ranges */
    struct Outer complex = {
        .id = 1,
        .inner.data = { [1 ... 2] = 100 },  /* count = 2 */
        .more = { [0 ... 1] = 200 }         /* count = 2 */
    };
    
    printf("Nested: %d %d\n", complex.inner.data[1], complex.more[0]);
}

/* 6. Enum constants for bounds */
static void test_enum_bounds(void)
{
    enum Constants { 
        LO = 3, 
        HI = 8,
        SIZE = 20
    };
    
    /* Use enum constants as bounds */
    int enum_arr[SIZE] = { [LO ... HI] = 999 };  /* count = 6 > 2 */
    
    /* Const variables that fold to constants */
    const int c_lo = 1;
    const int c_hi = 4;
    int const_arr[10] = { [c_lo ... c_hi] = 777 };  /* count = 4 > 2 */
    
    printf("Enum: %d Const: %d\n", enum_arr[5], const_arr[2]);
}

/* 7. Compound literals as targets */
static void test_compound_literals(void)
{
    struct Point {
        int x;
        int y;
        int z;
    };
    
    /* Compound literal initialization */
    struct Point *ptr = &(struct Point){ 
        .x = 1, 
        .y = 2, 
        .z = 3 
    };
    
    /* Array compound literal with range */
    int *arr_ptr = (int[10]){ [2 ... 5] = 42 };  /* count = 4 > 2 */
    
    printf("Compound: %d %d\n", ptr->y, arr_ptr[3]);
}

/* 8. Conditional initialization with constant conditions */
static void test_conditional_init(void)
{
    int flag = 1;
    int result;
    
    /* Constant condition ensures initialization is parsed */
    if (flag) {
        /* Automatic array inside conditional block */
        int cond_arr[15] = { [5 ... 12] = 333 };  /* count = 8 > 2 */
        result = cond_arr[8];
    } else {
        /* Different range in else branch */
        int else_arr[10] = { [0 ... 1] = 111 };  /* count = 2 */
        result = else_arr[0];
    }
    
    printf("Conditional: %d\n", result);
}

/* 9. Alignment attributes affecting target classification */
static void test_aligned_targets(void)
{
    /* Highly aligned array might affect MEM_P classification */
    int aligned_arr[32] __attribute__((aligned(64))) = { 
        [8 ... 15] = 888  /* count = 8 > 2 */
    };
    
    printf("Aligned: %d\n", aligned_arr[12]);
}

/* 10. Very small count cases */
static void test_small_counts(void)
{
    /* Single element range */
    int single[10] = { [5] = 42 };  /* count = 1 <= 2 */
    
    /* Two element range */
    int double_arr[10] = { [3 ... 4] = 99 };  /* count = 2 <= 2 */
    
    printf("Small counts: %d %d\n", single[5], double_arr[3]);
}

int main(void)
{
    printf("Testing constant bounds initialization paths:\n\n");
    
    /* Execute all test cases to cover different paths */
    test_register_target();           /* !MEM_P(target) path */
    test_memory_target_large();       /* MEM_P(target) && count > 2 */
    test_mixed_types();               /* Various element types */
    test_multi_dimensional();         /* Nested ranges */
    test_nested_aggregates();         /* Complex structures */
    test_enum_bounds();               /* Constant bounds via enum */
    test_compound_literals();         /* Compound literal targets */
    test_conditional_init();          /* Conditional contexts */
    test_aligned_targets();           /* Alignment effects */
    test_small_counts();              /* count <= 2 cases */
    
    printf("\nAll tests completed.\n");
    return 0;
}
