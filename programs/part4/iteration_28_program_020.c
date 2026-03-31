/* test_expr_coverage.c
 * Designed to cover constant bounds checking in GCC's expr.cc
 * Compile with: gcc -std=gnu11 -O0 -fno-omit-frame-pointer test_expr_coverage.c -o test
 * Also test with: gcc -std=gnu11 -O2 -ftree-vectorize test_expr_coverage.c -o test_opt
 */

#include <stdio.h>
#include <stddef.h>

/* 1. Constant bounds using enum */
enum { L = 2, H = 5, BIG_L = 10, BIG_H = 90 };

/* 2. Small packed struct with constant size */
struct __attribute__((packed)) PackedStruct {
    int a:7;
    int b:9;
    int c:3;
};

/* 3. Struct containing array */
struct Container {
    int id;
    int data[8];
};

/* 4. Multi-dimensional array type */
typedef int Matrix[4][6];

/* Static initialization with constant bounds (MEM_P(target) likely true) */
static int static_array[100] = { [BIG_L ... BIG_H] = 99 };  /* count > 2, MEM_P */

/* Another static with small range */
static short small_static[10] = { [3 ... 4] = 255 };  /* count = 2 */

/* Packed struct static initialization */
static struct PackedStruct packed_static = { .a = 63, .b = 255, .c = 3 };

/* Function to force register target (!MEM_P) scenario */
static int init_to_register(void) {
    /* Use register keyword to hint at register allocation */
    register int reg_target = ({ 
        int temp = 0;
        /* Compound literal might create register target */
        temp = (int){ [0] = 1, [1] = 2 };  /* count = 2 */
        temp;
    });
    return reg_target;
}

/* Function with automatic variables in different contexts */
static void test_automatic_vars(void) {
    /* Automatic array with constant bounds - stack target (MEM_P) */
    int auto_array[20] = { [L ... H] = 42 };  /* count = 4 > 2, MEM_P */
    
    /* Small automatic array with count <= 2 */
    char tiny_array[5] = { [2] = 'A', [4] = 'B' };  /* Two separate ranges, each count=1 */
    
    /* Volatile ensures MEM_P target */
    volatile int volatile_array[10] = { [0 ... 2] = 999 };  /* count = 3 > 2, MEM_P */
    
    /* Multi-dimensional array with nested constant range */
    Matrix md = { [0 ... 1][2 ... 3] = 7 };  /* Complex constant bounds */
    
    /* Struct with array member initialization using constant range */
    struct Container cont = { 
        .id = 1,
        .data = { [1 ... 3] = 100 }  /* count = 3 > 2 */
    };
    
    /* Use all variables to prevent elimination */
    printf("auto_array[%d] = %d\n", L, auto_array[L]);
    printf("tiny_array[2] = %c\n", tiny_array[2]);
    printf("volatile_array[0] = %d\n", volatile_array[0]);
    printf("md[0][2] = %d\n", md[0][2]);
    printf("cont.data[2] = %d\n", cont.data[2]);
}

/* Function with conditional initialization */
static void test_conditional_init(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Different count scenarios based on constant condition */
        int arr1[10] = { [0 ... 1] = 10 };  /* count = 2 */
        printf("arr1[0] = %d\n", arr1[0]);
    } else {
        int arr2[10] = { [0 ... 4] = 20 };  /* count = 5 > 2 */
        printf("arr2[0] = %d\n", arr2[0]);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 1: {
            /* count = 1 scenario */
            int single[5] = { [3] = 100 };
            printf("single[3] = %d\n", single[3]);
            break;
        }
        case 2: {
            /* Mixed initialization */
            int mixed[10] = { 1, 2, 3, [7 ... 9] = 99 };
            printf("mixed[8] = %d\n", mixed[8]);
            break;
        }
    }
}

/* Test different element types with constant sizes */
static void test_element_types(void) {
    /* char - size = 1 */
    char char_array[50] = { [10 ... 40] = 'X' };  /* count = 31 > 2 */
    
    /* short - size = 2 (typically) */
    short short_array[30] = { [5 ... 15] = 32767 };  /* count = 11 > 2 */
    
    /* long long - size = 8 (typically) */
    long long ll_array[10] = { [2 ... 5] = 0xFFFFFFFFLL };  /* count = 4 > 2 */
    
    /* Array of packed structs */
    struct PackedStruct ps_array[5] = { [0 ... 2] = { 1, 2, 3 } };  /* count = 3 > 2 */
    
    printf("char_array[20] = %c\n", char_array[20]);
    printf("short_array[10] = %d\n", short_array[10]);
    printf("ll_array[3] = %lld\n", ll_array[3]);
    printf("ps_array[1].a = %d\n", ps_array[1].a);
}

int main(void) {
    printf("Testing constant bounds initialization coverage\n");
    
    /* 1. Register target scenario (!MEM_P) with count <= 2 */
    int reg_result = init_to_register();
    printf("Register init result: %d\n", reg_result);
    
    /* 2. Static array with large range (count > 2, MEM_P) */
    printf("static_array[%d] = %d\n", BIG_L, static_array[BIG_L]);
    printf("static_array[%d] = %d\n", BIG_H, static_array[BIG_H]);
    
    /* 3. Small static range (count = 2) */
    printf("small_static[3] = %d\n", small_static[3]);
    
    /* 4. Packed struct static */
    printf("packed_static.b = %d\n", packed_static.b);
    
    /* 5. Automatic variables in different contexts */
    test_automatic_vars();
    
    /* 6. Conditional initialization */
    test_conditional_init(1);
    test_conditional_init(2);
    
    /* 7. Different element types */
    test_element_types();
    
    /* Additional edge cases */
    
    /* Zero-length range? GCC might not allow, but single element */
    int single[10] = { [5] = 123 };  /* count effectively 1 */
    printf("single[5] = %d\n", single[5]);
    
    /* Using __builtin_constant_p to verify constant-ness */
    if (__builtin_constant_p(L) && __builtin_constant_p(H)) {
        int verified[10] = { [L ... H] = 777 };
        printf("Verified constant range: %d\n", verified[L]);
    }
    
    /* Array with alignment attribute */
    int __attribute__((aligned(32))) aligned_array[16] = { [0 ... 7] = 888 };
    printf("Aligned array[0] = %d (align: %zu)\n", 
           aligned_array[0], __alignof__(aligned_array));
    
    /* Nested block with another initialization */
    {
        /* This creates a new scope, potentially affecting target classification */
        register int inner_reg = (int){ [0] = 111 };
        printf("Inner register: %d\n", inner_reg);
    }
    
    return 0;
}
