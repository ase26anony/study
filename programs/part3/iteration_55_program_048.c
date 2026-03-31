/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var1 = 10;
int global_var2 = 20;
float global_var3 = 3.14;
double global_var4 = 2.71828;
char global_var5 = 'A';

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int get_value() { return 42; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() override { return 84; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline)) 
int ssa_pattern1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    return x + y;
}

__attribute__((noinline))
float ssa_pattern2(int n) {
    float sum = 0.0f;
    float prod = 1.0f;
    for (int i = 1; i <= n; ++i) {
        sum = sum + (1.0f / i);
        prod = prod * (1.0f + 0.01f * i);
    }
    return sum * prod;
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
            
            /* Level 3 block with statement expression */
            result = ({
                int c = b * 3;
                int d = c - 5;
                d;
            });
        }
    }
    
    /* Another block with label address */
    {
        void* label_ptr = &&my_label;
        goto *label_ptr;
        
        my_label:
        result += 100;
    }
    
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
};

union MixedUnion {
    int as_int;
    float as_float;
    struct {
        char a, b, c, d;
    } as_chars;
};

__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct ComplexStruct s1 = { 
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'X'
    };
    
    /* Array initializer */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Compound literal */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += ((int[3]){10, 20, 30})[i];
    }
    
    /* Nested initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = { .int_field = 100, .float_field = 1.5f, .double_field = 3.0, .char_field = 'Z' },
        .extra = 999
    };
    
    /* Union initializer */
    union MixedUnion u = { .as_int = 0xDEADBEEF };
    
    return s1.int_field + arr[2] + sum + nested.extra + u.as_chars.a;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static, 10)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP section */
    int max_val = 0;
    #pragma omp parallel sections private(size) reduction(max:max_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < 50; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = 50; i < 100; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
    }
    
    /* OpenMP task */
    int task_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task firstprivate(size) shared(task_result)
            {
                task_result = arr[0] * arr[99];
            }
        }
    }
    
    return sum + max_val + task_result;
}

/* Vector operations function */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Extract elements to ensure computation */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec3[i] + vec4[i] + (int)fvec3[i];
    }
    return sum;
#else
    return 0;
#endif
}

/* C++ specific pattern */
#ifdef __cplusplus
__attribute__((noinline))
int cpp_binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* These operations involve TREE_BINFO nodes */
    int val1 = base_ptr->get_value();  // Virtual call
    int val2 = derived.get_value();     // Direct call
    
    BaseClass& base_ref = derived;
    int val3 = base_ref.get_value();    // Reference call
    
    return val1 + val2 + val3;
}
#endif

/* Main function that combines all patterns */
int main(void) {
    volatile int result = 0;  /* Prevent optimization */
    
    /* Use IDENTIFIER_NODE patterns */
    result += global_var1;
    result += global_var2;
    result += (int)global_var3;
    result += (int)global_var4;
    result += global_var5;
    
    /* Take addresses to force identifier lookups */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var3;
    
    /* Use sizeof on identifiers */
    result += sizeof(global_var1);
    result += sizeof(global_var2);
    
    /* Call SSA pattern functions */
    result += ssa_pattern1(100);
    result += (int)ssa_pattern2(50);
    
    /* Call block pattern */
    result += block_pattern(42);
    
    /* Call constructor pattern */
    result += constructor_pattern();
    
    /* Call vector pattern */
    result += vector_pattern();
    
    /* Call C++ pattern if in C++ mode */
#ifdef __cplusplus
    result += cpp_binfo_pattern();
#endif
    
    /* Call OpenMP pattern */
    result += omp_pattern(100);
    
    /* Final output to prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Result: " << result << std::endl;
#else
    printf("Result: %d\n", result);
#endif
    
    return result > 0 ? 0 : 1;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
