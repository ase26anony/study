/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global and local variables with various uses */
int global_var_1 = 10;
float global_var_2 = 20.5;
char global_var_3 = 'A';
static int static_var = 30;
extern int extern_func(int);  /* Forces identifier lookup */

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

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
    void* as_ptr;
};

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    /* Multiple variables modified in loop for SSA */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
        int z = x + y;  /* Additional SSA variable */
        x = z - i;
    }
    return x + y;
}

__attribute__((noinline))
int ssa_pattern_2(int n) {
    volatile int result = 0;  /* Prevent optimization */
    int a = 1, b = 2, c = 3;
    
    /* Nested loops create complex SSA web */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < i; j++) {
            a = a + b;
            b = b - c;
            c = c * a;
        }
        result += a + b + c;
    }
    
    /* Conditional creates phi nodes */
    int final = (result > 0) ? a : b;
    return final + c;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern(void) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int inner1 = 10;
        
        /* Level 2 block */
        {
            int inner2 = 20;
            
            /* Level 3 block with statement expression (GCC extension) */
            outer = ({
                int temp = inner1 + inner2;
                temp * 2;
            });
            
            /* Label and address-of-label */
            void* target = &&exit_block;
            goto *target;
            
            exit_block: ;
        }
        
        /* Another block with switch */
        {
            switch (inner1) {
                case 10: outer += 5; break;
                default: outer -= 5; break;
            }
        }
    }
    
    /* Additional scope with for loop block */
    for (int i = 0; i < 5; i++) {
        int loop_var = i * 2;
        outer += loop_var;
        {
            int inner_loop = loop_var + 1;
            outer -= inner_loop;
        }
    }
    
    return outer;
}

/* Pattern 6: CONSTRUCTOR - Function using various initializers */
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
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[3][2] = {{1, 2}, {3, 4}, {5, 6}};
    
    /* Compound literal */
    int sum = 0;
    int* dyn_arr = (int[]){10, 20, 30, 40};
    
    for (int i = 0; i < 4; i++) {
        sum += dyn_arr[i];
    }
    
    /* Nested structure initializer */
    struct Nested {
        struct ComplexStruct cs;
        int extra;
    } nested = {
        .cs = { .int_field = 100, .float_field = 1.5f, .double_field = 3.0, .char_field = 'Z' },
        .extra = 999
    };
    
    /* Union initializer */
    union MixedUnion u1 = { .as_int = 0xDEADBEEF };
    union MixedUnion u2 = { .as_float = 1.234f };
    
    return sum + s1.int_field + nested.extra + arr1[0] + arr2[0][0];
}

/* Pattern 2: TREE_VEC - Vector operations */
#ifdef __GNUC__
__attribute__((noinline))
v4si vector_pattern(v4si a, v4si b) {
    v4si result;
    
    /* Various vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    
    /* Vector comparisons */
    v4si mask = a > b;
    result = result & mask;
    
    /* Vector shuffling */
    result = __builtin_shuffle(result, (v4si){3, 2, 1, 0});
    
    return result;
}

__attribute__((noinline))
v4sf float_vector_pattern(v4sf a, v4sf b) {
    v4sf result = a + b;
    result = result * a;
    result = result / (b + (v4sf){1.0f, 1.0f, 1.0f, 1.0f});
    return result;
}
#endif

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int* arr, int n) {
    int sum = 0;
    int i;
    
    /* Parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static, 4)
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    int max_val = 0;
    /* Another parallel region with different clauses */
    #pragma omp parallel private(i) shared(arr, n)
    {
        #pragma omp for reduction(max:max_val) nowait
        for (i = 0; i < n; i++) {
            if (arr[i] > max_val) {
                max_val = arr[i];
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(i)
        {
            i = max_val;
        }
    }
    
    /* Sections with firstprivate/lastprivate */
    int section_result = 0;
    #pragma omp parallel sections firstprivate(section_result) lastprivate(section_result)
    {
        #pragma omp section
        {
            section_result = 1;
        }
        
        #pragma omp section
        {
            section_result = 2;
        }
    }
    
    return sum + max_val + section_result;
}
#endif

/* Pattern 1: IDENTIFIER_NODE - Function using identifiers in various contexts */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Use global identifiers */
    int local_copy = global_var_1;
    float f_copy = global_var_2;
    char c_copy = global_var_3;
    
    /* Take addresses */
    int* ptr1 = &global_var_1;
    float* ptr2 = &global_var_2;
    
    /* sizeof expressions */
    size_t sz1 = sizeof(global_var_1);
    size_t sz2 = sizeof(global_var_2);
    size_t sz3 = sizeof(global_var_3);
    
    /* Declare local variables with different names */
    int local_var_a = 100;
    int local_var_b = 200;
    int local_var_c = 300;
    
    /* Use them in expressions */
    int complex_expr = (local_var_a + local_var_b) * local_var_c;
    
    /* Address and dereference */
    int* ptr_a = &local_var_a;
    int* ptr_b = &local_var_b;
    *ptr_a = *ptr_b + complex_expr;
    
    /* Function pointer */
    int (*func_ptr)(int) = &extern_func;
    
    return local_copy + (int)f_copy + (int)c_copy + (int)sz1 + complex_expr;
}

/* Main function that calls all patterns */
int main(void) {
    volatile int total = 0;  /* Prevent optimization */
    
    /* Pattern 1: IDENTIFIER_NODE */
    total += identifier_pattern();
    
    /* Pattern 4: SSA_NAME */
    total += ssa_pattern_1(100);
    total += ssa_pattern_2(50);
    
    /* Pattern 5: BLOCK */
    total += block_pattern();
    
    /* Pattern 6: CONSTRUCTOR */
    total += constructor_pattern();
    
#ifdef __GNUC__
    /* Pattern 2: TREE_VEC */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_pattern(vec_a, vec_b);
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_result = float_vector_pattern(fvec_a, fvec_b);
    
    /* Extract elements from vectors */
    int vec_sum = vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
    total += vec_sum;
#endif

#ifdef _OPENMP
    /* Pattern 7: OMP_CLAUSE */
    int omp_arr[100];
    for (int i = 0; i < 100; i++) {
        omp_arr[i] = i + 1;
    }
    total += omp_pattern(omp_arr, 100);
#endif
    
    /* Use static variable */
    total += static_var;
    
    /* Final output to prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Total: " << total << std::endl;
#else
    printf("Total: %d\n", total);
#endif
    
    return total > 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */

/* Pattern 3: TREE_BINFO - C++ class hierarchy */
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method() override { return 84; }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int virtual_method() override { return 168; }
    int second_data;
};

__attribute__((noinline))
int binfo_pattern() {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Virtual call through base pointer */
    int result = base_ptr->virtual_method();
    
    /* Access through reference */
    BaseClass& base_ref = derived_obj;
    result += base_ref.virtual_method();
    
    /* Multiple inheritance levels */
    SecondDerived second_obj;
    BaseClass* base_ptr2 = &second_obj;
    result += base_ptr2->virtual_method();
    
    /* Casts that involve binfo */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->virtual_method();
    }
    
    return result;
}

/* C++ main that includes binfo pattern */
int main() {
    int total = 0;
    
    /* Call all C patterns */
    total += ::main();
    
    /* Add C++ binfo pattern */
    total += binfo_pattern();
    
    std::cout << "C++ Total: " << total << std::endl;
    return 0;
}
#endif

/* Dummy extern function for identifier pattern */
int extern_func(int x) {
    return x * 2;
}
