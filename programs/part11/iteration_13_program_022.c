#include <stddef.h>

/* Vector extensions for constant indexing */
typedef int v4si __attribute__((vector_size(16)));
typedef char v16c __attribute__((vector_size(16)));

/* Struct with array for constant slicing */
struct ArrayStruct {
    int data[20];
    char buffer[100];
};

/* Struct with flexible array member */
struct FlexStruct {
    int count;
    char items[];
};

/* Test function 1: Designated initializers with constant ranges */
void test_designated_init() {
    /* Single element constant index - count = 1 */
    int arr1[10] = {[5] = 42};
    
    /* Two element constant range - count = 2 */
    int arr2[10] = {[2] = 1, [3] = 2};
    
    /* Larger constant range with char - count > 2, small element size */
    char buf1[100] = {[10 ... 20] = 'x'};  /* 11 elements */
    
    /* Even larger range */
    char buf2[200] = {[50 ... 100] = 'y'};  /* 51 elements */
    
    /* Mixed initialization */
    int arr3[20] = {[0 ... 4] = 1, [10 ... 14] = 2};
}

/* Test function 2: Vector operations with constant indices */
void test_vector_ops() {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    
    /* Constant index access - likely goes to register */
    int x = a[2];  /* Non-MEM_P(target) path */
    
    /* Vector operations with constant masks */
    v4si mask = {0, -1, 0, -1};
    v4si res1 = a * mask;  /* Constant vector mask */
    
    /* Vector shuffle with constant indices */
    v4si shuffled = __builtin_shuffle(a, b, (v4si){0, 2, 1, 3});
    
    /* Character vector with many elements */
    v16c chars = {'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p'};
    char c = chars[10];  /* Constant index */
}

/* Test function 3: Array slicing in structs */
void test_struct_slicing(struct ArrayStruct *s) {
    /* Constant start index for pointer */
    int *p1 = &s->data[2];  /* Constant bound 2 */
    int *p2 = &s->data[5];  /* Constant bound 5 */
    
    /* Access sub-range with constant bounds */
    s->data[3] = 10;  /* Single element - MEM_P(target) true, count = 1 */
    s->data[4] = 20;  /* Another single element */
    
    /* Two adjacent elements */
    s->data[6] = 30;
    s->data[7] = 40;  /* count = 2 if processed together */
    
    /* Character array with constant range */
    for (int i = 10; i < 15; i++) {  /* Constant bound loop */
        s->buffer[i] = 'z';
    }
}

/* Test function 4: Complex expressions with constant bounds */
int test_complex_expr(int *arr, int cond) {
    /* Nested array access with constant inner index */
    int x = arr[arr[2]];  /* arr[2] is constant index access */
    
    /* Conditional with constant indices in both branches */
    int y = (cond ? arr[3] : arr[4]);
    
    /* Expression combining multiple constant indexed accesses */
    int z = arr[1] + arr[2] * arr[3] - arr[4];
    
    /* Switch with constant array indices */
    switch(cond) {
        case 0: return arr[0];
        case 1: return arr[1];
        case 2: return arr[2];
        default: return arr[3];
    }
    
    return x + y + z;
}

/* Test function 5: String literal constant indexing */
void test_string_indexing() {
    /* Direct constant indexing */
    char c1 = "hello world"[4];  /* 'o' */
    
    /* Multiple constant indices */
    char c2 = "constant string"[0];
    char c3 = "constant string"[7];
    
    /* In expression */
    int diff = "abc"[2] - "xyz"[1];
}

/* Test function 6: Loop with constant bounds (may be unrolled) */
void test_constant_loops(int *arr) {
    /* Small constant bound loop */
    for (int i = 0; i < 3; i++) {  /* count = 3 */
        arr[i] = i * 10;
    }
    
    /* Medium constant bound */
    for (int i = 0; i < 8; i++) {  /* count = 8 */
        arr[i + 10] = i * 5;
    }
    
    /* Nested constant loops */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i * 4 + j] = i + j;
        }
    }
}

/* Test function 7: __builtin_constant_p with array bounds */
void test_builtin_constant(int *arr) {
    /* Force constant evaluation of array access */
    if (__builtin_constant_p(arr[5])) {
        /* This branch might be taken during compilation */
        arr[5] = 100;
    }
    
    /* Check constant index expression */
    if (__builtin_constant_p(2 + 3)) {
        arr[2 + 3] = 200;  /* Constant index 5 */
    }
}

/* Test function 8: Mixed operations to trigger different paths */
void test_mixed_operations() {
    struct ArrayStruct s = {0};
    int local_arr[50] = {0};
    
    /* Combination of different patterns */
    
    /* 1. MEM_P(target) true with count = 1 */
    s.data[5] = 42;
    
    /* 2. MEM_P(target) true with count = 2 */
    s.data[10] = 1;
    s.data[11] = 2;
    
    /* 3. MEM_P(target) true with count > 2 and small element size */
    for (int i = 20; i < 30; i++) {
        s.buffer[i] = 'a' + (i - 20);
    }
    
    /* 4. Non-MEM_P(target) path - register destination */
    int reg1 = local_arr[2] + local_arr[3];
    int reg2 = local_arr[4] * local_arr[5];
    
    /* 5. Vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec_sum = vec1 + vec2;
    int vec_elem = vec_sum[1];  /* Goes to register */
    
    /* 6. String constant indexing */
    char str_elem = "test string"[6];
    
    /* Use results to prevent optimization */
    s.data[0] = reg1 + reg2 + vec_elem + str_elem;
}

/* Main function that exercises all test cases */
int main() {
    int array[100] = {0};
    struct ArrayStruct s = {0};
    
    /* Execute all test functions */
    test_designated_init();
    test_vector_ops();
    test_struct_slicing(&s);
    
    int result = test_complex_expr(array, 2);
    
    test_string_indexing();
    test_constant_loops(array);
    test_builtin_constant(array);
    test_mixed_operations();
    
    /* Use results to prevent dead code elimination */
    return result + array[0] + s.data[0];
}
