/* Test program for GCC expr.cc constant bounds initialization coverage */
#include <stdio.h>
#include <stddef.h>

/* Use enum to ensure constant folding */
enum { 
    LO = 2, 
    HI = 5,
    BIG_LO = 10,
    BIG_HI = 90,
    SMALL_COUNT = 2
};

/* Small packed struct with constant size */
struct __attribute__((packed)) PackedSmall {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 4;
}; /* Total: 20 bits = 3 bytes padded */

/* Struct with array member */
struct WithArray {
    int header;
    int data[8];
    struct PackedSmall ps;
};

/* 1. Register target with count <= 2 */
static void test_register_target(void) {
    /* Force register storage class for small struct */
    register struct PackedSmall reg_target = { 
        .a = 1, 
        .b = 2, 
        .c = 3 
    };
    
    /* Designated initializer with exactly 2 elements (count <= 2) */
    register int reg_arr[5] = { [0] = 100, [1] = 200 };
    
    /* Use values to prevent elimination */
    printf("Register target: a=%u, b=%u, c=%u\n", 
           reg_target.a, reg_target.b, reg_target.c);
    printf("Register array[0]=%d, [1]=%d\n", reg_arr[0], reg_arr[1]);
}

/* 2. Memory target with count > 2 and constant element size */
static void test_memory_target_large_count(void) {
    /* Static storage ensures MEM_P(target) == true */
    static int big_array[100] = { 
        [BIG_LO ... BIG_HI] = 99  /* count = 81 > 2 */
    };
    
    /* Another with char type (size = 1 byte) */
    static char char_array[256] = {
        [32 ... 127] = 'A'  /* count = 96 > 2 */
    };
    
    /* Packed struct array with count > 2 */
    static struct PackedSmall ps_array[10] = {
        [1 ... 4] = { .a = 7, .b = 511, .c = 15 }  /* count = 4 > 2 */
    };
    
    printf("Big array[%d]=%d, [%d]=%d\n", 
           BIG_LO, big_array[BIG_LO], BIG_HI, big_array[BIG_HI]);
    printf("Char array[32]=%c, [127]=%c\n", 
           char_array[32], char_array[127]);
    printf("Packed array[1].b=%u, [4].c=%u\n", 
           ps_array[1].b, ps_array[4].c);
}

/* 3. Automatic variable with constant bounds */
static void test_automatic_vars(void) {
    /* Automatic array - might be register promoted */
    int auto_array[20] = {
        [LO ... HI] = 42  /* count = 4 > 2 */
    };
    
    /* Volatile ensures MEM_P(target) == true */
    volatile int volatile_array[10] = {
        [3 ... 7] = -1  /* count = 5 > 2 */
    };
    
    /* Small count with automatic */
    int small_auto[5] = {
        [4] = 999  /* count = 1 <= 2 */
    };
    
    printf("Auto array[%d]=%d, [%d]=%d\n", 
           LO, auto_array[LO], HI, auto_array[HI]);
    printf("Volatile array[3]=%d, [7]=%d\n", 
           volatile_array[3], volatile_array[7]);
    printf("Small auto[4]=%d\n", small_auto[4]);
}

/* 4. Multi-dimensional array with nested constant ranges */
static void test_multi_dimensional(void) {
    int md_array[5][6] = {
        [0 ... 2][1 ... 3] = 77  /* 3x3 = 9 elements > 2 */
    };
    
    /* 3D array */
    int three_d[3][4][5] = {
        [0 ... 1][2 ... 3][1 ... 2] = 888
    };
    
    printf("2D array[0][1]=%d, [2][3]=%d\n", 
           md_array[0][1], md_array[2][3]);
    printf("3D array[0][2][1]=%d, [1][3][2]=%d\n", 
           three_d[0][2][1], three_d[1][3][2]);
}

/* 5. Struct with array member initialization */
static void test_struct_with_array(void) {
    struct WithArray s = {
        .header = 0xABCD,
        .data = { [1 ... 3] = 111 },  /* count = 3 > 2 */
        .ps = { .a = 3, .b = 127, .c = 7 }
    };
    
    /* Array of structs with designated range */
    struct WithArray s_array[5] = {
        [0 ... 2] = { 
            .header = 1,
            .data = { [0 ... 2] = 222 },
            .ps = { .a = 1, .b = 2, .c = 3 }
        }
    };
    
    printf("Struct header=0x%x, data[1]=%d\n", s.header, s.data[1]);
    printf("Struct array[0].data[0]=%d, [2].header=%d\n", 
           s_array[0].data[0], s_array[2].header);
}

/* 6. Compound literals (create initialization contexts) */
static void test_compound_literals(void) {
    /* Compound literal assignment */
    struct PackedSmall *ptr = &(struct PackedSmall){
        .a = 5, .b = 10, .c = 2
    };
    
    /* Array compound literal */
    int *arr_ptr = (int[8]){ [0 ... 3] = 55, [7] = 99 };
    
    printf("Compound literal: a=%u, b=%u\n", ptr->a, ptr->b);
    printf("Array literal[0]=%d, [7]=%d\n", arr_ptr[0], arr_ptr[7]);
}

/* 7. Conditional initialization with constant conditions */
static void test_conditional_init(void) {
    /* Always true condition ensures initialization is parsed */
    if (__builtin_constant_p(LO) && __builtin_constant_p(HI)) {
        int cond_array[10] = { [LO ... HI] = 333 };
        printf("Conditional array[%d]=%d\n", LO, cond_array[LO]);
    }
    
    switch (SMALL_COUNT) {
        case 2: {
            int switch_array[5] = { [0 ... 1] = 444 };  /* count = 2 */
            printf("Switch array[0]=%d\n", switch_array[0]);
            break;
        }
        default:
            break;
    }
}

/* 8. Mixed initializations with different element types */
static void test_mixed_types(void) {
    /* Different integer types with constant sizes */
    short short_array[50] = { [10 ... 20] = 32767 };
    long long_array[30] = { [5 ... 15] = 123456789LL };
    
    /* Mixed in union */
    union Mixed {
        int i;
        float f;
        char c[4];
    } u_array[5] = { [0 ... 2] = { .i = 0xDEADBEEF } };
    
    printf("Short array[10]=%d, [20]=%d\n", 
           short_array[10], short_array[20]);
    printf("Long array[5]=%ld, [15]=%ld\n", 
           long_array[5], long_array[15]);
    printf("Union array[0].i=0x%x\n", u_array[0].i);
}

int main(void) {
    printf("=== Testing constant bounds initialization ===\n\n");
    
    /* Execute tests in sequence to cover different paths */
    test_register_target();           /* !MEM_P(target) path */
    printf("\n");
    
    test_memory_target_large_count(); /* MEM_P(target), count > 2 */
    printf("\n");
    
    test_automatic_vars();            /* Mixed automatic vars */
    printf("\n");
    
    test_multi_dimensional();         /* Multi-dimensional arrays */
    printf("\n");
    
    test_struct_with_array();         /* Structs with array members */
    printf("\n");
    
    test_compound_literals();         /* Compound literals */
    printf("\n");
    
    test_conditional_init();          /* Conditional contexts */
    printf("\n");
    
    test_mixed_types();               /* Different element types */
    printf("\n");
    
    printf("=== All tests completed ===\n");
    
    return 0;
}
