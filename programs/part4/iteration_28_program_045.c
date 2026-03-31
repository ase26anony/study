/* test_expr_coverage.c
 * Designed to trigger constant bounds checking in GCC's expr.cc
 * Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -ftree-vectorize test_expr_coverage.c -o test_expr_coverage
 */

#include <stdio.h>
#include <stdint.h>

/* For count <= 2 path with !MEM_P(target) */
struct SmallStruct {
    int a;
    int b;
} __attribute__((packed));

/* For constant element size with packed bitfields */
struct PackedBits {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 16;
} __attribute__((packed));

/* For multi-dimensional array initialization */
struct NestedAggregate {
    int id;
    int values[5];
    struct PackedBits bits;
};

/* Static initialization - MEM_P(target) true, count > 2 */
static int static_array[100] = { [10 ... 90] = 99 };

/* Enum for constant bounds */
enum { LOWER = 2, UPPER = 5, WIDE_LOWER = 10, WIDE_UPPER = 90 };

/* Function to prevent dead code elimination */
__attribute__((noinline)) 
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(void) {
    printf("Testing constant bounds initialization paths in expr.cc\n");
    
    /* 1. Register target with count <= 2 - should trigger !MEM_P(target) path */
    {
        register struct SmallStruct reg_target = { 
            .a = 1,
            .b = 2 
        };
        /* Designated initializer with exactly 2 elements */
        register int reg_arr[2] = { [0] = 42, [1] = 43 };
        
        use_value(reg_target.a + reg_target.b);
        use_value(reg_arr[0] + reg_arr[1]);
    }
    
    /* 2. Memory target with count <= 2 */
    {
        /* Exactly 1 element range */
        int single_range[10] = { [5] = 100 };
        /* Exactly 2 elements using enum constants */
        int double_range[] = { [LOWER ... UPPER] = 7 }; /* count = 4, but let's do 2 */
        int explicit_two[10] = { [3] = 1, [4] = 2 }; /* Two separate designators */
        
        use_value(single_range[5]);
        use_value(double_range[LOWER]);
    }
    
    /* 3. Memory target with count > 2 and constant element size */
    {
        /* Large range in automatic array - MEM_P(target) true, count > 2 */
        int auto_large[50] = { [10 ... 40] = 255 };
        
        /* Packed struct array with constant size elements */
        struct PackedBits packed_array[20] = { [5 ... 15] = { .a = 1, .b = 2, .c = 3 } };
        
        /* Use volatile to ensure memory operand */
        volatile int volatile_array[30] = { [5 ... 25] = 999 };
        
        use_value(auto_large[20]);
        use_value(packed_array[10].c);
        use_value(volatile_array[15]);
    }
    
    /* 4. Multi-dimensional array with constant nested ranges */
    {
        int md[3][4] = { 
            [0 ... 1][2 ... 3] = 5,  /* 2x2 = 4 elements */
            [2][0 ... 1] = 7         /* 1x2 = 2 elements */
        };
        
        /* 3D array */
        int three_d[2][3][4] = { [0 ... 1][1 ... 2][0 ... 1] = 9 };
        
        use_value(md[0][2]);
        use_value(three_d[0][1][0]);
    }
    
    /* 5. Nested struct with array member initialization */
    {
        struct NestedAggregate nested = {
            .id = 1,
            .values = { [1 ... 3] = 42 },  /* 3 elements */
            .bits = { .a = 3, .b = 5, .c = 9 }
        };
        
        /* Compound literal assignment */
        struct NestedAggregate *ptr = &nested;
        *ptr = (struct NestedAggregate){ 
            .values = { [0 ... 4] = 99 }  /* 5 elements */
        };
        
        use_value(nested.values[2]);
        use_value(nested.bits.c);
    }
    
    /* 6. Mixed initializations with conditional compilation */
    #if 1  /* Always true, but creates constant context */
    {
        /* Array with size from constant expression */
        const int SIZE = 20;
        int sized_array[SIZE] = { [SIZE/2 ... SIZE-1] = 77 };
        
        /* Struct with bitfields and array */
        struct Mixed {
            unsigned char flags[4];
            int data[3];
            unsigned int bitfield : 4;
        } mixed = {
            .flags = { [0 ... 3] = 0xFF },
            .data = { [0 ... 2] = -1 },
            .bitfield = 7
        };
        
        use_value(sized_array[SIZE/2]);
        use_value(mixed.data[1]);
    }
    #endif
    
    /* 7. Test with different element types for TYPE_SIZE check */
    {
        /* char - size 1 */
        char char_array[100] = { [20 ... 80] = 'A' };
        
        /* short - size 2 */
        short short_array[50] = { [10 ... 40] = 32767 };
        
        /* long long - size 8 */
        long long ll_array[30] = { [5 ... 25] = 0x123456789ABCDEFLL };
        
        /* Mixed in union */
        union MixedTypes {
            char c[10];
            int i[5];
            double d[3];
        } u = { .i = { [0 ... 4] = 123 } };
        
        use_value(char_array[50]);
        use_value(short_array[25]);
        use_value((int)ll_array[10]);
        use_value(u.i[2]);
    }
    
    /* 8. Zero-length range (count = 1) edge case */
    {
        int single[10] = { [5 ... 5] = 42 };  /* hi = lo, count = 1 */
        use_value(single[5]);
    }
    
    /* Use static array to prevent optimization */
    printf("Static array element: %d\n", static_array[50]);
    
    /* Final computation to ensure all paths are used */
    int result = 0;
    for (int i = 0; i < 100; i++) {
        result += static_array[i];
    }
    
    printf("Result: %d\n", result);
    printf("All constant bounds initializations tested.\n");
    
    return 0;
}
