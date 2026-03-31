/* Test program to cover constant bounds checking in GCC's expr.cc */
#include <stdio.h>
#include <stdint.h>

/* Compile with: gcc -O0 -fno-omit-frame-pointer -std=gnu11 -fextended-identifiers test.c -o test */
/* Also test with: gcc -O2 -ftree-vectorize -std=gnu11 test.c -o test_opt */

/* ========== PART 1: Constant bounds definitions ========== */
enum { 
    LOWER = 2, 
    UPPER = 5,
    SMALL_COUNT = 2
};

const int C_LOW = 0;
const int C_HIGH = 9;

/* ========== PART 2: Small struct with constant size ========== */
struct SmallPacked {
    unsigned int a : 7;
    unsigned int b : 9;
    unsigned int c : 4;
} __attribute__((packed));

/* ========== PART 3: Main test scenarios ========== */

/* Scenario A: Register target with !MEM_P(target) */
static void test_register_target(void) {
    /* Force register storage class for a small struct */
    register struct SmallPacked reg_target = {
        .a = 1,
        .b = 2,
        .c = 3
    };
    
    /* Use designated initializer with constant range (count=4) */
    register int reg_arr[10] = {[LOWER ... UPPER] = 42};
    
    /* Prevent dead code elimination */
    volatile int use1 = reg_target.a + reg_target.b;
    volatile int use2 = reg_arr[LOWER] + reg_arr[UPPER];
    (void)use1;
    (void)use2;
}

/* Scenario B: Memory target with count <= 2 */
static void test_small_count(void) {
    /* Static initialization - definitely MEM_P */
    static int small_range[10] = {[3] = 100, [4] = 200};  /* count=2 */
    
    /* Automatic variable with exactly 1 element range */
    int single_elem[20] = {[15] = 999};  /* count=1 */
    
    /* Use volatile to ensure memory operand */
    volatile int vol_arr[5] = {[0 ... 1] = 77};  /* count=2 */
    
    /* Prevent optimization */
    printf("Small count: %d %d %d\n", 
           small_range[3], single_elem[15], vol_arr[0]);
}

/* Scenario C: Memory target with count > 2 and constant element size */
static void test_large_count(void) {
    /* Large constant range initialization */
    int big_array[100] = {[10 ... 90] = 12345};  /* count=81 > 2 */
    
    /* With different element type (char) */
    char char_array[256] = {[32 ... 127] = 'A'};  /* count=96 > 2 */
    
    /* Struct array with constant size */
    struct SmallPacked packed_array[50] = {
        [10 ... 40] = {.a = 1, .b = 2, .c = 3}  /* count=31 > 2 */
    };
    
    printf("Large count[50]=%d, char[64]=%c, packed[20].b=%u\n",
           big_array[50], char_array[64], packed_array[20].b);
}

/* Scenario D: Multi-dimensional arrays with constant ranges */
static void test_multi_dim(void) {
    /* 2D array with range in both dimensions */
    int matrix[5][10] = { 
        [0 ... 2][3 ... 7] = 888  /* Nested constant ranges */
    };
    
    /* 3D array */
    int cube[3][4][5] = {
        [0 ... 1][1 ... 2][2 ... 3] = 777
    };
    
    /* Struct containing array with range */
    struct Container {
        int id;
        int values[8];
    } container = {
        .id = 1,
        .values = {[1 ... 6] = 42}  /* count=6 > 2 */
    };
    
    printf("Matrix[1][5]=%d, Cube[0][1][3]=%d, Container.values[3]=%d\n",
           matrix[1][5], cube[0][1][3], container.values[3]);
}

/* Scenario E: Mixed contexts with conditional compilation */
static void test_mixed_contexts(int selector) {
    /* Constant condition ensures initialization is parsed */
    if (selector > 0) {
        /* Automatic array - might be register or memory depending on optimization */
        int auto_array[20] = {[C_LOW ... C_HIGH] = 555};  /* count=10 > 2 */
        
        /* Compound literal assignment */
        struct SmallPacked *ptr = &(struct SmallPacked){
            .a = 5, .b = 10, .c = 2
        };
        
        printf("Auto[5]=%d, ptr->b=%u\n", auto_array[5], ptr->b);
    }
    
    /* Switch with constant cases */
    switch (selector) {
        case 0: {
            /* Nested block with initialization */
            volatile int switch_arr[8] = {[2 ... 5] = 333};  /* count=4 > 2 */
            printf("Switch case 0: %d\n", switch_arr[3]);
            break;
        }
        case 1: {
            /* Different range */
            int another[6] = {[1] = 111, [2] = 222};  /* count=2 */
            printf("Switch case 1: %d\n", another[1]);
            break;
        }
    }
}

/* Scenario F: Alignment attributes affecting MEM_P classification */
static void test_aligned_targets(void) {
    /* Aligned array might affect how GCC classifies the target */
    int aligned_array[16] __attribute__((aligned(64))) = {
        [4 ... 12] = 1024  /* count=9 > 2 */
    };
    
    /* Packed struct array */
    struct __attribute__((packed, aligned(1))) TightPacked {
        char a;
        int b;
    } tight_array[10] = {
        [2 ... 8] = {.a = 'x', .b = 999}  /* count=7 > 2 */
    };
    
    printf("Aligned[8]=%d, Tight[5].b=%d\n", 
           aligned_array[8], tight_array[5].b);
}

/* ========== PART 4: Main driver ========== */
int main(void) {
    printf("=== Testing constant bounds initialization paths ===\n");
    
    /* 1. Register target scenario (exploring !MEM_P(target)) */
    test_register_target();
    
    /* 2. Small count scenarios (count <= 2) */
    test_small_count();
    
    /* 3. Large count with memory target */
    test_large_count();
    
    /* 4. Multi-dimensional arrays */
    test_multi_dim();
    
    /* 5. Mixed contexts with different selectors */
    for (int i = 0; i < 3; i++) {
        test_mixed_contexts(i);
    }
    
    /* 6. Aligned targets */
    test_aligned_targets();
    
    /* Additional edge case: Zero-length range? */
    {
        /* GCC might handle this specially */
        int edge[10] = {[5 ... 5] = 9999};  /* count=1 */
        printf("Edge case: %d\n", edge[5]);
    }
    
    printf("=== All tests completed ===\n");
    return 0;
}
