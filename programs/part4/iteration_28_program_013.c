/* test_expr_coverage.c
 * 
 * This program is designed to trigger the specific uncovered lines in expr.cc
 * related to constant-bounded array/aggregate initialization bounds checking.
 * It creates various initialization scenarios to exercise different paths
 * in the condition:
 *   if (const_bounds_p
 *       && tree_fits_shwi_p (lo_index)
 *       && tree_fits_shwi_p (hi_index)
 *       && (lo = tree_to_shwi (lo_index),
 *           hi = tree_to_shwi (hi_index),
 *           count = hi - lo + 1,
 *           (!MEM_P (target)
 *            || count <= 2
 *            || (tree_fits_uhwi_p (TYPE_SIZE (elttype))
 *                && (tree_to_uhwi (TYPE_SIZE (elttype)) * count ... )))))
 */

#include <stdio.h>
#include <stdint.h>

/* ==================== PART 1: Constant bounds definitions ==================== */
/* Use enum and const to ensure constant folding */
enum { L = 2, H = 5 };
enum { SMALL_L = 0, SMALL_H = 1 };
enum { BIG_L = 10, BIG_H = 90 };
enum { MDIM_L1 = 0, MDIM_H1 = 1, MDIM_L2 = 2, MDIM_H2 = 3 };

const int const_low = 3;
const int const_high = 8;

/* ==================== PART 2: Element types with constant sizes ==================== */
/* Basic types - guaranteed constant size */
typedef char small_type;
typedef int medium_type;
typedef long large_type;

/* Packed struct with constant bitfield size */
struct __attribute__((packed)) PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 13;
}; /* Total size: 29 bits = 4 bytes (on most platforms) */

/* Nested struct with array */
struct Container {
    int id;
    int values[10];
    struct PackedStruct ps;
};

/* ==================== PART 3: Initialization scenarios ==================== */

/* Scenario A: Register target with !MEM_P(target) likely true */
static void test_register_target(void) {
    /* Use 'register' keyword to hint at register allocation */
    register int reg_target = ({ 
        int temp = 0;
        /* Compound literal might be expanded with register target */
        temp = (int){ [L] = 100, [H] = 200 };  /* count = 4, but !MEM_P? */
        temp;
    });
    
    /* Small struct that might go into register */
    register struct { int a; int b; } reg_struct = { 
        .a = ({ int x = 0; x = (int){[0] = 5}; x; }),
        .b = ({ int y = 0; y = (int){[1] = 6}; y; })
    };
    
    printf("Register target - reg_target component: %d\n", reg_target);
    printf("Register struct - a=%d, b=%d\n", reg_struct.a, reg_struct.b);
}

/* Scenario B: count <= 2 (path taken regardless of MEM_P) */
static void test_small_count(void) {
    /* Case B1: Exactly 1 element - static storage (MEM_P true) */
    static int single_elem[] = { [10] = 42 };  /* count = 1 */
    
    /* Case B2: Exactly 2 elements - automatic storage */
    int two_elems[20] = { [5] = 1, [6] = 2 };  /* count = 2 */
    
    /* Case B3: Struct with small designated range */
    struct { int arr[10]; } s1 = { .arr = { [2] = 10, [3] = 20 } }; /* count = 2 */
    
    /* Use volatile to ensure MEM_P classification */
    volatile int vol_arr[10] = { [7] = 99 };  /* count = 1, MEM_P true */
    
    printf("Small count - single_elem[10]=%d\n", single_elem[10]);
    printf("Small count - two_elems[5]=%d, [6]=%d\n", two_elems[5], two_elems[6]);
    printf("Small count - vol_arr[7]=%d\n", vol_arr[7]);
}

/* Scenario C: count > 2, MEM_P(target) true, constant element size */
static void test_large_count_const_size(void) {
    /* Large array with wide constant range - count = 81 */
    static int big_array[100] = { [BIG_L ... BIG_H] = 0xABCD };
    
    /* Array of packed structs with constant range */
    struct PackedStruct ps_array[50] = { 
        [10 ... 40] = { .a = 1, .b = 2, .c = 3 }  /* count = 31 */
    };
    
    /* Multi-dimensional with large constant range */
    int md1[3][4] = { [0 ... 1][2 ... 3] = 5 };  /* count = 4 (2*2) */
    
    printf("Large count - big_array[50]=%d\n", big_array[50]);
    printf("Large count - ps_array[20].b=%u\n", ps_array[20].b);
    printf("Large count - md1[0][2]=%d, [1][3]=%d\n", md1[0][2], md1[1][3]);
}

/* Scenario D: Mixed contexts to explore different target classifications */
static void test_mixed_contexts(void) {
    /* D1: Static initialization (definitely MEM_P) with medium count */
    static char static_chars[100] = { [20 ... 50] = 'X' };  /* count = 31 */
    
    /* D2: Automatic with constant bounds from const variables */
    int auto_arr[20] = { [const_low ... const_high] = 777 };  /* count = 6 */
    
    /* D3: Volatile pointer target */
    volatile int vol_target[10];
    /* Simulate initialization via memcpy-like pattern the compiler might generate */
    *vol_target = (int){ [0 ... 2] = 123 };  /* count = 3, MEM_P true */
    
    /* D4: Nested block with register variable */
    {
        register int r = ({ 
            int tmp = (int){[1 ... 3] = 456};  /* count = 3, !MEM_P possible */
            tmp;
        });
        printf("Mixed - register in nested block: %d\n", r);
    }
    
    printf("Mixed - static_chars[30]=%c\n", static_chars[30]);
    printf("Mixed - auto_arr[5]=%d\n", auto_arr[5]);
    printf("Mixed - vol_target[1]=%d\n", vol_target[1]);
}

/* Scenario E: Complex nested aggregates */
static void test_nested_aggregates(void) {
    /* Struct containing array with designated range */
    struct Container c = { 
        .id = 1,
        .values = { [2 ... 7] = 42 },  /* count = 6 */
        .ps = { .a = 7, .b = 255, .c = 8191 }
    };
    
    /* Array of structs with constant range */
    struct Container carr[10] = { 
        [1 ... 5] = { 
            .id = 99, 
            .values = { [0 ... 9] = 88 },  /* count = 10 */
            .ps = { .a = 3, .b = 127, .c = 4095 }
        }
    };  /* Outer count = 5 */
    
    /* Multi-dimensional with multiple ranges */
    int md2[5][5][5] = { 
        [0 ... 2][1 ... 3][2 ... 4] = 999  /* count = 3*3*3 = 27 */
    };
    
    printf("Nested - c.values[5]=%d\n", c.values[5]);
    printf("Nested - carr[3].values[9]=%d\n", carr[3].values[9]);
    printf("Nested - md2[1][2][3]=%d\n", md2[1][2][3]);
}

/* Scenario F: Edge cases with constant expressions */
static void test_edge_cases(void) {
    /* Using sizeof in bounds (constant expression) */
    int size_based[100] = { [sizeof(int) ... 2*sizeof(int)] = 0xFF };
    
    /* Enum in array designator */
    enum { E1 = 1, E2 = 8 };
    int enum_arr[20] = { [E1 ... E2] = 0xCC };
    
    /* Character array with byte-sized elements */
    char byte_arr[256] = { [0x10 ... 0x20] = 0x7F };
    
    /* Bitfield array - each element has constant size */
    struct { unsigned bits : 4; } bit_arr[10] = { [2 ... 5] = { .bits = 15 } };
    
    printf("Edge - size_based[6]=%d\n", size_based[6]);
    printf("Edge - enum_arr[5]=%d\n", enum_arr[5]);
    printf("Edge - byte_arr[0x15]=%d\n", byte_arr[0x15]);
    printf("Edge - bit_arr[3].bits=%u\n", bit_arr[3].bits);
}

/* ==================== MAIN: Orchestrate all scenarios ==================== */
int main(void) {
    printf("=== Testing expr.cc constant bounds initialization coverage ===\n\n");
    
    /* 1. First: Register target scenario (exploring !MEM_P(target)) */
    printf("1. Testing register target scenario:\n");
    test_register_target();
    printf("\n");
    
    /* 2. Small count scenario (count <= 2) */
    printf("2. Testing small count scenario:\n");
    test_small_count();
    printf("\n");
    
    /* 3. Large count with constant element size */
    printf("3. Testing large count with constant element size:\n");
    test_large_count_const_size();
    printf("\n");
    
    /* 4. Mixed contexts for different target classifications */
    printf("4. Testing mixed contexts:\n");
    test_mixed_contexts();
    printf("\n");
    
    /* 5. Nested aggregates */
    printf("5. Testing nested aggregates:\n");
    test_nested_aggregates();
    printf("\n");
    
    /* 6. Edge cases */
    printf("6. Testing edge cases:\n");
    test_edge_cases();
    printf("\n");
    
    printf("All tests completed.\n");
    
    /* Use results to prevent dead code elimination */
    volatile int prevent_optimization = 0;
    if (prevent_optimization) {
        test_register_target();
        test_small_count();
        test_large_count_const_size();
        test_mixed_contexts();
        test_nested_aggregates();
        test_edge_cases();
    }
    
    return 0;
}
