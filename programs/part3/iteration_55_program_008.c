/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

/* Enable C++ features for TREE_BINFO */
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif

/* Global variables for IDENTIFIER_NODE coverage */
int global_var1 = 10;
float global_var2 = 20.5;
double global_var3 = 30.7;
char global_var4 = 'A';

/* Function prototypes for IDENTIFIER_NODE usage */
extern void external_func1(int);
extern double external_func2(float, char);

/* Vector types for TREE_VEC coverage */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Structure for CONSTRUCTOR coverage */
struct ComplexStruct {
    int int_field;
    float float_field;
    double double_field;
    char char_field;
};

/* Nested structure for complex constructors */
struct NestedStruct {
    struct ComplexStruct inner;
    int array[3];
};

/* Function using IDENTIFIER_NODES extensively */
__attribute__((noinline))
int use_identifiers(int param) {
    /* Local variables */
    int local1 = global_var1;
    float local2 = global_var2;
    double local3 = global_var3;
    char local4 = global_var4;
    
    /* Operations that create identifier nodes */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    size_t sz1 = sizeof(global_var3);
    size_t sz2 = sizeof(global_var4);
    
    /* Use in expressions */
    int result = local1 + *ptr1 + param;
    result += (int)local2 + (int)(*ptr2);
    
    /* Function calls with identifiers */
    external_func1(result);
    double d = external_func2(local2, local4);
    
    return result + (int)d;
}

/* Function using TREE_VEC nodes */
#ifdef __GNUC__
__attribute__((noinline))
v4si use_vectors(v4si a, v4si b) {
    v4si result;
    
    /* Vector operations */
    result = a + b;
    result = result * a;
    result = result - b;
    
    /* Vector comparisons */
    v4si mask = result > a;
    result = result & mask;
    
    return result;
}

__attribute__((noinline))
v4sf use_float_vectors(v4sf a, v4sf b) {
    v4sf result = a + b;
    result = result * b;
    return result;
}
#endif

/* Function with SSA_NAME generation */
__attribute__((noinline))
int ssa_heavy_computation(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops creating SSA variables */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;
        x = x + z;
    }
    
    /* Complex control flow */
    int w = 0;
    while (w < n) {
        y = y + w;
        if (y > 100) {
            x = x - w;
        } else {
            x = x + w;
        }
        w++;
    }
    
    return x + y + z;
}

/* Function with BLOCK nodes */
__attribute__((noinline))
int nested_blocks(int val) {
    int result = val;
    
    /* Level 1 block */
    {
        int block_var1 = result * 2;
        
        /* Level 2 block */
        {
            int block_var2 = block_var1 + 10;
            
            /* Level 3 block with statement expression */
            result = ({
                int temp = block_var2;
                temp = temp * 3;
                temp;
            });
        }
    }
    
    /* Another block with label address */
    {
        void* target = &&exit_block;
        result += 5;
        goto *target;
        
        exit_block:
        result -= 2;
    }
    
    return result;
}

/* Function with CONSTRUCTOR nodes */
__attribute__((noinline))
struct ComplexStruct use_constructors(void) {
    /* Structure initializer */
    struct ComplexStruct s1 = {
        .int_field = 42,
        .float_field = 3.14f,
        .double_field = 2.71828,
        .char_field = 'Z'
    };
    
    /* Array initializer */
    int arr1[5] = {1, 2, 3, 4, 5};
    
    /* Nested structure initializer */
    struct NestedStruct ns = {
        .inner = {
            .int_field = 100,
            .float_field = 1.5f,
            .double_field = 3.5,
            .char_field = 'X'
        },
        .array = {10, 20, 30}
    };
    
    /* Compound literals */
    int* ptr = (int[3]){7, 8, 9};
    s1.int_field += ptr[0];
    
    /* Designated initializer in array */
    float arr2[10] = {
        [0] = 1.0f,
        [5] = 2.0f,
        [9] = 3.0f
    };
    
    s1.float_field += arr2[5];
    
    return s1;
}

/* OpenMP functions for OMP_CLAUSE coverage */
#ifdef _OPENMP
__attribute__((noinline))
int openmp_reduction(int* arr, int n) {
    int sum = 0;
    
    #pragma omp parallel for reduction(+:sum) private(n) shared(arr)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    
    return sum;
}

__attribute__((noinline))
void openmp_sections(void) {
    int x = 0, y = 0;
    
    #pragma omp parallel sections private(x) firstprivate(y)
    {
        #pragma omp section
        {
            x = 1;
            y = x + 2;
        }
        
        #pragma omp section
        {
            x = 3;
            y = x * 2;
        }
    }
}

__attribute__((noinline))
void openmp_task(void) {
    int shared_var = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task shared(shared_var)
            {
                shared_var = 42;
            }
            
            #pragma omp taskwait
        }
    }
}
#endif

/* C++ specific code for TREE_BINFO coverage */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int virtual_method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int virtual_method() override { return 2; }
    int derived_data;
};

__attribute__((noinline))
int use_inheritance(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int result = base_ptr->virtual_method();
    
    /* Access through reference */
    BaseClass& base_ref = derived;
    result += base_ref.virtual_method();
    
    /* Typeid might involve BINFO */
    result += sizeof(DerivedClass);
    
    return result;
}
#endif

/* Main function that ties everything together */
int main(void) {
    volatile int checksum = 0;
    
    /* 1. IDENTIFIER_NODE coverage */
    checksum += use_identifiers(42);
    
    /* 2. TREE_VEC coverage */
    #ifdef __GNUC__
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = use_vectors(vec_a, vec_b);
    checksum += vec_result[0] + vec_result[1];
    
    v4sf fvec_a = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec_b = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec_result = use_float_vectors(fvec_a, fvec_b);
    checksum += (int)fvec_result[0];
    #endif
    
    /* 3. SSA_NAME coverage */
    checksum += ssa_heavy_computation(50);
    
    /* 4. BLOCK coverage */
    checksum += nested_blocks(100);
    
    /* 5. CONSTRUCTOR coverage */
    struct ComplexStruct cs = use_constructors();
    checksum += cs.int_field + (int)cs.float_field;
    
    /* 6. OpenMP coverage */
    #ifdef _OPENMP
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    checksum += openmp_reduction(arr, 100);
    openmp_sections();
    openmp_task();
    #endif
    
    /* 7. C++ inheritance coverage */
    #ifdef __cplusplus
    checksum += use_inheritance();
    #endif
    
    /* Prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    return checksum > 0 ? 0 : 1;
}

/* Dummy external functions for IDENTIFIER_NODE */
void external_func1(int x) {
    /* Do nothing */
}

double external_func2(float f, char c) {
    return (double)f + (double)c;
}
