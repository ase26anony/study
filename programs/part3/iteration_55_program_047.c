/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */

#ifdef __cplusplus
#include <iostream>
extern "C" {
#else
#include <stdio.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
int global_var1 = 10;
float global_var2 = 20.5;
char global_var3 = 'A';
double global_var4 = 30.75;

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class hierarchy (only in C++ mode) */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int method1() { return 1; }
    virtual ~BaseClass() {}
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method1() override { return 2; }
    int derived_data;
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern_function(int n) {
    int x = 0;
    int y = 1;
    
    /* Multiple loops to create SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        x = x + i;  /* Creates SSA for x and i */
    }
    
    for (int j = 0; j < n; ++j) {
        y = y * (j + 1);  /* Creates SSA for y and j */
    }
    
    int z = 0;
    while (z < n) {
        z = z + 2;  /* Creates SSA for z */
    }
    
    return x + y + z;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
__attribute__((noinline))
int block_pattern_function(int val) {
    int result = val;
    
    /* Level 1 block */
    {
        int inner1 = result * 2;
        
        /* Level 2 block */
        {
            int inner2 = inner1 + 5;
            
            /* Level 3 block with statement expression (GCC extension) */
            result = ({
                int temp = inner2;
                temp * 3;
            });
        }
    }
    
    /* Another block with label address */
    {
        void* label_ptr = &&my_label;
        goto *label_ptr;
        
        my_label:
        result += 10;
    }
    
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern_function(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int a;
        float b;
        double c;
        char d;
    } s1 = { .a = 42, .b = 3.14f, .c = 2.71828, .d = 'X' };
    
    /* Array initializer */
    int arr[5] = {1, 2, 3, 4, 5};
    
    /* Compound literal */
    int* ptr = (int[3]){10, 20, 30};
    
    /* Nested structure initializer */
    struct Nested {
        struct { int x; int y; } point;
        int value;
    } nested = { .point = {.x = 1, .y = 2}, .value = 100 };
    
    return s1.a + arr[2] + ptr[1] + nested.value;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern_function(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size && i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP section */
    int max_val = 0;
    #pragma omp parallel sections private(i) reduction(max:max_val)
    {
        #pragma omp section
        {
            for (int i = 0; i < size/2; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = size/2; i < size; i++) {
                if (arr[i] > max_val) max_val = arr[i];
            }
        }
    }
    
    return sum + max_val;
}

/* Pattern 2: Vector operations function */
__attribute__((noinline))
int vector_pattern_function(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;  /* Vector addition */
    v4si vec4 = vec1 * vec2;  /* Vector multiplication */
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Use vectors in expressions */
    int result = vec3[0] + vec3[1] + vec3[2] + vec3[3];
    result += (int)(fvec3[0] + fvec3[1] + fvec3[2] + fvec3[3]);
    
    return result;
#else
    return 0;
#endif
}

/* Pattern 3: C++ inheritance pattern (only in C++) */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern_function(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Access through base pointer - involves BINFO */
    int result = base_ptr->method1();
    result += derived.method1();
    
    /* Cast operations that involve BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += 10;
    }
    
    return result;
}
#endif

/* Main function that calls all patterns */
int main(void) {
    volatile int final_result = 0;  /* volatile to prevent optimization */
    
    /* Use IDENTIFIER_NODE patterns */
    final_result += global_var1;
    final_result += (int)global_var2;
    final_result += global_var3;
    final_result += (int)global_var4;
    
    /* Take addresses and use sizeof to force identifier lookups */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    size_t s1 = sizeof(global_var3);
    size_t s2 = sizeof(global_var4);
    final_result += *ptr1 + (int)*ptr2 + (int)s1 + (int)s2;
    
    /* Call pattern functions */
    final_result += ssa_pattern_function(10);
    final_result += block_pattern_function(5);
    final_result += constructor_pattern_function();
    final_result += vector_pattern_function();
    
    /* OpenMP pattern */
    #ifdef _OPENMP
    final_result += omp_pattern_function(50);
    #endif
    
    /* C++ BINFO pattern */
    #ifdef __cplusplus
    final_result += binfo_pattern_function();
    #endif
    
    /* Prevent dead code elimination */
    #ifdef __cplusplus
    std::cout << "Final result: " << final_result << std::endl;
    #else
    printf("Final result: %d\n", final_result);
    #endif
    
    return final_result != 0 ? 0 : 1;
}

#ifdef __cplusplus
}  /* extern "C" */
#endif
