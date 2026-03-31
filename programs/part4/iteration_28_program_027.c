/* test_expr_coverage.c
 * Designed to trigger constant bounds checking in GCC's expr.cc
 * Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_coverage.c -o test_expr_coverage
 * Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test_expr_coverage.c -o test_expr_coverage_opt
 */

#include <stdio.h>
#include <stdint.h>

/* Use enum to ensure constant folding */
enum { L = 2, H = 5, SMALL_COUNT = 2 };

/* Packed struct with constant bitfield sizes */
struct __attribute__((packed)) PackedStruct {
    int a:7;
    int b:9;
    int c:16;
};

/* Struct containing array for nested initialization */
struct Container {
    int id;
    int data[10];
    struct PackedStruct ps;
};

/* Static large array - will be MEM_P(target) with count > 2 */
static int large_array[100] = { [10 ... 90] = 99 };  /* count = 81 > 2 */

/* Multi-dimensional array with constant bounds */
static int md_array[4][5] = { [0 ... 2][1 ... 3] = 7 };  /* Nested constant ranges */

/* Function to force register target with !MEM_P(target) */
static void test_register_target(void) {
    /* Use register keyword to encourage register allocation */
    register struct PackedStruct reg_target = { .a = 1, .b = 2, .c = 3 };
    
    /* Designated initializer with constant range of 2 elements (count <= 2) */
    register int reg_arr[5] = { [1 ... 2] = 42 };  /* count = 2 */
    
    /* Compound literal assignment - may create initialization context */
    struct PackedStruct ps;
    ps = (struct PackedStruct){ .a = 5, .b = 10, .c = 15 };
    
    /* Use values to prevent elimination */
    printf("Register target: %d %d %d\n", reg_target.a, reg_target.b, reg_target.c);
    printf("Register array: %d %d\n", reg_arr[1], reg_arr[2]);
    printf("Compound literal: %d %d %d\n", ps.a, ps.b, ps.c);
}

/* Function with automatic variables and volatile */
static void test_memory_targets(void) {
    /* Automatic array - may be MEM_P or register depending on optimization */
    int auto_array[20] = { [3 ... 8] = 17 };  /* count = 6 > 2, MEM_P likely */
    
    /* Volatile ensures MEM_P(target) */
    volatile int volatile_array[15] = { [5 ... 10] = 23 };  /* count = 6 > 2 */
    
    /* Small count with memory target */
    char small_mem[10] = { [7] = 65 };  /* count = 1 <= 2 */
    
    /* Use __builtin_constant_p to verify constant bounds */
    if (__builtin_constant_p(L) && __builtin_constant_p(H)) {
        int const_range_array[10] = { [L ... H] = 88 };  /* count = 4 > 2 */
        printf("Constant range array[%d]: %d\n", L, const_range_array[L]);
    }
    
    printf("Auto array[5]: %d\n", auto_array[5]);
    printf("Volatile array[7]: %d\n", volatile_array[7]);
    printf("Small mem[7]: %c\n", small_mem[7]);
}

/* Test nested struct initialization with constant bounds */
static void test_nested_aggregates(void) {
    /* Struct with array member using designated initializer range */
    struct Container cont = {
        .id = 100,
        .data = { [2 ... 6] = 255 },  /* count = 5 > 2 */
        .ps = { .a = 3, .b = 7, .c = 255 }
    };
    
    /* Array of packed structs with constant range */
    struct PackedStruct ps_array[8] = { [1 ... 4] = { .a = 1, .b = 2, .c = 3 } };
    
    /* Multi-dimensional with mixed bounds */
    int mixed_md[3][4][5] = { 
        [0 ... 1][2 ... 3][1 ... 2] = 99  /* Complex constant range */
    };
    
    printf("Container data[3]: %d\n", cont.data[3]);
    printf("Packed array[2].b: %d\n", ps_array[2].b);
    printf("3D array[0][2][1]: %d\n", mixed_md[0][2][1]);
}

/* Test different element types with constant sizes */
static void test_element_types(void) {
    /* Different types to test TYPE_SIZE fitting in uhwi */
    short short_array[50] = { [10 ... 40] = -12345 };  /* count = 31 > 2 */
    long long_array[30] = { [5 ... 15] = 999999999L };  /* count = 11 > 2 */
    float float_array[20] = { [3 ... 12] = 3.14f };     /* count = 10 > 2 */
    double double_array[25] = { [5 ... 20] = 2.71828 }; /* count = 16 > 2 */
    
    /* Very small element type */
    unsigned char tiny_array[100] = { [20 ... 80] = 0xAB };  /* count = 61 > 2 */
    
    printf("Short array[25]: %d\n", short_array[25]);
    printf("Long array[10]: %ld\n", long_array[10]);
    printf("Float array[8]: %f\n", float_array[8]);
    printf("Double array[15]: %f\n", double_array[15]);
    printf("Tiny array[50]: 0x%02x\n", tiny_array[50]);
}

/* Main function orchestrating all tests */
int main(void) {
    printf("=== Testing expr.cc constant bounds coverage ===\n\n");
    
    /* 1. First test: register target with count <= 2 */
    printf("1. Testing register target (count <= 2):\n");
    test_register_target();
    printf("\n");
    
    /* 2. Test: static large array (MEM_P, count > 2) */
    printf("2. Testing static large array (MEM_P, count > 2):\n");
    printf("Large array[50]: %d\n", large_array[50]);
    printf("Multi-dim array[1][2]: %d\n", md_array[1][2]);
    printf("\n");
    
    /* 3. Test: automatic and volatile arrays */
    printf("3. Testing automatic/volatile arrays:\n");
    test_memory_targets();
    printf("\n");
    
    /* 4. Test: nested aggregates */
    printf("4. Testing nested aggregates:\n");
    test_nested_aggregates();
    printf("\n");
    
    /* 5. Test: different element types */
    printf("5. Testing different element types:\n");
    test_element_types();
    printf("\n");
    
    /* Additional edge case: exactly count = 2 with memory target */
    printf("6. Edge case: count = 2 with memory target:\n");
    int exact_two[10] = { [3 ... 4] = 777 };  /* count = 2 */
    printf("Exact two[3]: %d, [4]: %d\n", exact_two[3], exact_two[4]);
    
    /* Use all initialized values in computation */
    int sum = large_array[10] + md_array[0][0] + exact_two[3];
    printf("\nFinal computed sum: %d\n", sum);
    
    return 0;
}
