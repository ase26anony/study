/* test_expr_coverage.c
 * Designed to cover constant bounds checking in GCC's expr.cc
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

/* Global/static initializations (MEM_P(target) likely true) */

/* Large array with wide constant range - triggers count > 2 path */
static int big_array[100] = { [10 ... 90] = 99, [0] = 1, [99] = 100 };

/* Multi-dimensional array with constant nested range */
static int md_array[5][6] = { [1 ... 3][2 ... 4] = 7 };

/* Struct with array member using constant range */
static struct Container global_cont = { 
    .id = 1,
    .data = { [1 ... 3] = 42, [5 ... 7] = 43 },
    .ps = { .a = 63, .b = 255, .c = 32767 }
};

/* Function to force register-target initialization */
static void test_register_target(void) {
    /* Try to force register target with small struct */
    register struct PackedStruct reg_target = { 
        .a = 31, 
        .b = 127, 
        .c = 16383 
    };
    
    /* Use designated initializer with constant range for scalar */
    register int reg_arr[4] = { [0 ... 1] = 10, [2 ... 3] = 20 };
    
    /* Use the values to prevent optimization */
    printf("Register target: a=%d, b=%d, c=%d\n", 
           reg_target.a, reg_target.b, reg_target.c);
    printf("Register array: %d %d %d %d\n", 
           reg_arr[0], reg_arr[1], reg_arr[2], reg_arr[3]);
}

/* Function with automatic variables and volatile */
static void test_memory_targets(void) {
    /* Automatic array with constant range - may be register or stack */
    int auto_array[10] = { [L ... H] = 42 };
    
    /* Volatile ensures MEM_P(target) is true */
    volatile int volatile_array[8] = { [0 ... 3] = 1, [4 ... 7] = 2 };
    
    /* Small count (<= 2) with memory target */
    int small_range[10] = { [5] = 100 };  /* count = 1 */
    
    /* Another small count example */
    int two_elem[10] = { [2 ... 3] = 200 };  /* count = 2 */
    
    /* Use values */
    printf("Auto array[%d]=%d\n", L, auto_array[L]);
    printf("Volatile array[0]=%d\n", volatile_array[0]);
    printf("Small range[5]=%d\n", small_range[5]);
    printf("Two elem[2]=%d\n", two_elem[2]);
}

/* Function with conditional initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Array with constant bounds in conditional block */
        int cond_array[20] = { [5 ... 15] = selector };
        printf("Cond array[10]=%d\n", cond_array[10]);
    } else {
        /* Different initialization pattern */
        int alt_array[8] = { [0 ... 7] = -selector };
        printf("Alt array[4]=%d\n", alt_array[4]);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 1: {
            /* Struct with packed bitfields and array */
            struct PackedStruct ps_local = { .a = 3, .b = 7, .c = 15 };
            int switch_array[6] = { [1 ... 4] = 999 };
            printf("Switch case 1: ps.a=%d, array[2]=%d\n", 
                   ps_local.a, switch_array[2]);
            break;
        }
        case 2: {
            /* Multi-dimensional in switch */
            int local_md[3][4] = { [0 ... 1][1 ... 2] = 777 };
            printf("Switch case 2: md[0][1]=%d\n", local_md[0][1]);
            break;
        }
    }
}

/* Test compound literals */
static void test_compound_literals(void) {
    /* Compound literal assignment - creates initialization context */
    struct Container *cont_ptr = &(struct Container){
        .id = 100,
        .data = { [2 ... 5] = 55 },
        .ps = { .a = 1, .b = 2, .c = 3 }
    };
    
    /* Array compound literal */
    int *arr_ptr = (int[8]){ [0 ... 3] = 10, [4 ... 7] = 20 };
    
    printf("Compound literal: id=%d, data[3]=%d\n", 
           cont_ptr->id, cont_ptr->data[3]);
    printf("Array literal: [0]=%d, [5]=%d\n", arr_ptr[0], arr_ptr[5]);
}

/* Test different element types with constant sizes */
static void test_different_types(void) {
    /* char type - small element size */
    char char_array[50] = { [10 ... 40] = 'A' };
    
    /* short type */
    short short_array[30] = { [5 ... 25] = 32000 };
    
    /* long long type */
    long long ll_array[10] = { [2 ... 8] = 0xFFFFFFFFLL };
    
    /* Mixed in struct */
    struct Mixed {
        char c_arr[5];
        short s_arr[4];
        int i_arr[3];
    } mixed = {
        .c_arr = { [0 ... 4] = 'X' },
        .s_arr = { [1 ... 3] = 1234 },
        .i_arr = { [0 ... 2] = -1 }
    };
    
    printf("Char array[20]=%c\n", char_array[20]);
    printf("Short array[10]=%d\n", short_array[10]);
    printf("Long long array[5]=%lld\n", ll_array[5]);
    printf("Mixed: c_arr[2]=%c, s_arr[2]=%d, i_arr[1]=%d\n",
           mixed.c_arr[2], mixed.s_arr[2], mixed.i_arr[1]);
}

int main(void) {
    printf("=== Testing constant bounds initialization coverage ===\n\n");
    
    /* 1. Register target with small count (count <= 2) */
    printf("1. Testing register target with small count:\n");
    test_register_target();
    printf("\n");
    
    /* 2. Memory targets with various count values */
    printf("2. Testing memory targets:\n");
    test_memory_targets();
    printf("\n");
    
    /* 3. Global/static arrays already initialized */
    printf("3. Global/static arrays:\n");
    printf("   big_array[10]=%d, big_array[50]=%d\n", 
           big_array[10], big_array[50]);
    printf("   md_array[2][3]=%d\n", md_array[2][3]);
    printf("   global_cont.data[2]=%d\n", global_cont.data[2]);
    printf("\n");
    
    /* 4. Conditional initialization */
    printf("4. Conditional initialization:\n");
    test_conditional_init(1);
    test_conditional_init(2);
    printf("\n");
    
    /* 5. Compound literals */
    printf("5. Compound literals:\n");
    test_compound_literals();
    printf("\n");
    
    /* 6. Different element types */
    printf("6. Different element types:\n");
    test_different_types();
    printf("\n");
    
    /* Final computation using all initialized values */
    int sum = big_array[10] + md_array[1][2] + global_cont.data[1];
    printf("Final sum (anti-optimization): %d\n", sum);
    
    return 0;
}
