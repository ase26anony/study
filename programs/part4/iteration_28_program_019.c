/* test_expr_coverage.c
 * Designed to cover constant bounds checking in GCC's expr.cc
 * Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_coverage.c -o test_expr
 * Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test_expr_coverage.c -o test_opt
 */

#include <stdio.h>
#include <stddef.h>

/* 1. Constant bounds using enum */
enum { L = 2, H = 5, BIG_START = 10, BIG_END = 90 };

/* 2. Small packed struct with constant size */
struct PackedSmall {
    int a:7;
    int b:9;
    int c:4;
} __attribute__((packed));

/* 3. Struct containing array */
struct WithArray {
    int header;
    int data[8];
    char tail;
};

/* 4. Multi-dimensional array type */
typedef int Matrix[4][6];

/* Global/static initializations (MEM_P(target) likely true) */

/* Large static array with wide range - triggers count > 2 path */
static int big_array[100] = { [BIG_START ... BIG_END] = 99 };

/* Packed struct array with constant range */
static struct PackedSmall packed_arr[10] = { [1 ... 3] = { .a = 1, .b = 2, .c = 3 } };

/* Multi-dimensional static array */
static Matrix md_static = { [0 ... 1][2 ... 3] = 7 };

/* Function to test various initialization contexts */
void test_initializations(void) {
    /* A. Register target with small count (count <= 2) */
    /* Force register target using small struct initialization */
    register struct PackedSmall reg_target = { .a = 1, .b = 2, .c = 3 };
    
    /* Compound literal assignment to register-like target */
    register int reg_arr[2] = { [0 ... 1] = 42 };  /* count = 2 */
    
    /* B. Automatic variables with constant bounds */
    /* Small range (count = 1) */
    int single[10] = { [5] = 100 };
    
    /* Medium range (count = 4) - should go through memory path */
    int auto_array[10] = { [L ... H] = 42 };
    
    /* C. Volatile target (ensures MEM_P) with count > 2 */
    volatile int volatile_arr[20] = { [3 ... 8] = 77 };
    
    /* D. Nested struct with array initialization */
    struct WithArray nested = { 
        .header = 1,
        .data = { [1 ... 3] = 7 },  /* count = 3 */
        .tail = 'x'
    };
    
    /* E. Multi-dimensional automatic array */
    Matrix md_auto = { [2 ... 3][0 ... 2] = 9 };
    
    /* F. Array of packed structs with constant range */
    struct PackedSmall local_packed[5] = { [0 ... 2] = { .a = 5, .b = 6, .c = 7 } };
    
    /* G. Using __builtin_constant_p to ensure constant evaluation */
    if (__builtin_constant_p(L) && __builtin_constant_p(H)) {
        int verified_const[10] = { [L ... H] = 123 };
        (void)verified_const; /* Use to avoid warning */
    }
    
    /* H. Conditional initialization with constant condition */
    if (1) {  /* Always true, but creates a scope */
        int scoped_array[8] = { [0 ... 2] = 88 };
        (void)scoped_array;
    }
    
    /* I. Switch case with initialization */
    switch (3) {
        case 3: {
            int case_array[6] = { [1 ... 4] = 33 };  /* count = 4 */
            (void)case_array;
            break;
        }
    }
    
    /* Use all variables to prevent dead code elimination */
    printf("reg_target: %d %d %d\n", reg_target.a, reg_target.b, reg_target.c);
    printf("reg_arr: %d %d\n", reg_arr[0], reg_arr[1]);
    printf("single[5]: %d\n", single[5]);
    printf("auto_array[%d]: %d\n", L, auto_array[L]);
    printf("volatile_arr[5]: %d\n", volatile_arr[5]);
    printf("nested.data[2]: %d\n", nested.data[2]);
    printf("md_auto[2][1]: %d\n", md_auto[2][1]);
    printf("local_packed[1].b: %d\n", local_packed[1].b);
    printf("big_array[50]: %d\n", big_array[50]);
    printf("packed_arr[2].c: %d\n", packed_arr[2].c);
    printf("md_static[0][2]: %d\n", md_static[0][2]);
}

/* Additional test with different element types */
void test_mixed_types(void) {
    /* Different element types with constant sizes */
    char char_arr[20] = { [5 ... 15] = 'A' };      /* 1-byte elements */
    short short_arr[15] = { [3 ... 8] = 256 };     /* 2-byte elements */
    long long_arr[12] = { [2 ... 6] = 1000L };     /* 8-byte elements (on 64-bit) */
    
    /* Array of pointers with constant range */
    const char *str_arr[10] = { [0 ... 4] = "test" };
    
    /* Struct array with designated initializers */
    struct Point { int x, y; } points[5] = { [0 ... 2] = { .x = 1, .y = 2 } };
    
    /* Use variables */
    printf("char_arr[10]: %c\n", char_arr[10]);
    printf("short_arr[5]: %d\n", short_arr[5]);
    printf("long_arr[4]: %ld\n", long_arr[4]);
    printf("str_arr[2]: %s\n", str_arr[2]);
    printf("points[1].x: %d\n", points[1].x);
}

/* Test with alignment attributes */
void test_aligned_targets(void) {
    /* Aligned array - might affect MEM_P classification */
    int aligned_arr[16] __attribute__((aligned(64))) = { [2 ... 7] = 512 };
    
    /* Packed and aligned struct */
    struct __attribute__((packed, aligned(2))) Mixed {
        char a;
        int b;
    } mixed_arr[8] = { [0 ... 3] = { .a = 'z', .b = 999 } };
    
    printf("aligned_arr[5]: %d\n", aligned_arr[5]);
    printf("mixed_arr[2].b: %d\n", mixed_arr[2].b);
}

int main(void) {
    printf("Testing constant bounds initialization coverage...\n");
    
    /* Test 1: Basic initializations */
    test_initializations();
    
    /* Test 2: Mixed types */
    test_mixed_types();
    
    /* Test 3: Aligned targets */
    test_aligned_targets();
    
    /* Additional direct tests in main */
    
    /* Very small count (1) with memory target */
    int tiny[100] = { [99] = 9999 };
    
    /* Exactly count = 2 with automatic variable */
    int exact_two[5] = { [1 ... 2] = 22 };
    
    /* Large count with static storage */
    static int huge_range[1000] = { [100 ... 899] = 1234 };
    
    printf("tiny[99]: %d\n", tiny[99]);
    printf("exact_two[1]: %d\n", exact_two[1]);
    printf("huge_range[500]: %d\n", huge_range[500]);
    
    return 0;
}
