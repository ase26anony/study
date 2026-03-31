/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: IDENTIFIER_NODE - Global variables with various uses */
int global_var_1 = 10;
float global_var_2 = 20.5;
char global_var_3 = 'A';
double global_var_4 = 30.75;

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: CONSTRUCTOR - Structure and array initializers */
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

/* Pattern functions with noinline to ensure separate processing */
__attribute__((noinline)) 
int identifier_pattern(void) {
    /* Local identifiers with various operations */
    int local_var_1 = global_var_1;
    float local_var_2 = global_var_2;
    char local_var_3 = global_var_3;
    
    /* Operations that create identifier nodes */
    int* ptr1 = &global_var_1;
    float* ptr2 = &global_var_2;
    size_t sz1 = sizeof(global_var_3);
    size_t sz2 = sizeof(local_var_1);
    
    /* Function calls with identifiers */
    extern int dummy_extern_func(int);
    int result = dummy_extern_func(global_var_1) + dummy_extern_func(local_var_1);
    
    /* Complex expressions with identifiers */
    return (global_var_1 * local_var_1) + (int)(global_var_2 * local_var_2) + global_var_3;
}

__attribute__((noinline))
#ifdef __GNUC__
v4si vector_pattern(v4si a, v4si b) {
    /* Vector operations create TREE_VEC nodes */
    v4si result = a + b;
    v4si product = a * b;
    v4si shifted = result << 2;
    
    /* Mixed vector operations */
    v4sf float_vec = (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
    v4sf float_result = float_vec * 2.0f;
    
    /* Vector comparisons */
    v4si mask = result > product;
    
    return result + product + shifted;
}
#else
int vector_pattern(int a, int b) {
    /* Fallback for non-GCC compilers */
    return a + b;
}
#endif

__attribute__((noinline))
int ssa_pattern(int n) {
    /* Pattern 4: SSA_NAME - Loop with variable modification */
    int x = 0;
    int y = 1;
    
    /* Multiple loops to create SSA names */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    /* Nested loop with different variable */
    for (int j = 0; j < n/2; ++j) {
        int z = x;
        x = z + j;
        y = y - j;
    }
    
    /* Conditional that creates phi nodes */
    int result = (n > 0) ? x : y;
    
    /* Another SSA-intensive computation */
    int a = 1, b = 1;
    for (int k = 0; k < 10; ++k) {
        int temp = a;
        a = b;
        b = temp + b;  /* Fibonacci creates many SSA names */
    }
    
    return result + a;
}

__attribute__((noinline))
int block_pattern(void) {
    /* Pattern 5: BLOCK - Nested blocks and statement expressions */
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1 = 10;
        
        /* Level 2 block */
        {
            int level2 = level1 * 2;
            
            /* Level 3 block with statement expression (GCC extension) */
            int level3 = ({
                int temp = level2;
                temp * 3;
            });
            
            outer = level3;
        }
        
        /* Another block with different variables */
        {
            char block_char = 'z';
            float block_float = 3.14f;
            outer += (int)block_char + (int)block_float;
        }
    }
    
    /* Label address taking (creates BLOCK nodes) */
    void* target = &&end_block;
    
    /* Use computed goto */
    goto *target;
    
end_block:
    return outer;
}

__attribute__((noinline))
int constructor_pattern(void) {
    /* Pattern 6: CONSTRUCTOR - Various initializers */
    
    /* Structure with designated initializer */
    struct ComplexStruct s1 = {
        .int_field = 100,
        .float_field = 200.5f,
        .double_field = 300.75,
        .char_field = 'X'
    };
    
    /* Array initializer */
    int arr1[5] = {1, 2, 3, 4, 5};
    
    /* Nested structure initializer */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = {
        .inner = { .int_field = 50, .float_field = 60.5f, 
                   .double_field = 70.25, .char_field = 'Y' },
        .extra = 99
    };
    
    /* Compound literal */
    int* dynamic_arr = (int[3]){10, 20, 30};
    
    /* Union initializer */
    union MixedUnion u1 = { .as_int = 42 };
    union MixedUnion u2 = { .as_float = 3.14159f };
    
    /* Multi-dimensional array initializer */
    int matrix[2][3] = { {1, 2, 3}, {4, 5, 6} };
    
    return s1.int_field + arr1[2] + nested.extra + dynamic_arr[1] + u1.as_int;
}

#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int* arr, int n) {
    /* Pattern 7: OMP_CLAUSE - Various OpenMP directives */
    int sum = 0;
    int product = 1;
    int max_val = arr[0];
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel for private(n) shared(arr) reduction(+:sum) reduction(*:product) schedule(static, 4)
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
        product *= (arr[i] > 0) ? arr[i] : 1;
    }
    
    /* Another parallel region with different clauses */
    #pragma omp parallel reduction(max:max_val)
    {
        #pragma omp for
        for (int i = 0; i < n; ++i) {
            if (arr[i] > max_val) {
                max_val = arr[i];
            }
        }
    }
    
    /* Sections with private/firstprivate */
    #pragma omp parallel sections private(sum)
    {
        #pragma omp section
        {
            sum = 0;
            for (int i = 0; i < n/2; ++i) sum += arr[i];
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            for (int i = n/2; i < n; ++i) local_sum += arr[i];
            #pragma omp atomic
            sum += local_sum;
        }
    }
    
    /* Task with depend clause */
    #pragma omp task depend(in: arr) depend(out: sum)
    {
        sum = sum * 2;
    }
    
    return sum + product + max_val;
}
#else
__attribute__((noinline))
int omp_pattern(int* arr, int n) {
    /* Fallback without OpenMP */
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}
#endif

/* C++ specific patterns for TREE_BINFO */
#ifdef __cplusplus

class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 84; }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int method() override { return 168; }
    int second_data;
};

__attribute__((noinline))
int binfo_pattern(void) {
    /* Pattern for TREE_BINFO - Class hierarchy usage */
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int result = base_ptr->method();
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->derived_data;
    }
    
    /* Multiple inheritance-like access */
    SecondDerived second;
    BaseClass* base_ptr2 = &second;
    result += base_ptr2->method();
    
    /* Typeid (requires RTTI) */
    result += (typeid(*base_ptr) == typeid(DerivedClass)) ? 1 : 0;
    
    return result;
}

#endif

int main(void) {
    volatile int checksum = 0;
    
    /* Call all pattern functions */
    checksum += identifier_pattern();
    
#ifdef __GNUC__
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_pattern(vec_a, vec_b);
    checksum += vec_result[0] + vec_result[1] + vec_result[2] + vec_result[3];
#endif
    
    checksum += ssa_pattern(100);
    checksum += block_pattern();
    checksum += constructor_pattern();
    
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    checksum += omp_pattern(arr, 10);
    
#ifdef __cplusplus
    checksum += binfo_pattern();
#endif
    
    /* Use checksum to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    return 0;
}

#ifdef __cplusplus
}
#endif
