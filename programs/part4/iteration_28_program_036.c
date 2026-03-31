/* Test program to cover constant bounds checking in GCC's expr.cc
 * Specifically targeting lines 7691-7700 related to array/aggregate
 * initialization with constant bounds.
 */

#include <stdio.h>
#include <stdint.h>

/* ====== 1. Constant bounds definitions ====== */
enum { 
    LOWER = 2, 
    UPPER = 5,
    SMALL_COUNT = 2,
    LARGE_LOWER = 10,
    LARGE_UPPER = 90
};

/* ====== 2. Small packed struct with constant size ====== */
struct __attribute__((packed)) PackedStruct {
    int a:7;
    int b:9;
    int c:16;
};

/* ====== 3. Struct containing array ====== */
struct WithArray {
    int header;
    int data[8];
    struct PackedStruct ps;
};

/* ====== 4. Static initialization (MEM_P(target) likely true) ====== */
/* Large array with wide range - triggers count > 2 path */
static int global_large[100] = { [LARGE_LOWER ... LARGE_UPPER] = 99 };

/* Multi-dimensional with constant range */
static int md_global[3][4] = { [0 ... 1][2 ... 3] = 5 };

/* ====== 5. Function to create various initialization contexts ====== */
void test_initializations(void) {
    /* ----- Path 1: !MEM_P(target) ----- */
    /* Small struct that might go in register */
    register struct PackedStruct reg_target = { 
        .a = 1,
        .b = 2,
        .c = 3
    };
    
    /* Designated initializer with constant range on register target */
    struct PackedStruct reg_init = { 
        .a = 1,
        .b = 2,
        .c = 3 
    };
    
    /* ----- Path 2: count <= 2 (regardless of MEM_P) ----- */
    /* Exactly 2 elements in range */
    int small_range[10] = { [0 ... 1] = 42 };
    
    /* Single element range */
    int single_range[10] = { [5] = 99 };
    
    /* ----- Path 3: count > 2, MEM_P(target) true ----- */
    /* Automatic array with wide range */
    int auto_large[50] = { [10 ... 40] = 77 };
    
    /* Volatile ensures memory operand */
    volatile int volatile_arr[20] = { [5 ... 15] = 123 };
    
    /* ----- Path 4: Nested aggregates ----- */
    struct WithArray nested = {
        .header = 1,
        .data = { [1 ... 3] = 7 },
        .ps = { .a = 3, .b = 5, .c = 9 }
    };
    
    /* ----- Path 5: Multi-dimensional in automatic ----- */
    int md_local[3][4] = { [0 ... 1][2 ... 3] = 5 };
    
    /* ----- Path 6: Compound literal ----- */
    struct PackedStruct *mem = &(struct PackedStruct){ 
        .a = 1, 
        .b = 2, 
        .c = 3 
    };
    
    /* ----- Path 7: Mixed with conditional ----- */
    if (1) {  /* Constant condition */
        /* Array in nested block */
        int block_array[30] = { [LOWER ... UPPER] = 42 };
        
        /* Use volatile to force memory access */
        volatile int block_volatile[10] = { [2 ... 7] = 88 };
    }
    
    /* ----- Path 8: Different element types ----- */
    char char_array[100] = { [10 ... 90] = 'A' };
    short short_array[100] = { [20 ... 80] = 1234 };
    long long_array[50] = { [5 ... 45] = 999999L };
    
    /* ====== 6. Prevent dead code elimination ====== */
    int sum = 0;
    
    sum += reg_target.a + reg_target.b;
    sum += small_range[0] + small_range[1];
    sum += single_range[5];
    sum += auto_large[20];
    sum += volatile_arr[10];
    sum += nested.data[2];
    sum += md_local[0][2];
    sum += char_array[50];
    sum += short_array[50];
    sum += long_array[25];
    sum += global_large[50];
    sum += md_global[0][2];
    
    printf("Sum: %d\n", sum);
}

/* ====== 7. Main function with varied contexts ====== */
int main(void) {
    /* Test 1: Register target with constant range */
    {
        struct PackedStruct local_reg = { 
            .a = 1,
            .b = 2,
            .c = 3
        };
        printf("Local reg struct: %d\n", local_reg.a);
    }
    
    /* Test 2: Array with exactly 2-element range */
    {
        int two_elem[10] = { [3 ... 4] = 55 };
        printf("Two elem sum: %d\n", two_elem[3] + two_elem[4]);
    }
    
    /* Test 3: Large range in automatic variable */
    {
        int big_local[200] = { [50 ... 150] = 123 };
        printf("Big local[100]: %d\n", big_local[100]);
    }
    
    /* Test 4: Struct with array member using range */
    {
        struct WithArray sa = {
            .data = { [0 ... 7] = 1 }
        };
        printf("Struct array sum: %d\n", sa.data[0] + sa.data[7]);
    }
    
    /* Test 5: Multi-dimensional with complex range */
    {
        int md_complex[5][5] = { [1 ... 3][2 ... 4] = 9 };
        printf("MD complex: %d\n", md_complex[2][3]);
    }
    
    /* Test 6: Aligned array (might affect MEM_P classification) */
    {
        int __attribute__((aligned(32))) aligned_arr[64] = { 
            [16 ... 48] = 321 
        };
        printf("Aligned array: %d\n", aligned_arr[32]);
    }
    
    /* Run the comprehensive test */
    test_initializations();
    
    return 0;
}
