/* Test program to cover constant bounds checking in GCC's expr.cc
 * Specifically targeting lines 7691-7700 for array/aggregate initialization
 * with constant bounds and varying target conditions.
 */

#include <stdio.h>
#include <stdint.h>

/* Use enum to ensure constant folding */
enum { L = 2, H = 5, SMALL_COUNT = 2 };
enum { BIG_LO = 10, BIG_HI = 90, BIG_COUNT = 81 };

/* 1. Register target with count <= 2 - should trigger !MEM_P(target) path */
static void test_register_target(void) {
    /* Small struct that fits in register */
    struct SmallReg {
        int a;
        int b;
    } __attribute__((packed));
    
    /* Use register keyword to encourage register allocation */
    register struct SmallReg reg_target = { .a = 1, .b = 2 };
    
    /* Designated initializer with constant range (count = 2) */
    int reg_array[10] = { [L ... H] = 42 };  /* count = 4, but array is memory */
    
    /* Force use to prevent elimination */
    printf("Register target: %d, %d\n", reg_target.a, reg_target.b);
    printf("Reg array[%d] = %d\n", L, reg_array[L]);
}

/* 2. Memory target with count <= 2 */
static void test_small_memory_target(void) {
    /* Exactly 2 elements in range */
    int small[10] = { [0 ... 1] = 99 };  /* count = 2 */
    
    /* Single element range */
    int single[10] = { [5] = 77 };  /* count = 1 */
    
    /* Volatile ensures MEM_P(target) */
    volatile int vol_small[5] = { [2 ... 3] = 88 };  /* count = 2 */
    
    printf("Small[0] = %d, Single[5] = %d, Vol[2] = %d\n", 
           small[0], single[5], vol_small[2]);
}

/* 3. Large memory target with count > 2 and constant element size */
static void test_large_memory_target(void) {
    /* Static storage ensures MEM_P(target) */
    static int big_array[100] = { [BIG_LO ... BIG_HI] = 123 };
    
    /* Element type with constant size that fits in unsigned HWI */
    struct PackedStruct {
        uint32_t a : 7;
        uint32_t b : 9;
        uint32_t c : 13;
    } __attribute__((packed));
    
    /* Array of packed structs with constant range */
    static struct PackedStruct packed_array[50] = 
        { [10 ... 40] = { .a = 1, .b = 2, .c = 3 } };
    
    /* Use attribute to affect alignment */
    int aligned_array[64] __attribute__((aligned(64))) = 
        { [16 ... 48] = 456 };
    
    printf("Big[%d] = %d, Packed[20].a = %d, Aligned[32] = %d\n",
           BIG_LO, big_array[BIG_LO], packed_array[20].a, aligned_array[32]);
}

/* 4. Multi-dimensional array with constant nested ranges */
static void test_multi_dimensional(void) {
    /* GCC extended designated initializers for multi-dimensional arrays */
    int md_array[5][6] = { [0 ... 2][1 ... 4] = 999 };
    
    /* Struct containing array with constant range */
    struct Container {
        int id;
        int data[8];
        struct {
            int x;
            int y;
        } point;
    };
    
    struct Container container = {
        .id = 1,
        .data = { [2 ... 5] = 777 },
        .point = { .x = 10, .y = 20 }
    };
    
    /* Compound literal with designated initializer */
    int *ptr = (int[10]){ [3 ... 7] = 333 };
    
    printf("MD[1][2] = %d, Container.data[3] = %d, ptr[4] = %d\n",
           md_array[1][2], container.data[3], ptr[4]);
}

/* 5. Mixed contexts with conditional compilation */
static void test_mixed_contexts(int selector) {
    /* Automatic variable inside function - stack based */
    if (selector > 0) {
        int auto_array[20] = { [5 ... 15] = 555 };
        printf("Auto[10] = %d\n", auto_array[10]);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 1: {
            /* Nested block with initialization */
            const int local_lo = 3, local_hi = 8;
            int local_array[10] = { [local_lo ... local_hi] = 222 };
            printf("Case 1 local[5] = %d\n", local_array[5]);
            break;
        }
        case 2: {
            /* Different element type */
            short short_array[30] = { [10 ... 25] = 32767 };
            printf("Case 2 short[20] = %d\n", short_array[20]);
            break;
        }
        default: {
            /* Bit-field struct array */
            struct BitField {
                unsigned int a : 3;
                unsigned int b : 5;
                unsigned int c : 12;
            } __attribute__((packed)) bf_array[10] = 
                { [0 ... 9] = { .a = 1, .b = 2, .c = 3 } };
            printf("Default bf[5].c = %d\n", bf_array[5].c);
        }
    }
}

/* 6. Test with __builtin_constant_p to verify constant folding */
static void test_constant_verification(void) {
    enum { VERIFY_LO = 1, VERIFY_HI = 3 };
    
    /* These should all be constant at compile time */
    int verify_array[5] = { [VERIFY_LO ... VERIFY_HI] = 888 };
    
    /* Use __builtin_constant_p to assert constant-ness */
    if (__builtin_constant_p(VERIFY_LO) && 
        __builtin_constant_p(VERIFY_HI)) {
        printf("Constant verification passed: array[2] = %d\n", verify_array[2]);
    }
}

int main(void) {
    printf("=== Testing constant bounds initialization paths ===\n");
    
    /* 1. Register target scenario */
    test_register_target();
    
    /* 2. Small count memory targets */
    test_small_memory_target();
    
    /* 3. Large count memory targets */
    test_large_memory_target();
    
    /* 4. Multi-dimensional and nested aggregates */
    test_multi_dimensional();
    
    /* 5. Mixed contexts with conditionals */
    test_mixed_contexts(1);
    test_mixed_contexts(2);
    test_mixed_contexts(3);
    
    /* 6. Constant verification */
    test_constant_verification();
    
    printf("=== All tests completed ===\n");
    
    return 0;
}
