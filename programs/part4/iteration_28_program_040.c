/* Test program for GCC expr.cc constant bounds checking */
#include <stdio.h>
#include <stdint.h>

/* Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test.c -o test */
/* Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test.c -o test_opt */

/* 1. Constant bounds definitions using enum */
enum { 
    LOWER = 2,
    UPPER = 5,
    SMALL_COUNT = 2,
    WIDE_START = 10,
    WIDE_END = 90
};

/* 2. Small packed struct with constant bitfield sizes */
struct __attribute__((packed)) PackedStruct {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 3;
    /* Total size: 19 bits, should fit in uhwi */
};

/* 3. Struct containing array */
struct WithArray {
    int header;
    int data[8];
    struct PackedStruct ps;
};

/* 4. Static initialization with wide range (count > 2, MEM_P(target) = true) */
static int static_array[100] = { 
    [WIDE_START ... WIDE_END] = 99,  /* count = 81 > 2 */
    [0] = 1,
    [99] = 100
};

/* 5. Multi-dimensional array with constant range */
static int md_array[4][6] = {
    [0 ... 2][1 ... 4] = 7,  /* 3x4 = 12 elements > 2 */
    [3][5] = 42
};

/* Helper function to prevent dead code elimination */
__attribute__((noinline)) 
static void use_value(int val) {
    volatile int sink = val;
    (void)sink;
}

int main(void) {
    printf("Testing constant bounds initialization paths\n");
    
    /* ===== PATH 1: Register target with !MEM_P(target) ===== */
    /* Small struct that might go in registers */
    register struct PackedStruct reg_target = {
        .a = 3,
        .b = 127,
        .c = 2
    };
    use_value(reg_target.a + reg_target.b);
    
    /* ===== PATH 2: count <= 2 (exactly 2 elements) ===== */
    /* Automatic array with exactly 2-element range */
    int small_range[10] = {
        [3 ... 4] = 42  /* count = 2 */
    };
    use_value(small_range[3] + small_range[4]);
    
    /* Single element range (count = 1) */
    int single_range[20] = {
        [15] = 100  /* count = 1 */
    };
    use_value(single_range[15]);
    
    /* ===== PATH 3: count > 2 with MEM_P(target) = true ===== */
    /* Automatic array with wide constant range */
    int auto_big[50] = {
        [10 ... 40] = 77,  /* count = 31 > 2 */
        [0] = 1,
        [49] = 2
    };
    use_value(auto_big[20] + auto_big[30]);
    
    /* Use static array already initialized */
    use_value(static_array[WIDE_START] + static_array[WIDE_END]);
    
    /* ===== PATH 4: Using enum constants for bounds ===== */
    int enum_bounded[] = {
        [LOWER ... UPPER] = 123,  /* count = 4 > 2 */
        [0] = 1
    };
    use_value(enum_bounded[LOWER] + enum_bounded[UPPER]);
    
    /* ===== PATH 5: Volatile ensures MEM_P(target) = true ===== */
    volatile int volatile_array[30] = {
        [5 ... 15] = 999  /* count = 11 > 2 */
    };
    use_value(volatile_array[10]);
    
    /* ===== PATH 6: Nested block with different scope ===== */
    {
        /* Compound literal assignment - creates initialization context */
        struct WithArray *ptr = &(struct WithArray){
            .header = 1,
            .data = { [1 ... 3] = 55 },  /* count = 3 > 2 */
            .ps = { .a = 1, .b = 2, .c = 1 }
        };
        use_value(ptr->data[2]);
    }
    
    /* ===== PATH 7: Multi-dimensional with constant ranges ===== */
    int local_md[3][5] = {
        [0 ... 1][2 ... 3] = 88  /* 2x2 = 4 elements > 2 */
    };
    use_value(local_md[0][2] + local_md[1][3]);
    
    /* Use static multi-dimensional */
    use_value(md_array[0][1] + md_array[2][4]);
    
    /* ===== PATH 8: Different element types with constant sizes ===== */
    /* char - size 8 bits */
    char char_array[100] = {
        [20 ... 50] = 'A'  /* count = 31 > 2 */
    };
    use_value(char_array[30]);
    
    /* short - size 16 bits */
    short short_array[64] = {
        [10 ... 20] = 32000  /* count = 11 > 2 */
    };
    use_value(short_array[15]);
    
    /* long long - size 64 bits */
    long long ll_array[32] = {
        [5 ... 15] = 0x123456789ABCDEFLL  /* count = 11 > 2 */
    };
    use_value(ll_array[10] & 0xFF);
    
    /* ===== PATH 9: Mixed initializers with conditional ===== */
    if (1) {  /* Always true, but creates control flow context */
        int conditional_array[40] = {
            [8 ... 25] = 777,  /* count = 18 > 2 */
            [0] = 1
        };
        use_value(conditional_array[10]);
    }
    
    /* ===== PATH 10: Switch with constant case ===== */
    switch (3) {
        case 3: {
            int switch_array[25] = {
                [5 ... 15] = 333  /* count = 11 > 2 */
            };
            use_value(switch_array[10]);
            break;
        }
        default:
            break;
    }
    
    /* ===== PATH 11: Array with attribute affecting alignment ===== */
    int __attribute__((aligned(32))) aligned_array[64] = {
        [16 ... 48] = 1024  /* count = 33 > 2 */
    };
    use_value(aligned_array[32]);
    
    /* ===== PATH 12: Struct with nested array initializer ===== */
    struct WithArray sa = {
        .header = 100,
        .data = { [2 ... 6] = 999 },  /* count = 5 > 2 */
        .ps = { .a = 7, .b = 255, .c = 3 }
    };
    use_value(sa.data[3] + sa.ps.b);
    
    printf("All initializations completed\n");
    
    return 0;
}
