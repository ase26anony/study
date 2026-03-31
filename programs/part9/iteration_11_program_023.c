/* Test case for expr.cc constant bounds analysis coverage */
/* Compile with: gcc -O2 -fdump-tree-ccp1 -fprofile-arcs -ftest-coverage -o test_expr test_expr.c */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization from removing test functions */
#define NOINLINE __attribute__((noinline))

/* Vector types for non-memory reference cases */
typedef int v4si __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* Small struct for 2-element access */
struct two_elem { int a; int b; };

/* Volatile to prevent constant propagation */
extern volatile int g_volatile;

/* Scenario 1: Small element count (count <= 2) - MEM_P path */
static NOINLINE void test_small_count_mem(void) {
    int arr[100] = {0};
    struct two_elem s = {0};
    
    /* Single element access - count = 1 */
    if (g_volatile) {
        arr[5] = 42;           /* lo=5, hi=5, count=1 */
    } else {
        arr[10] = 43;          /* Alternative constant bound */
    }
    
    /* Two adjacent elements - count = 2 */
    s.a = arr[20];             /* First element */
    s.b = arr[21];             /* Second element, adjacent */
    
    /* Vector with 2-element access */
    v4si v = {1, 2, 3, 4};
    int x = v[0];              /* Single element from vector */
    int y = v[1];              /* Second element from vector */
    
    /* Prevent dead code elimination */
    arr[0] = s.a + s.b + x + y;
}

/* Scenario 2: Larger constant-sized access - MEM_P path */
static NOINLINE void test_larger_constant_mem(void) {
    char char_arr[256] = {0};
    int int_arr[100] = {0};
    
    /* Access 8 chars: TYPE_SIZE = 8 bits, count = 8, total = 64 bits */
    for (int i = 2; i < 10; ++i) {  /* lo=2, hi=9, count=8 */
        char_arr[i] = (char)(i * 3);
    }
    
    /* Access 10 ints: TYPE_SIZE = 32 bits, count = 10, total = 320 bits */
    /* Use volatile in condition to preserve loop */
    int limit = g_volatile ? 10 : 20;
    for (int i = 5; i < 5 + limit; ++i) {  /* Compiler sees both bounds */
        if (i < 15) {  /* But we ensure i < 15 at runtime */
            int_arr[i] = i * 2;
        }
    }
    
    /* Fixed-size array section with constant bounds */
    int sum = 0;
    for (int i = 30; i <= 49; ++i) {  /* lo=30, hi=49, count=20 */
        sum += int_arr[i];
    }
    
    /* Prevent elimination */
    char_arr[0] = (char)(sum & 0xFF);
}

/* Scenario 3: Non-memory reference (!MEM_P) - vector operations */
static NOINLINE v4si test_non_mem_vector(void) {
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    
    /* Vector shuffle with constant indices - creates VEC_PERM_EXPR */
    v4si shuffled;
    if (g_volatile) {
        /* Constant bounds: indices 0,1,2,3 */
        shuffled = __builtin_shufflevector(v1, v2, 0, 1, 2, 3);
    } else {
        /* Alternative constant bounds: indices 3,2,1,0 */
        shuffled = __builtin_shufflevector(v1, v2, 3, 2, 1, 0);
    }
    
    /* Vector compound literal with constant indices */
    v4si constructed;
    if (g_volatile) {
        /* Access elements 0 and 1 from v1, 0 and 1 from v2 */
        constructed = (v4si){v1[0], v1[1], v2[0], v2[1]};
    } else {
        /* Alternative: elements 2 and 3 from both vectors */
        constructed = (v4si){v1[2], v1[3], v2[2], v2[3]};
    }
    
    /* Vector blend with constant mask */
    v4si blended;
    for (int i = 0; i < 4; ++i) {
        /* Constant index in each iteration */
        blended[i] = (i < 2) ? v1[i] : v2[i];
    }
    
    return shuffled + constructed + blended;
}

/* Scenario 4: Mixed array/vector with complex bounds */
static NOINLINE int test_mixed_complex(void) {
    float farr[64] = {0};
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Constant bounds from arithmetic expression */
    int start = (g_volatile & 1) ? 8 : 16;  /* Volatile but known 0 or 1 */
    int lo = start * 2;      /* Constant: either 0 or 2 */
    int hi = lo + 7;         /* Constant: either 7 or 9 */
    
    /* Memory access with constant bounds from derived values */
    float sum = 0.0f;
    for (int i = lo; i <= hi; ++i) {  /* lo and hi are constant */
        farr[i] = i * 0.5f;
        sum += farr[i];
    }
    
    /* Vector extract with constant index */
    float extracted;
    if (g_volatile) {
        extracted = fvec1[0] + fvec1[1];  /* Two elements */
    } else {
        extracted = fvec1[2] + fvec1[3];  /* Alternative two elements */
    }
    
    /* Vector construction from array with constant indices */
    v4sf mixed_vec = {
        farr[0], farr[1], farr[2], farr[3]  /* All constant indices */
    };
    
    return (int)(sum + extracted + mixed_vec[0]);
}

/* Main driver that calls all scenarios */
int main(void) {
    int checksum = 0;
    
    /* Initialize volatile to deterministic value for reproducible execution */
    int *volatile ptr = &g_volatile;
    *ptr = 0;  /* But compiler doesn't know this */
    
    /* Execute all test scenarios */
    test_small_count_mem();
    
    test_larger_constant_mem();
    
    v4si vec_result = test_non_mem_vector();
    for (int i = 0; i < 4; ++i) {
        checksum += vec_result[i];
    }
    
    checksum += test_mixed_complex();
    
    /* Print checksum to prevent elimination and verify execution */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
