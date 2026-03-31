/* test_tree_nodes.c - Comprehensive test to trigger tree_kind coverage */

#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables for symbol table lookups */
int global_var_1 = 10;
int global_var_2 = 20;
float global_var_3 = 3.14f;
double global_var_4 = 2.71828;
char global_var_5 = 'A';

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy (only in C++ mode) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int get_value() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() { return 84; }
    int derived_data;
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    /* Multiple loops to generate SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = n; j > 0; --j) {
        x = x - j;
        y = y / (j + 1);
    }
    
    int z = 0;
    while (z < n) {
        x = x ^ z;  /* XOR operation */
        z = z + 2;
    }
    
    return x + y;
}

__attribute__((noinline))
float ssa_pattern_2(float limit) {
    float a = 0.0f;
    float b = 1.0f;
    
    for (float f = 0.0f; f < limit; f += 0.5f) {
        a = a + f;
        b = b * (f + 1.0f);
    }
    
    return a + b;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(int x) {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = x * 2;
        
        /* Level 2 block */
        {
            int b = a + 10;
            
            /* Level 3 block with GCC statement expression */
            result = ({
                int c = b * 3;
                int d = c - 5;
                d;  /* Returns d */
            });
            
            /* Label and address for potential BLOCK nodes */
            my_label:
            result += 1;
        }
    }
    
    /* Another block with different variables */
    {
        volatile int temp = 100;
        result += temp;
    }
    
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int int_field;
        float float_field;
        double double_field;
        char char_field;
    };
    
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'X'
    };
    
    struct ComplexStruct s2 = { 1, 2.0f, 3.0, 'Y' };
    
    /* Array initializer */
    int arr1[5] = { 10, 20, 30, 40, 50 };
    
    /* Compound literal */
    int sum = 0;
    int *arr2 = (int[3]){ 1, 2, 3 };
    
    for (int i = 0; i < 3; i++) {
        sum += arr2[i];
    }
    
    /* Nested structure initializer */
    struct Inner {
        int a;
        int b;
    };
    
    struct Outer {
        struct Inner inner;
        int extra;
    };
    
    struct Outer outer = {
        .inner = { .a = 100, .b = 200 },
        .extra = 300
    };
    
    return s1.int_field + arr1[0] + sum + outer.inner.a;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int *arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) firstprivate(size) lastprivate(max_val)
    for (int i = 0; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    
    free(arr);
    return sum + max_val;
}

/* Pattern 2: TREE_VEC - Vector operations function */
#ifdef __GNUC__
__attribute__((noinline))
v4si vector_pattern(v4si a, v4si b) {
    v4si result;
    
    /* Various vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    result = result / (a + 1);
    
    /* Vector comparisons */
    v4si mask = a > b;
    result = result & mask;
    
    return result;
}

__attribute__((noinline))
v4sf vector_float_pattern(v4sf a, v4sf b) {
    v4sf result = a + b;
    result = result * a;
    return result;
}
#endif

/* Pattern 3: TREE_BINFO - C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Access through base pointer - involves BINFO */
    int value = base_ptr->get_value();
    base_ptr->base_data = 100;
    
    /* Create another derived object */
    DerivedClass derived2;
    BaseClass& base_ref = derived2;
    value += base_ref.get_value();
    
    return value;
}
#endif

/* Main function that calls all patterns */
int main(int argc, char** argv) {
    volatile int result = 0;  /* volatile to prevent optimization */
    
    /* Use global identifiers (triggers IDENTIFIER_NODE) */
    result += global_var_1;
    result += (int)global_var_3;
    result += (int)global_var_4;
    result += global_var_5;
    
    /* Take addresses of globals */
    int* ptr1 = &global_var_1;
    float* ptr2 = &global_var_3;
    result += *ptr1;
    result += (int)*ptr2;
    
    /* Use sizeof on identifiers */
    result += sizeof(global_var_2);
    result += sizeof(global_var_4);
    
    /* Call SSA pattern functions */
    result += ssa_pattern_1(100);
    result += (int)ssa_pattern_2(10.0f);
    
    /* Call block pattern */
    result += block_pattern(50);
    
    /* Call constructor pattern */
    result += constructor_pattern();
    
    /* Call OpenMP pattern */
    result += omp_pattern(1000);
    
    #ifdef __GNUC__
    /* Vector operations */
    v4si vec_a = { 1, 2, 3, 4 };
    v4si vec_b = { 5, 6, 7, 8 };
    v4si vec_result = vector_pattern(vec_a, vec_b);
    
    v4sf vec_fa = { 1.0f, 2.0f, 3.0f, 4.0f };
    v4sf vec_fb = { 0.5f, 1.5f, 2.5f, 3.5f };
    v4sf vec_fresult = vector_float_pattern(vec_fa, vec_fb);
    
    /* Extract elements from vector results */
    int* vec_ptr = (int*)&vec_result;
    result += vec_ptr[0] + vec_ptr[1] + vec_ptr[2] + vec_ptr[3];
    #endif
    
    #ifdef __cplusplus
    /* C++ BINFO pattern */
    result += binfo_pattern();
    #endif
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}
