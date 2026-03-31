/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and usage */
int global_var_1 = 10;
int global_var_2 = 20;
float global_var_3 = 3.14;
char global_var_4 = 'A';

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
    int extra_value = 100;
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
    
    int z = 0;
    for (int j = 0; j < n; ++j) {
        z = z - j;
        x = x + z;
    }
    
    return x + y;
}

__attribute__((noinline))
float ssa_pattern_2(int n) {
    float a = 1.0;
    float b = 2.0;
    
    for (int i = 0; i < n; ++i) {
        a = a * 1.1f;
        b = b / 1.1f;
        if (i % 2 == 0) {
            a = a + b;
        } else {
            b = b - a;
        }
    }
    
    return a + b;
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
            
            /* GCC statement expression */
            result += ({ 
                int temp = 5;
                temp * 2; 
            });
        }
        
        /* Another nested block with variables */
        {
            float f = 3.14;
            double d = 2.718;
            result += (int)(f + d);
        }
    }
    
    /* Label address and goto (creates block nodes) */
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
        int id;
        float values[4];
        char name[16];
        double extra;
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test",
        .extra = 99.99
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literal */
    int* arr = (int[5]){10, 20, 30, 40, 50};
    
    /* Nested structure initializer */
    struct Inner {
        int x, y;
    };
    
    struct Outer {
        struct Inner a, b;
        int flags;
    };
    
    struct Outer outer = {
        .a = {.x = 1, .y = 2},
        .b = {.x = 3, .y = 4},
        .flags = 0xFF
    };
    
    return cs.id + matrix[1][1] + arr[2] + outer.a.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = 0;
    
    #ifdef _OPENMP
    arr = (int*)__builtin_alloca(size * sizeof(int));
    
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
    
    sum += max_val;
    
    /* OpenMP sections */
    #pragma omp parallel sections private(arr)
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
    #endif
    
    return sum;
}

/* Vector pattern function */
__attribute__((noinline))
#ifdef __GNUC__
v4si vector_pattern(v4si a, v4si b) {
    v4si result = a + b;
    result = result * a;
    result = result - b;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fresult = fvec1 * fvec2;
    
    /* Use vector in conditional */
    v4si mask = a > b;
    result = (mask & result) | (~mask & a);
    
    return result;
}
#else
int vector_pattern(int a, int b) {
    return a + b;
}
#endif

/* C++ BINFO pattern function */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call through base pointer */
    int val1 = base_ptr->get_value();
    
    /* Dynamic cast */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    int val2 = derived_ptr ? derived_ptr->extra_value : 0;
    
    /* Reference to base */
    BaseClass& base_ref = derived;
    int val3 = base_ref.get_value();
    
    return val1 + val2 + val3;
}
#endif

/* Main function that combines all patterns */
int main(int argc, char** argv) {
    volatile int result = 0;  /* Prevent optimization */
    
    /* Use IDENTIFIER_NODE patterns */
    result += global_var_1;
    result += global_var_2;
    result += (int)global_var_3;
    result += global_var_4;
    
    /* Take addresses and use sizeof */
    result += (int)(&global_var_1 != 0);
    result += sizeof(global_var_2);
    
    /* SSA patterns */
    result += ssa_pattern_1(100);
    result += (int)ssa_pattern_2(50);
    
    /* Block pattern */
    result += block_pattern();
    
    /* Constructor pattern */
    result += constructor_pattern();
    
    /* Vector pattern */
    #ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec_result = vector_pattern(vec1, vec2);
    result += vec_result[0] + vec_result[1];
    #else
    result += vector_pattern(10, 20);
    #endif
    
    /* OpenMP pattern */
    result += omp_pattern(100);
    
    /* C++ BINFO pattern */
    #ifdef __cplusplus
    result += binfo_pattern();
    #endif
    
    /* Final volatile output to prevent dead code elimination */
    volatile int final_output = result;
    
    #ifdef __cplusplus
    std::cout << "Result: " << final_output << std::endl;
    #else
    printf("Result: %d\n", final_output);
    #endif
    
    return final_output != 0 ? 0 : 1;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
