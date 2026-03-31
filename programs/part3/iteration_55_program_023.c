/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */
#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables for identifier creation */
int global_var_1;
float global_var_2;
double global_var_3;
char global_var_4;

/* Function declarations that force identifier lookups */
extern int extern_func_1(int);
extern void extern_func_2(float);
extern double extern_func_3(void);

/* Force identifier node creation through various uses */
__attribute__((noinline))
int identifier_pattern(void) {
    /* Local variables */
    int local_var_1;
    float local_var_2;
    double local_var_3;
    
    /* Operations that create identifier nodes */
    int *ptr1 = &global_var_1;
    float *ptr2 = &global_var_2;
    size_t s1 = sizeof(global_var_3);
    size_t s2 = sizeof(local_var_1);
    
    /* Use in expressions */
    local_var_1 = global_var_1 + 10;
    local_var_2 = global_var_2 * 2.0f;
    local_var_3 = global_var_3 / 3.0;
    
    /* Function calls with identifiers */
    if (extern_func_1) {
        local_var_1 = extern_func_1(local_var_1);
    }
    
    return local_var_1 + (int)local_var_2 + (int)local_var_3;
}

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
/* Vector type declarations */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

__attribute__((noinline))
int vector_pattern(void) {
    /* Vector variables */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4sf fvec3 = fvec1 + fvec2;
    v4sf fvec4 = fvec1 * fvec2;
    
    /* Extract elements */
    int sum = vec3[0] + vec3[1] + vec3[2] + vec3[3];
    sum += (int)(fvec3[0] + fvec3[1] + fvec3[2] + fvec3[3]);
    
    return sum;
}
#else
__attribute__((noinline))
int vector_pattern(void) {
    /* Fallback for non-GCC */
    int arr[4] = {1, 2, 3, 4};
    return arr[0] + arr[1] + arr[2] + arr[3];
}
#endif

/* ========== SSA_NAME patterns ========== */
__attribute__((noinline))
int ssa_pattern(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to create SSA names */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n; ++j) {
        z = z - j;
        x = x + z;
    }
    
    /* Complex control flow */
    int w = 0;
    for (int k = 0; k < n; ++k) {
        if (k % 2 == 0) {
            w = w + k;
        } else {
            w = w - k;
        }
    }
    
    return x + y + z + w;
}

/* ========== BLOCK patterns ========== */
__attribute__((noinline))
int block_pattern(void) {
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
    
    /* GCC statement expression (creates a block) */
    int val = ({
        int temp = 100;
        int temp2 = 200;
        temp + temp2;
    });
    
    result += val;
    
    /* Labels and gotos (involve blocks) */
    void *label_ptr = &&my_label;
    
    if (result > 0) {
        goto my_label;
    }
    
    result += 50;
    
my_label:
    result += 100;
    
    /* Prevent goto being optimized away */
    if (label_ptr) {
        result += (long)label_ptr % 1000;
    }
    
    return result;
}

/* ========== CONSTRUCTOR patterns ========== */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
    int array_field[3];
};

__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure initializer with designated initializers */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'A',
        .array_field = {1, 2, 3}
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literals */
    int sum = ((int[3]){1, 2, 3})[0] + 
              ((int[3]){1, 2, 3})[1] + 
              ((int[3]){1, 2, 3})[2];
    
    /* Nested initializers */
    struct Nested {
        struct {
            int a;
            int b;
        } inner;
        int c;
    } nested = { .inner = {.a = 1, .b = 2}, .c = 3 };
    
    sum += s1.int_field + (int)s1.float_field + arr[0] + nested.inner.a;
    
    return sum;
}

/* ========== OpenMP patterns (OMP_CLAUSE) ========== */
#ifdef _OPENMP
#include <omp.h>

__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int *arr = 0;
    
    if (size > 0) {
        arr = (int*)__builtin_alloca(size * sizeof(int));
        for (int i = 0; i < size; i++) {
            arr[i] = i + 1;
        }
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) if(size > 100)
    for (int i = 0; i < (size > 0 ? size : 1); i++) {
        if (arr) {
            sum += arr[i];
        } else {
            sum += i;
        }
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val) schedule(static, 4)
    for (int i = 0; i < 100; i++) {
        if (i > max_val) {
            max_val = i;
        }
    }
    
    /* OpenMP sections */
    #pragma omp parallel sections private(sum)
    {
        #pragma omp section
        {
            sum += 1;
        }
        #pragma omp section
        {
            sum += 2;
        }
    }
    
    return sum + max_val;
}
#else
__attribute__((noinline))
int omp_pattern(int size) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < (size > 0 ? size : 10); i++) {
        sum += i;
    }
    return sum;
}
#endif

/* ========== Main function ========== */
int main(void) {
    volatile int result = 0;
    
    /* Call all pattern functions */
    result += identifier_pattern();
    result += vector_pattern();
    result += ssa_pattern(100);
    result += block_pattern();
    result += constructor_pattern();
    result += omp_pattern(1000);
    
    /* Prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Result: " << result << std::endl;
#else
    printf("Result: %d\n", result);
#endif
    
    return result != 0 ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */

/* ========== C++ patterns for TREE_BINFO ========== */
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

__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    
    /* Use polymorphism */
    int result = base_ptr->virtual_method();
    
    /* Access through derived pointer */
    DerivedClass* derived_ptr = static_cast<DerivedClass*>(base_ptr);
    result += derived_ptr->virtual_method();
    
    /* Try dynamic cast (involves BINFO) */
    BaseClass* another_base = dynamic_cast<BaseClass*>(derived_ptr);
    if (another_base) {
        result += another_base->virtual_method();
    }
    
    return result;
}

/* C++ main */
int cpp_main(void) {
    volatile int result = 0;
    
    result += identifier_pattern();
    result += vector_pattern();
    result += ssa_pattern(100);
    result += block_pattern();
    result += constructor_pattern();
    result += omp_pattern(1000);
    result += binfo_pattern();
    
    std::cout << "C++ Result: " << result << std::endl;
    
    return result != 0 ? 0 : 1;
}
#endif
