/* Test program for GCC expr.cc constant bounds checking logic */
/* Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test.c -o test */
/* Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test.c -o test_opt */

#include <stdio.h>
#include <stdint.h>

/* Use enum to define constant bounds */
enum { L = 2, H = 5, SMALL_COUNT = 2 };

/* 1. Register target with small count (count <= 2) */
/* This should trigger !MEM_P(target) path */
static void test_register_target(void)
{
    /* Small struct that fits in register */
    struct SmallReg {
        int a;
        int b;
    } __attribute__((packed));
    
    /* Use register variable with constant initialization */
    register struct SmallReg reg_target = { .a = 1, .b = 2 };
    
    /* Designated initializer with constant range of 2 elements */
    int arr1[10] = { [0] = 100, [1] = 200 };  /* count = 2 */
    
    printf("Register test: %d %d\n", reg_target.a, arr1[0]);
}

/* 2. Memory target with count <= 2 */
static void test_small_memory_target(void)
{
    /* Exactly 2 elements in range */
    volatile int small_range[10] = { [3] = 30, [4] = 40 };  /* count = 2, MEM_P true */
    
    /* Single element range */
    static int single_elem[100] = { [50] = 999 };  /* count = 1, static MEM_P */
    
    printf("Small memory: %d %d\n", small_range[3], single_elem[50]);
}

/* 3. Large memory target with count > 2 and constant element size */
/* This should trigger the third condition path */
static void test_large_memory_target(void)
{
    /* Large array with wide constant range */
    static int big_array[1000] = { 
        [100 ... 900] = 0xABCD,  /* count = 801 > 2 */
    };
    
    /* Different element types with constant sizes */
    static char char_array[500] = { [100 ... 200] = 'X' };  /* count = 101 */
    static short short_array[300] = { [50 ... 150] = 1234 }; /* count = 101 */
    
    /* Packed struct with constant bitfield sizes */
    struct PackedBits {
        unsigned int a : 7;
        unsigned int b : 9;
        unsigned int c : 3;
    } __attribute__((packed));
    
    static struct PackedBits bit_structs[10] = { 
        [0 ... 5] = { .a = 127, .b = 511, .c = 7 }  /* count = 6 */
    };
    
    printf("Large memory: %d %c %d\n", 
           big_array[500], char_array[150], short_array[100]);
}

/* 4. Multi-dimensional arrays with constant ranges */
static void test_multi_dimensional(void)
{
    /* 2D array with constant range in both dimensions */
    int md1[10][10] = { 
        [2 ... 5][3 ... 7] = 42  /* Nested constant ranges */
    };
    
    /* 3D array */
    int md2[5][5][5] = {
        [1 ... 3][0 ... 2][4] = 99
    };
    
    printf("Multi-dim: %d %d\n", md1[3][5], md2[2][1][4]);
}

/* 5. Struct containing arrays with designated initializers */
static void test_nested_structs(void)
{
    struct Container {
        int id;
        int data[8];
        struct {
            int x;
            int y;
        } point;
    };
    
    /* Initialize with constant ranges inside struct */
    struct Container c1 = {
        .id = 1,
        .data = { [1 ... 4] = 255 },  /* count = 4 */
        .point = { .x = 10, .y = 20 }
    };
    
    /* Array of structs with range initialization */
    struct Container c_array[5] = {
        [0 ... 2] = { 
            .id = 100,
            .data = { [0 ... 3] = 50 },
            .point = { .x = 1, .y = 2 }
        }
    };
    
    printf("Nested struct: %d %d\n", c1.data[2], c_array[1].id);
}

/* 6. Compound literals as targets */
static void test_compound_literals(void)
{
    /* Compound literal assignment - creates initialization context */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    struct Point *ptr = &(struct Point){ 
        .x = 1, 
        .y = 2, 
        .z = 3 
    };
    
    /* Array compound literal with range */
    int *arr_ptr = (int[20]){ [5 ... 15] = 777 };
    
    printf("Compound: %d %d\n", ptr->x, arr_ptr[10]);
}

/* 7. Mixed contexts with conditional compilation */
static void test_mixed_contexts(void)
{
    /* Automatic variable in different scopes */
    {
        /* This block creates a new scope for the initialization */
        int auto_array[50] = { [10 ... 20] = 333 };  /* count = 11 */
        printf("Auto scope: %d\n", auto_array[15]);
    }
    
    /* Conditional with constant condition */
    if (1) {  /* Always true, but creates control flow */
        static int cond_array[30] = { [5 ... 10] = 444 };  /* count = 6 */
        printf("Conditional: %d\n", cond_array[7]);
    }
    
    /* Switch with constant case */
    switch (3) {
        case 3: {
            int switch_array[10] = { [2 ... 5] = 555 };  /* count = 4 */
            printf("Switch: %d\n", switch_array[3]);
            break;
        }
    }
}

/* 8. Aligned arrays affecting MEM_P classification */
static void test_aligned_targets(void)
{
    /* Aligned array might affect how target is classified */
    int aligned_arr[64] __attribute__((aligned(64))) = { 
        [16 ... 48] = 0xDEADBEEF  /* count = 33 */
    };
    
    printf("Aligned: %d\n", aligned_arr[32]);
}

/* 9. Constant expressions using sizeof */
static void test_sizeof_constants(void)
{
    /* Use sizeof in bounds - still constant */
    int size_based[100] = { 
        [0 ... (sizeof(int[10])/sizeof(int) - 1)] = 888  /* count = 10 */
    };
    
    printf("Sizeof: %d\n", size_based[5]);
}

int main(void)
{
    printf("Testing constant bounds initialization paths in expr.cc\n\n");
    
    /* Execute tests in sequence to cover different paths */
    test_register_target();        /* !MEM_P(target) path */
    test_small_memory_target();    /* count <= 2 path with MEM_P */
    test_large_memory_target();    /* count > 2, MEM_P, constant size */
    test_multi_dimensional();      /* Nested constant ranges */
    test_nested_structs();         /* Struct with array ranges */
    test_compound_literals();      /* Compound literal targets */
    test_mixed_contexts();         /* Various contexts */
    test_aligned_targets();        /* Aligned memory targets */
    test_sizeof_constants();       /* sizeof in constant bounds */
    
    printf("\nAll tests completed.\n");
    
    return 0;
}
