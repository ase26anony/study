/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */
#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var_1 = 10;
int global_var_2 = 20;
float global_var_3 = 3.14;
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
int ssa_pattern_1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    return x + y;
}

__attribute__((noinline))
int ssa_pattern_2(int n) {
    int a = 1, b = 2, c = 3;
    for (int i = 0; i < n; ++i) {
        a = b + c;
        b = c - a;
        c = a * b;
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
    int val = ({
        int x = 5;
        int y = 10;
        x * y + 15;
    });
    
    result += val;
    
    /* Label address and goto */
    void* label_addr = &&my_label;
    goto *label_addr;
    
my_label:
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern() {
    /* Structure with designated initializers */
    struct ComplexStruct {
        int id;
        float data[4];
        char name[16];
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .data = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test_struct"
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literal */
    int sum = 0;
    int* arr = (int[5]){10, 20, 30, 40, 50};
    for (int i = 0; i < 5; i++) {
        sum += arr[i];
    }
    
    return cs.id + matrix[1][1] + sum;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
#ifdef _OPENMP
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)__builtin_alloca(size * sizeof(int));
    
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum)
    for (int i = 0; i < size; i++) {
        arr[i] = i * 2;
        sum += arr[i];
    }
    
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val)
    for (int i = 0; i < size; i++) {
        if (arr[i] > max_val) {
            max_val = arr[i];
        }
    }
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            arr[0] = 1;
        }
        #pragma omp section
        {
            arr[1] = 2;
        }
    }
    
    return sum + max_val;
}
#endif

/* Vector operations function */
__attribute__((noinline))
int vector_pattern() {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Extract elements to force usage */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += vec3[i] + vec4[i] + (int)fvec3[i];
    }
    return sum;
#else
    return 0;
#endif
}

/* C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int cpp_binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* This should involve TREE_BINFO nodes */
    int val1 = base_ptr->get_value();
    int val2 = derived.get_value();
    
    /* Try dynamic_cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    int val3 = derived_ptr ? derived_ptr->get_value() : 0;
    
    return val1 + val2 + val3;
}
#endif

/* Main function that combines all patterns */
int main() {
    volatile int result = 0;  /* volatile to prevent optimization */
    
    /* Use IDENTIFIER_NODE patterns */
    result += global_var_1;
    result += global_var_2;
    result += (int)global_var_3;
    result += (int)global_var_4;
    result += global_var_5;
    
    /* Take addresses to force identifier lookups */
    void* addr1 = &global_var_1;
    void* addr2 = &global_var_2;
    (void)addr1; (void)addr2;  /* Prevent unused warning */
    
    /* Use sizeof on identifiers */
    result += sizeof(global_var_3);
    result += sizeof(global_var_4);
    
    /* Call SSA pattern functions */
    result += ssa_pattern_1(100);
    result += ssa_pattern_2(50);
    
    /* Call block pattern */
    result += block_pattern();
    
    /* Call constructor pattern */
    result += constructor_pattern();
    
    /* Call vector pattern */
    result += vector_pattern();
    
#ifdef _OPENMP
    /* Call OpenMP pattern */
    result += omp_pattern(100);
#endif
    
#ifdef __cplusplus
    /* Call C++ pattern */
    result += cpp_binfo_pattern();
#endif
    
    /* Final output to prevent dead code elimination */
#ifdef __cplusplus
    std::cout << "Result: " << result << std::endl;
#else
    printf("Result: %d\n", result);
#endif
    
    return 0;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
