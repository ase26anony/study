/* test_expr_coverage.c - Targeting expr.cc lines 7691-7700 */

#include <stdio.h>
#include <string.h>

/* Vector extension types for GCC */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));
typedef short v8s __attribute__((vector_size(16)));

/* Struct with array for memory access patterns */
struct ArrayStruct {
    int data[20];
    char buffer[100];
    short shorts[50];
};

/* Struct with flexible array member */
struct FlexStruct {
    int count;
    char items[];
};

/* Helper function to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* Test function 1: Designated initializers with constant ranges */
void test_designated_init(void) {
    /* Single element - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two elements - count = 2 */
    int arr2[10] = {[3] = 1, [4] = 2};
    
    /* Multiple elements (count > 2) with char type */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Multiple elements (count > 2) with int type */
    int arr3[20] = {[2 ... 8] = 99};  /* 7 elements */
    
    use(arr1); use(arr2); use(buf1); use(arr3);
}

/* Test function 2: Vector operations with constant indices */
void test_vector_ops(void) {
    v4si vec1 = {1, 2, 3, 4};
    v16c vec2 = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h',
                 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p'};
    v8s vec3 = {10, 20, 30, 40, 50, 60, 70, 80};
    
    /* Constant indexing into vectors (register targets) */
    int idx1 = vec1[2];      /* constant index 2 */
    char idx2 = vec2[5];     /* constant index 5 */
    short idx3 = vec3[3];    /* constant index 3 */
    
    /* Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si masked = vec1 * mask;
    
    /* Vector conditional with constant indices */
    v4si cond_vec = (vec1 > 2) ? vec1 : (v4si){0, 0, 0, 0};
    
    use(&idx1); use(&idx2); use(&idx3); use(&masked); use(&cond_vec);
}

/* Test function 3: Array slicing in structs */
void test_struct_array_slicing(void) {
    struct ArrayStruct s = {0};
    
    /* Constant start index for pointer */
    int *p1 = &s.data[2];           /* MEM_P target with constant index */
    char *p2 = &s.buffer[10];       /* MEM_P target with constant index */
    short *p3 = &s.shorts[5];       /* MEM_P target with constant index */
    
    /* Direct memory access with constant indices */
    s.data[3] = 100;                /* Single element, MEM_P target */
    s.buffer[15] = 'A';             /* Single element, MEM_P target */
    s.shorts[10] = 500;             /* Single element, MEM_P target */
    
    /* Multiple elements with constant bounds */
    for (int i = 0; i < 5; i++) {   /* Loop with constant bound 5 */
        s.data[i] = i * 10;
    }
    
    use(p1); use(p2); use(p3);
}

/* Test function 4: Complex expressions with constant bounds */
void test_complex_expressions(void) {
    int arr[20] = {0};
    int lookup[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    /* Nested array access with constant inner index */
    int x = arr[lookup[2]];         /* constant index 2 in inner access */
    
    /* Conditional array access with constant indices */
    int cond = 1;
    int y = (cond ? arr[2] : arr[3]);  /* both branches constant indices */
    
    /* Switch with array indexing */
    int z;
    switch (cond) {
        case 0: z = arr[1]; break;  /* constant index 1 */
        case 1: z = arr[2]; break;  /* constant index 2 */
        default: z = arr[3]; break; /* constant index 3 */
    }
    
    /* Expression combining multiple constant-index accesses */
    int sum = arr[1] + arr[2] + arr[3];  /* register target for sum */
    
    /* Builtin to force constant evaluation */
    if (__builtin_constant_p(arr[5])) {
        /* This path may be taken during compilation */
    }
    
    use(&x); use(&y); use(&z); use(&sum);
}

/* Test function 5: String literal operations */
void test_string_ops(void) {
    /* Constant indexing into string literal */
    char c1 = "hello"[2];           /* constant index 2 */
    char c2 = "world"[4];           /* constant index 4 */
    
    /* String copy with constant bounds */
    char dest[20];
    strncpy(dest, "constant string", 10);  /* constant bound 10 */
    
    /* Character array with constant range init */
    char letters[26] = {['a' - 'a' ... 'e' - 'a'] = 1};  /* 5 elements */
    
    use(&c1); use(&c2); use(dest); use(letters);
}

/* Test function 6: Mixed operations to trigger different paths */
void test_mixed_operations(void) {
    /* Small element type with large count */
    unsigned char bytes[256] = {[32 ... 127] = 0xFF};  /* 96 elements */
    
    /* Medium element type with medium count */
    int medium[50] = {[10 ... 19] = 42};  /* 10 elements */
    
    /* Operations that might go to registers */
    int temp = bytes[64] + bytes[65];  /* likely register target */
    
    /* Memory operations with different element sizes */
    struct {
        long long big[10];
        int medium[20];
        char small[30];
    } mixed;
    
    mixed.big[2] = 1000LL;      /* 8-byte elements */
    mixed.medium[5] = 500;      /* 4-byte elements */
    mixed.small[10] = 1;        /* 1-byte elements */
    
    /* Array section copy with constant bounds */
    int src[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int dst[10];
    
    /* This might trigger memcpy with constant bounds */
    for (int i = 0; i < 3; i++) {  /* constant bound 3 */
        dst[i] = src[i + 2];        /* constant offset 2 */
    }
    
    use(bytes); use(medium); use(&temp); use(&mixed); use(dst);
}

/* Main function that calls all tests */
int main(void) {
    printf("Testing constant bounds array/vector operations...\n");
    
    test_designated_init();
    test_vector_ops();
    test_struct_array_slicing();
    test_complex_expressions();
    test_string_ops();
    test_mixed_operations();
    
    printf("All tests completed (coverage during compilation).\n");
    return 0;
}
