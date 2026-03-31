/* test_expr_coverage.c
 * Designed to cover constant bounds checking in GCC's expr.cc
 * Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test_expr_coverage.c -o test
 * Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test_expr_coverage.c -o test_opt
 */

#include <stdio.h>
#include <stdint.h>

/* 1. Constant bounds using enum */
enum { L = 2, H = 5, BIG_START = 10, BIG_END = 90 };

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
    struct PackedStruct ps;
};

/* 4. Static large array - will be MEM_P(target) with count > 2 */
static int big_array[100] = { [BIG_START ... BIG_END] = 99 };

/* 5. Multi-dimensional array */
static int md_array[4][5] = { [0 ... 2][1 ... 3] = 7 };

/* Function to use register target (!MEM_P(target)) */
static void test_register_target(void) {
    /* Force register storage class for small aggregate */
    register struct PackedStruct reg_target = { 
        .a = 1, 
        .b = 2, 
        .c = 3 
    };
    
    /* Designated initializer with constant range, count = 1 */
    register int reg_arr[10] = { [L] = 42 };
    
    /* Use volatile to prevent optimization */
    volatile int dummy = reg_target.a + reg_arr[L];
    (void)dummy;
}

/* Function with automatic variables */
static void test_automatic_vars(void) {
    /* Automatic array with constant range, count = 4 (H-L+1 = 4) */
    int auto_array[] = { [L ... H] = 255 };
    
    /* Volatile array - definitely MEM_P(target) */
    volatile int volatile_array[20] = { [5 ... 15] = 123 };
    
    /* Small count (2) with memory target */
    struct Container local_container = { 
        .id = 1,
        .data = { [2 ... 3] = 88 }  /* count = 2 */
    };
    
    /* Multi-dimensional automatic array */
    int local_md[3][4] = { [0 ... 1][2 ... 3] = 5 };
    
    /* Use values to prevent dead code elimination */
    printf("auto_array[%d] = %d\n", L, auto_array[L]);
    printf("volatile_array[10] = %d\n", volatile_array[10]);
    printf("local_container.data[2] = %d\n", local_container.data[2]);
    printf("local_md[0][2] = %d\n", local_md[0][2]);
}

/* Function with conditional initialization */
static void test_conditional_init(int selector) {
    if (selector > 0) {
        /* This should still be processed as constant bounds */
        int cond_array[10] = { [0 ... 4] = selector * 10 };
        printf("cond_array[0] = %d\n", cond_array[0]);
    } else {
        /* Different constant range */
        int cond_array[10] = { [5 ... 9] = 999 };
        printf("cond_array[5] = %d\n", cond_array[5]);
    }
}

/* Test compound literals */
static void test_compound_literals(void) {
    /* Compound literal assignment - creates initialization context */
    struct Container *ptr = &(struct Container){
        .id = 100,
        .data = { [1 ... 6] = 77 }  /* count = 6 > 2 */
    };
    
    /* Another with small count */
    struct PackedStruct ps = (struct PackedStruct){ 
        .a = 5, 
        .b = 10,
        .c = 2 
    };
    
    printf("ptr->id = %d\n", ptr->id);
    printf("ps.a = %d\n", ps.a);
}

/* Test various element types with constant sizes */
static void test_different_types(void) {
    /* char - size = 1 */
    char char_array[50] = { [10 ... 40] = 'X' };
    
    /* short - size = 2 */
    short short_array[30] = { [5 ... 25] = 32000 };
    
    /* long long - size = 8 */
    long long ll_array[20] = { [2 ... 15] = 0x123456789ABCDEFLL };
    
    /* Mixed struct */
    struct Mixed {
        char c;
        int i;
        short s;
    } __attribute__((packed));
    
    struct Mixed mixed_array[10] = { [0 ... 9] = { 'A', 42, 1000 } };
    
    printf("char_array[25] = %c\n", char_array[25]);
    printf("short_array[15] = %d\n", short_array[15]);
    printf("ll_array[10] = %llx\n", ll_array[10]);
    printf("mixed_array[5].c = %c\n", mixed_array[5].c);
}

/* Test bitfield arrays */
static void test_bitfield_arrays(void) {
    struct BitfieldElem {
        unsigned int a:5;
        unsigned int b:7;
        unsigned int c:4;
    } __attribute__((packed));
    
    struct BitfieldElem bf_array[8] = { 
        [0 ... 7] = { .a = 1, .b = 2, .c = 3 } 
    };
    
    /* Single element initialization */
    struct BitfieldElem bf_single = { [0 ... 0] = { .a = 31, .b = 127, .c = 15 } };
    
    printf("bf_array[3].a = %u\n", bf_array[3].a);
    printf("bf_single.b = %u\n", bf_single.b);
}

int main(void) {
    printf("Testing constant bounds initialization coverage...\n\n");
    
    /* 1. Register target - should trigger !MEM_P(target) path */
    printf("1. Testing register target:\n");
    test_register_target();
    
    /* 2. Static initialization - MEM_P(target), count > 2 */
    printf("\n2. Testing static arrays (MEM_P, count > 2):\n");
    printf("big_array[%d] = %d\n", BIG_START, big_array[BIG_START]);
    printf("big_array[%d] = %d\n", BIG_END, big_array[BIG_END]);
    printf("md_array[1][2] = %d\n", md_array[1][2]);
    
    /* 3. Automatic variables */
    printf("\n3. Testing automatic variables:\n");
    test_automatic_vars();
    
    /* 4. Conditional initialization */
    printf("\n4. Testing conditional initialization:\n");
    test_conditional_init(1);
    test_conditional_init(0);
    
    /* 5. Compound literals */
    printf("\n5. Testing compound literals:\n");
    test_compound_literals();
    
    /* 6. Different types */
    printf("\n6. Testing different element types:\n");
    test_different_types();
    
    /* 7. Bitfield arrays */
    printf("\n7. Testing bitfield arrays:\n");
    test_bitfield_arrays();
    
    /* Additional test: Nested struct with array */
    printf("\n8. Testing nested initialization:\n");
    struct Outer {
        int x;
        struct {
            int a[5];
            char b[3];
        } inner;
    } outer = { 
        .x = 100,
        .inner = {
            .a = { [1 ... 3] = 50 },  /* count = 3 > 2 */
            .b = { [0 ... 2] = 'Z' }   /* count = 3 > 2 */
        }
    };
    printf("outer.inner.a[2] = %d\n", outer.inner.a[2]);
    printf("outer.inner.b[1] = %c\n", outer.inner.b[1]);
    
    /* Test with exactly count = 2 */
    printf("\n9. Testing count == 2:\n");
    int exact_two[10] = { [3 ... 4] = 777 };
    printf("exact_two[3] = %d, exact_two[4] = %d\n", exact_two[3], exact_two[4]);
    
    /* Test with count = 1 */
    printf("\n10. Testing count == 1:\n");
    int single[5] = { [2] = 999 };
    printf("single[2] = %d\n", single[2]);
    
    return 0;
}
