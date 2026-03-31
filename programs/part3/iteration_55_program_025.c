/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* ========== IDENTIFIER_NODE patterns ========== */
/* Global variables to force identifier creation */
int global_var_1;
double global_var_2;
char global_var_3;
volatile int volatile_global;

/* Function declarations that require identifier lookup */
extern int external_func_1(int);
extern void external_func_2(double);
extern char* external_func_3(void);

/* ========== TREE_VEC patterns ========== */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* ========== CONSTRUCTOR patterns ========== */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
};

union TestUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* ========== Function declarations ========== */
__attribute__((noinline)) int test_identifiers(void);
__attribute__((noinline)) int test_vectors(void);
__attribute__((noinline)) int test_ssa_names(int n);
__attribute__((noinline)) int test_blocks_and_scopes(void);
__attribute__((noinline)) int test_constructors(void);
__attribute__((noinline)) int test_openmp(int* arr, int n);

#ifdef __cplusplus
}
#endif

/* ========== C++ specific patterns for TREE_BINFO ========== */
#ifdef __cplusplus
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

__attribute__((noinline)) int test_binfo(void) {
    DerivedClass derived_obj;
    BaseClass* base_ptr = &derived_obj;
    return base_ptr->virtual_method();
}
#endif

/* ========== IDENTIFIER_NODE implementation ========== */
__attribute__((noinline)) int test_identifiers(void) {
    /* Local variables with different names */
    int local_identifier_1 = 1;
    double local_identifier_2 = 2.0;
    char local_identifier_3 = '3';
    
    /* Operations that require identifier lookup */
    int* addr_of_local = &local_identifier_1;
    size_t size_of_local = sizeof(local_identifier_2);
    
    /* Using global identifiers */
    global_var_1 = 100;
    global_var_2 = 200.0;
    global_var_3 = 'A';
    
    /* sizeof operations on globals */
    size_t size1 = sizeof(global_var_1);
    size_t size2 = sizeof(global_var_2);
    
    /* Address-of operations on globals */
    volatile int* volatile_ptr = &volatile_global;
    *volatile_ptr = 999;
    
    /* Complex expression with multiple identifiers */
    return (int)(local_identifier_1 + *addr_of_local + global_var_1);
}

/* ========== TREE_VEC implementation ========== */
__attribute__((noinline)) int test_vectors(void) {
#ifdef __GNUC__
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_c;
    
    /* Vector operations */
    vec_c = vec_a + vec_b;
    vec_c = vec_c * vec_a;
    
    /* Vector comparisons */
    v4si mask = vec_c > vec_b;
    
    /* Extract elements */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec_c[i];
    }
    
    /* Float vectors */
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_c = fvec_a * fvec_b;
    
    return sum + (int)fvec_c[0];
#else
    return 0;
#endif
}

/* ========== SSA_NAME implementation ========== */
__attribute__((noinline)) int test_ssa_names(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to create SSA variables */
    for (int i = 0; i < n; ++i) {
        x = x + i;  /* Creates SSA_NAME for x */
        y = y * (i + 1);  /* Creates SSA_NAME for y */
    }
    
    /* Another loop with different variable */
    for (int j = 0; j < n; ++j) {
        z = z - j;  /* Creates SSA_NAME for z */
        x = x + z;  /* Creates phi nodes */
    }
    
    /* Conditional updates */
    int w = 0;
    for (int k = 0; k < n; ++k) {
        if (k % 2 == 0) {
            w = w + k;  /* Creates SSA_NAME for w with phi */
        } else {
            w = w - k;  /* Different definition for w */
        }
    }
    
    return x + y + z + w;
}

/* ========== BLOCK implementation ========== */
__attribute__((noinline)) int test_blocks_and_scopes(void) {
    int result = 0;
    
    /* Level 1 block */
    {
        int block_var_1 = 10;
        
        /* Level 2 nested block */
        {
            int block_var_2 = 20;
            
            /* Level 3 nested block */
            {
                int block_var_3 = 30;
                result += block_var_1 + block_var_2 + block_var_3;
            }
        }
    }
    
    /* GCC statement expression (creates a block) */
    int stmt_expr_result = ({
        int temp_a = 5;
        int temp_b = 10;
        temp_a * temp_b;
    });
    result += stmt_expr_result;
    
    /* Labels and goto (involves block nodes) */
    void* label_ptr = &&my_label;
    
    if (result > 0) {
        goto *label_ptr;
    }
    
    return 0;
    
my_label:
    return result + 100;
}

/* ========== CONSTRUCTOR implementation ========== */
__attribute__((noinline)) int test_constructors(void) {
    /* Structure initializers */
    struct ComplexStruct s1 = {
        .int_field = 1,
        .float_field = 2.0f,
        .double_field = 3.0,
        .char_field = 'D'
    };
    
    /* Array initializers */
    int array_init[5] = {10, 20, 30, 40, 50};
    
    /* Nested structure initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = { .int_field = 5, .float_field = 6.0f },
        .extra = 7
    };
    
    /* Union initializer */
    union TestUnion u1 = { .as_int = 42 };
    union TestUnion u2 = { .as_float = 3.14f };
    
    /* Compound literals */
    int* dynamic_array = (int[]){1, 2, 3, 4, 5};
    struct ComplexStruct* dynamic_struct = &(struct ComplexStruct){
        .int_field = 100,
        .float_field = 200.0f
    };
    
    /* Designated array initializers */
    int sparse_array[10] = {[0] = 1, [5] = 2, [9] = 3};
    
    return s1.int_field + array_init[0] + nested.extra + u1.as_int;
}

/* ========== OMP_CLAUSE implementation ========== */
__attribute__((noinline)) int test_openmp(int* arr, int n) {
    int sum = 0;
    int i;
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr, n) reduction(+:sum) schedule(static)
    for (i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel
    {
        #pragma omp for reduction(max:max_val) nowait
        for (int j = 0; j < n; j++) {
            if (arr[j] > max_val) {
                max_val = arr[j];
            }
        }
        
        /* Nested parallel region */
        #pragma omp sections private(i)
        {
            #pragma omp section
            {
                int local_sum = 0;
                for (i = 0; i < n/2; i++) {
                    local_sum += arr[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp section
            {
                int local_sum = 0;
                for (i = n/2; i < n; i++) {
                    local_sum += arr[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
        }
    }
    
    /* OpenMP task with if clause */
    #pragma omp task if(n > 1000)
    {
        arr[0] = sum;
    }
    
    return sum + max_val;
}

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test all patterns */
    result += test_identifiers();
    
#ifdef __GNUC__
    result += test_vectors();
#endif
    
    result += test_ssa_names(100);
    result += test_blocks_and_scopes();
    result += test_constructors();
    
    /* Prepare data for OpenMP test */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
#ifdef _OPENMP
    result += test_openmp(arr, 100);
#endif
    
#ifdef __cplusplus
    result += test_binfo();
#endif
    
    /* Volatile output to prevent dead code elimination */
    volatile int final_output = result;
    
#ifdef __cplusplus
    std::cout << "Result: " << final_output << std::endl;
#else
    printf("Result: %d\n", final_output);
#endif
    
    return final_output > 0 ? 0 : 1;
}
