/* Test case for GCC expr.cc constant bounds analysis coverage.
   Specifically targeting lines 7691-7700 in expr.cc.
   
   Compile with: gcc -O2 -fdump-tree-ccp1 -fprofile-arcs -ftest-coverage -o test_expr test_expr.c
   Run with: ./test_expr
*/

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from eliminating entire functions */
#define NOINLINE __attribute__((noinline))

/* Vector types for non-MEM_P scenarios */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Volatile variables to prevent constant folding */
extern volatile int VOL_INT;
extern volatile char VOL_CHAR;

/* Scenario 1: Small element count (count <= 2) for MEM_P(target) path */
static NOINLINE void test_small_count_mem(void) {
    int arr[100] = {0};
    float farr[50] = {0.0f};
    
    /* Single element access - count = 1 */
    if (VOL_INT) {
        arr[5] = 42;           /* lo=5, hi=5, count=1 */
        farr[10] = 3.14f;      /* lo=10, hi=10, count=1 */
    }
    
    /* Two adjacent elements - count = 2 */
    if (VOL_CHAR) {
        arr[20] = 1;           /* lo=20, hi=21, count=2 */
        arr[21] = 2;
    } else {
        /* Alternative two-element pattern */
        farr[30] = 1.0f;
        farr[31] = 2.0f;
    }
    
    /* Small struct-like access */
    struct two_ints { int a; int b; } s;
    s.a = arr[0];  /* Could be analyzed as two separate 1-element accesses */
    s.b = arr[1];
}

/* Scenario 2: Larger constant-sized memory access where total size fits in unsigned HWI */
static NOINLINE void test_larger_constant_mem(void) {
    char carr[256] = {0};
    int iarr[64] = {0};
    
    /* Fixed-size loop with constant bounds: lo=2, hi=9, count=8 */
    /* For int: TYPE_SIZE = 32 bits, total = 32 * 8 = 256 bits (fits in uhwi) */
    for (int i = 2; i < 10; ++i) {
        iarr[i] = i * 2;
    }
    
    /* Another constant slice: lo=10, hi=25, count=16 */
    /* For char: TYPE_SIZE = 8 bits, total = 8 * 16 = 128 bits */
    for (int j = 10; j < 26; ++j) {
        carr[j] = j & 0xFF;
    }
    
    /* Mixed with volatile to prevent loop unrolling from changing analysis */
    int start = VOL_INT ? 5 : 15;
    /* Still constant bounds: lo=5 or 15, hi=24, count=20 or 10 */
    for (int k = start; k < start + 10; ++k) {
        iarr[k] += k;
    }
}

/* Scenario 3: Non-memory reference (!MEM_P(target)) - vector operations */
static NOINLINE v4si test_non_mem_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - not a direct memory load */
    /* Creates VEC_PERM_EXPR or CONSTRUCTOR node */
    v4si v3 = __builtin_shufflevector(v1, v2, 0, 1, 4, 5);  /* {1, 2, 5, 6} */
    
    /* Vector compound literal with constant indices */
    v4si v4 = (v4si){v1[0], v1[1], v1[2], v1[3]};  /* Full slice */
    
    /* Partial vector construction */
    v4si v5 = (v4si){v1[1], v1[2], v2[0], v2[1]};  /* Mixed slice */
    
    return v3 + v4 + v5;
}

/* Scenario 4: Vector memory access with constant bounds */
static NOINLINE v4sf test_vector_mem_access(void) {
    v4sf vec[10];
    float f = 1.0f;
    
    /* Initialize vector array */
    for (int i = 0; i < 10; ++i) {
        vec[i] = (v4sf){f, f+1, f+2, f+3};
        f += 4.0f;
    }
    
    /* Constant-bounded vector access: vec[2] to vec[5] */
    /* Each v4sf is 128 bits, count=4, total=512 bits */
    v4sf sum = {0.0f, 0.0f, 0.0f, 0.0f};
    for (int i = 2; i < 6; ++i) {
        sum += vec[i];
    }
    
    /* Single vector element access (count=1) */
    float first = ((float*)&vec[0])[0];  /* vec[0][0] */
    float last = ((float*)&vec[9])[3];   /* vec[9][3] */
    
    return sum + (v4sf){first, last, first, last};
}

/* Scenario 5: Multi-dimensional array with constant bounds */
static NOINLINE void test_multi_dim_constant(void) {
    int matrix[10][20];
    
    /* Constant slice in 2D array */
    for (int i = 2; i < 6; ++i) {          /* lo=2, hi=5, count=4 */
        for (int j = 3; j < 8; ++j) {      /* lo=3, hi=7, count=5 */
            matrix[i][j] = i * 100 + j;
        }
    }
    
    /* Different constant bounds based on volatile */
    int row_start = VOL_INT ? 0 : 5;
    int col_start = VOL_CHAR ? 0 : 10;
    
    /* Still constant: row_start+0 to row_start+3, col_start+0 to col_start+4 */
    for (int i = row_start; i < row_start + 4; ++i) {
        for (int j = col_start; j < col_start + 5; ++j) {
            matrix[i][j] += 1;
        }
    }
}

/* Main driver that calls all scenarios */
int main(void) {
    int checksum = 0;
    
    /* Initialize some volatile variables */
    volatile int vol_int = 1;
    volatile char vol_char = 'A';
    
    /* Point extern volatiles to local volatiles */
    volatile int* volatile_ptr_int = &vol_int;
    volatile char* volatile_ptr_char = &vol_char;
    
    test_small_count_mem();
    checksum += 1;
    
    test_larger_constant_mem();
    checksum += 2;
    
    v4si vec_result = test_non_mem_vector();
    checksum += vec_result[0] + vec_result[1];
    
    v4sf vecf_result = test_vector_mem_access();
    checksum += (int)vecf_result[0];
    
    test_multi_dim_constant();
    checksum += 3;
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
