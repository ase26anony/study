/* test_tree_nodes.cc - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and usage */
int global_var_1 = 10;
int global_var_2 = 20;
float global_var_3 = 30.5;
double global_var_4 = 40.7;

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class inheritance (C++ only) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int get_value() { return 100; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() override { return 200; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * 2;
    }
    return x + y;
}

__attribute__((noinline))
int ssa_pattern_2(int n) {
    int a = 5, b = 10, c = 15;
    for (int i = 0; i < n; ++i) {
        a = a + b;
        b = b + c;
        c = c + a;
    }
    return a + b + c;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern() {
    int result = 0;
    
    /* Level 1 block */
    {
        int a = 10;
        
        /* Level 2 block */
        {
            int b = 20;
            
            /* Level 3 block */
            {
                int c = 30;
                result = a + b + c;
            }
        }
    }
    
    /* GCC statement expression */
    result += ({
        int temp = 100;
        temp * 2;
    });
    
    /* Labels and goto for block creation */
    void* label_ptr = &&my_label;
    goto *label_ptr;
    
my_label:
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern() {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int int_field;
        float float_field;
        double double_field;
        char char_field;
    };
    
    struct ComplexStruct s1 = {
        .int_field = 1,
        .float_field = 2.0f,
        .double_field = 3.0,
        .char_field = 'A'
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int* ptr = (int[3]){100, 200, 300};
    
    /* Nested initializer */
    struct Nested {
        int a;
        struct {
            int x;
            int y;
        } inner;
    } nested = { .a = 5, .inner = { .x = 1, .y = 2 } };
    
    return s1.int_field + arr[0] + ptr[0] + nested.a;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* Multiple OpenMP clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int val = i * 10 + j;
            if (val > max_val) max_val = val;
        }
    }
    
    return sum + max_val;
}
#endif

/* Pattern 2: TREE_VEC - Vector operations function */
#ifdef __GNUC__
__attribute__((noinline))
int vector_pattern() {
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c = vec_a + vec_b;
    v4si vec_d = vec_a * vec_b;
    
    v4sf vec_f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec_f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf vec_f3 = vec_f1 * vec_f2;
    
    /* Extract elements to force usage */
    int* ptr = (int*)&vec_c;
    float* fptr = (float*)&vec_f3;
    
    return ptr[0] + ptr[1] + (int)fptr[0];
}
#endif

/* Pattern 3: TREE_BINFO - C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int val1 = base_ptr->get_value();
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    int val2 = derived_ptr ? derived_ptr->get_value() : 0;
    
    /* Array of base pointers */
    BaseClass* base_array[3];
    base_array[0] = &derived;
    
    return val1 + val2;
}
#endif

/* Main function that combines all patterns */
int main() {
    volatile int result = 0;
    
    /* Use IDENTIFIER_NODE patterns */
    result += global_var_1;
    result += global_var_2;
    result += (int)global_var_3;
    result += (int)global_var_4;
    
    /* Take addresses to force identifier lookups */
    int* p1 = &global_var_1;
    float* p2 = &global_var_3;
    
    /* sizeof expressions with identifiers */
    result += sizeof(global_var_1);
    result += sizeof(global_var_4);
    
    /* SSA patterns */
    result += ssa_pattern_1(100);
    result += ssa_pattern_2(50);
    
    /* Block pattern */
    result += block_pattern();
    
    /* Constructor pattern */
    result += constructor_pattern();
    
#ifdef __GNUC__
    /* Vector pattern */
    result += vector_pattern();
#endif
    
#ifdef __cplusplus
    /* BINFO pattern */
    result += binfo_pattern();
#endif
    
#ifdef _OPENMP
    /* OpenMP pattern */
    result += omp_pattern(1000);
#endif
    
    /* Prevent dead code elimination */
    volatile int* volatile_output = &result;
    
#ifdef __cplusplus
    std::cout << "Result: " << result << std::endl;
#else
    /* Simple output for C */
    printf("Result: %d\n", result);
#endif
    
    return result > 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
