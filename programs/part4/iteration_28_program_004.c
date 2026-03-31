/* test_expr_coverage.c
 * Designed to trigger constant bounds checking in GCC's expr.cc
 * Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -ftree-vectorize test_expr_coverage.c -o test_expr_coverage
 */

#include <stdio.h>
#include <stdint.h>

/* For count <= 2 path, with potential register target */
struct SmallStruct {
    int a;
    int b;
};

/* For constant element size with packed attribute */
struct __attribute__((packed)) PackedStruct {
    int a:7;
    int b:9;
    int c:16;
};

/* For multi-dimensional array initialization */
struct NestedStruct {
    int id;
    int values[5];
};

/* Static initialization - MEM_P(target) true, count > 2 */
static int static_array[100] = { [10 ... 90] = 99 };

/* Another static with different element type */
static char char_array[50] = { [5 ... 25] = 'A' };

int main(void) {
    int result = 0;
    
    /* 1. Register target scenario (!MEM_P(target)) with small struct */
    /* Using register keyword to hint at register allocation */
    register struct SmallStruct reg_target = { .a = 1, .b = 2 };
    
    /* Designated initializer with constant range of 2 elements */
    /* This should trigger count <= 2 path */
    int small_range[10] = { [2 ... 3] = 42 };
    result += small_range[2] + small_range[3];
    
    /* 2. Automatic array with constant range, count > 2, MEM_P(target) true */
    int auto_array[50] = { [5 ... 15] = 77 };
    for (int i = 5; i <= 15; i++) {
        result += auto_array[i];
    }
    
    /* 3. Volatile array - definitely MEM_P(target) true */
    volatile int volatile_array[20] = { [3 ... 8] = 123 };
    result += volatile_array[5];
    
    /* 4. Packed struct array - constant element size, count > 2 */
    struct PackedStruct packed_array[10] = { [1 ... 5] = { .a = 1, .b = 2, .c = 3 } };
    result += packed_array[2].c;
    
    /* 5. Multi-dimensional array with nested constant ranges */
    int md_array[4][5] = { [0 ... 2][1 ... 3] = 88 };
    result += md_array[1][2];
    
    /* 6. Using enum for constant bounds */
    enum { LOWER = 3, UPPER = 7 };
    int enum_array[10] = { [LOWER ... UPPER] = 33 };
    result += enum_array[5];
    
    /* 7. Compound literal assignment - creates initialization context */
    struct SmallStruct *ptr = &reg_target;
    *ptr = (struct SmallStruct){ .a = 10, .b = 20 };
    result += ptr->a;
    
    /* 8. Nested struct with array initialization */
    struct NestedStruct nested = { 
        .id = 1,
        .values = { [1 ... 3] = 100 }
    };
    result += nested.values[2];
    
    /* 9. Large range initialization - triggers the third condition branch */
    /* count > 2, MEM_P(target) true, constant element size */
    int large_local[200] = { [50 ... 150] = 999 };
    result += large_local[100];
    
    /* 10. Mixed initializers with gaps */
    int mixed[20] = { 
        [0] = 1,
        [5 ... 8] = 2,  /* count = 4 */
        [15] = 3
    };
    result += mixed[6];
    
    /* 11. Using const variable for bounds (should fold to constant) */
    const int start = 2;
    const int end = 5;
    int const_bounds_array[10] = { [start ... end] = 44 };
    result += const_bounds_array[3];
    
    /* 12. Bitfield struct array - unusual but constant size */
    struct BitField {
        unsigned int a:3;
        unsigned int b:5;
        unsigned int c:8;
    } __attribute__((packed));
    
    struct BitField bf_array[5] = { [0 ... 2] = { .a = 1, .b = 2, .c = 3 } };
    result += bf_array[1].c;
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    printf("Static array element: %d\n", static_array[50]);
    printf("Char array element: %c\n", char_array[10]);
    
    return result > 0 ? 0 : 1;
}

/* Additional static initializers outside main */
struct GlobalStruct {
    int data[10];
};

/* Global with designated range */
struct GlobalStruct global = { .data = { [2 ... 6] = 255 } };

/* Union with array to test different layouts */
union TestUnion {
    int as_int[4];
    char as_char[16];
};

union TestUnion test_union = { .as_int = { [0 ... 2] = 0xABCD } };
