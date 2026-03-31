/* test_expr_coverage.c
 * Designed to trigger constant bounds checking logic in GCC's expr.cc
 * Specifically targets lines 7691-7700 related to array/aggregate initialization
 */

#include <stdio.h>
#include <stdint.h>

/* Use enum to ensure constant folding */
enum { 
    L = 2, 
    H = 5,
    BIG_START = 10,
    BIG_END = 90
};

/* Small packed struct with constant bitfield size */
struct __attribute__((packed)) PackedStruct {
    int a:7;
    int b:9;
    int c:3;
};

/* Struct containing array for nested initialization */
struct Container {
    int id;
    int data[8];
    struct PackedStruct ps;
};

/* Static initialization - will be MEM_P(target) */
static int static_array[100] = { 
    [BIG_START ... BIG_END] = 99,  /* count > 2, constant bounds */
    [0] = 1,
    [99] = 100
};

/* Multi-dimensional array with constant range */
static int md_array[4][5] = {
    [0 ... 2][1 ... 3] = 7,  /* Nested constant range */
    [3][4] = 42
};

/* Function to test register target initialization (!MEM_P(target)) */
static void test_register_target(void) {
    /* Try to force register target with small struct */
    register struct PackedStruct reg_target = { 
        .a = 3, 
        .b = 127, 
        .c = 2 
    };
    
    /* Small array initialization with count <= 2 */
    register int reg_arr[2] = { [0] = 1, [1] = 2 };  /* count = 2 */
    
    /* Use volatile to prevent optimization */
    volatile int use1 = reg_target.a + reg_target.b;
    volatile int use2 = reg_arr[0] + reg_arr[1];
    (void)use1; (void)use2;
}

/* Function to test automatic variable initialization */
static void test_automatic_vars(void) {
    /* Automatic array with constant range - may be MEM_P or not */
    int auto_array[10] = { [L ... H] = 42 };  /* count = 4 > 2 */
    
    /* Volatile array to ensure MEM_P target */
    volatile int volatile_array[20] = { [5 ... 15] = 77 };  /* count = 11 > 2 */
    
    /* Struct with array member using designated initializer */
    struct Container cont = {
        .id = 1,
        .data = { [2 ... 6] = 99 },  /* count = 5 > 2 */
        .ps = { .a = 1, .b = 2, .c = 1 }
    };
    
    /* Compound literal assignment - creates initialization context */
    struct Container *ptr = &cont;
    *ptr = (struct Container){ 
        .data = { [1 ... 3] = 55 },  /* count = 3 > 2 */
        .id = 2
    };
    
    /* Use values to prevent dead code elimination */
    printf("auto_array[%d] = %d\n", L, auto_array[L]);
    printf("volatile_array[10] = %d\n", volatile_array[10]);
    printf("cont.data[3] = %d\n", cont.data[3]);
}

/* Function with conditional initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Array with exactly 1 element range (count = 1) */
        int single_range[10] = { [5] = 123 };  /* count = 1 <= 2 */
        
        /* Array with 2 element range (count = 2) */
        int double_range[10] = { [3 ... 4] = 456 };  /* count = 2 <= 2 */
        
        printf("single_range[5] = %d\n", single_range[5]);
        printf("double_range[3] = %d\n", double_range[3]);
    } else {
        /* Different initialization path */
        int alt_array[8] = { [0 ... 7] = 999 };  /* count = 8 > 2 */
        printf("alt_array[0] = %d\n", alt_array[0]);
    }
}

/* Test different element types with constant sizes */
static void test_element_types(void) {
    /* char - size = 1, fits in uhwi */
    char char_array[50] = { [10 ... 40] = 'A' };  /* count = 31 > 2 */
    
    /* short - size = 2 (typically), fits in uhwi */
    short short_array[30] = { [5 ... 25] = 32000 };  /* count = 21 > 2 */
    
    /* long long - size = 8, fits in uhwi */
    long long ll_array[20] = { [2 ... 15] = 0x123456789ABCDEFLL };  /* count = 14 > 2 */
    
    /* Packed struct array */
    struct PackedStruct ps_array[10] = { 
        [0 ... 9] = { .a = 1, .b = 2, .c = 1 }  /* count = 10 > 2 */
    };
    
    /* Use values */
    printf("char_array[20] = %c\n", char_array[20]);
    printf("short_array[10] = %d\n", short_array[10]);
    printf("ll_array[5] = %llx\n", ll_array[5]);
    printf("ps_array[3].b = %d\n", ps_array[3].b);
}

/* Main function orchestrating all tests */
int main(void) {
    printf("Testing constant bounds initialization paths in expr.cc\n\n");
    
    /* 1. Register target with count <= 2 */
    printf("1. Testing register target with count <= 2:\n");
    test_register_target();
    
    /* 2. Static initialization (MEM_P, count > 2) */
    printf("\n2. Testing static array (MEM_P, count > 2):\n");
    printf("static_array[%d] = %d\n", BIG_START, static_array[BIG_START]);
    printf("static_array[%d] = %d\n", BIG_END, static_array[BIG_END]);
    
    /* 3. Multi-dimensional array */
    printf("\n3. Testing multi-dimensional array:\n");
    printf("md_array[1][2] = %d\n", md_array[1][2]);
    printf("md_array[3][4] = %d\n", md_array[3][4]);
    
    /* 4. Automatic variables */
    printf("\n4. Testing automatic variables:\n");
    test_automatic_vars();
    
    /* 5. Conditional initialization */
    printf("\n5. Testing conditional initialization:\n");
    test_conditional_init(1);  /* count <= 2 path */
    test_conditional_init(0);  /* count > 2 path */
    
    /* 6. Different element types */
    printf("\n6. Testing different element types:\n");
    test_element_types();
    
    /* Additional test: Zero-length range (count = 0?) */
    printf("\n7. Testing edge cases:\n");
    int empty_range[10] = { [5 ... 4] = 999 };  /* Should produce count = 0 */
    (void)empty_range;
    
    /* Test with __builtin_constant_p to verify constant-ness */
    if (__builtin_constant_p(L) && __builtin_constant_p(H)) {
        printf("Bounds are compile-time constants (as expected)\n");
    }
    
    return 0;
}
