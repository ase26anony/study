/* test_tree_nodes.c - Comprehensive test to trigger tree_kind classification */
#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
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
    virtual ~BaseClass() {}
    virtual int get_value() { return 42; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int get_value() { return 84; }
    int derived_data;
};

class AnotherDerived : public BaseClass {
public:
    virtual int get_value() { return 168; }
    int another_data;
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern_function(int n) {
    int x = 0;
    int y = 1;
    int z = 2;
    
    /* Multiple loops to generate SSA_NAME nodes */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    
    for (int j = 0; j < n * 2; ++j) {
        z = z - j;
        x = x ^ z;
    }
    
    int w = 0;
    while (w < n) {
        y = y + w;
        w = w + 1;
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
                temp = temp * 3;
                temp;
            });
        }
    }
    
    /* Another block with label address */
    {
        void* target = &&end_block;
        goto *target;
        
        dead_code:
            result = 0;
        
        end_block:
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
    };
    
    struct ComplexStruct s1 = { .a = 1, .b = 2.0f, .c = 3.0, .d = 'X' };
    struct ComplexStruct s2 = { 4, 5.0f, 6.0, 'Y' };
    
    /* Array initializers */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[3][2] = {{1, 2}, {3, 4}, {5, 6}};
    
    /* Compound literals */
    int* ptr1 = (int[3]){10, 20, 30};
    struct ComplexStruct* ptr2 = &(struct ComplexStruct){ .a = 100, .b = 200.0f };
    
    /* Nested initializers */
    struct Outer {
        struct Inner {
            int x;
            int y;
        } inner;
        int z;
    } outer = { .inner = { .x = 7, .y = 8 }, .z = 9 };
    
    return s1.a + arr1[0] + ptr1[1] + outer.inner.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives with various clauses */
__attribute__((noinline))
int omp_pattern_function(int size) {
    int sum = 0;
    int product = 1;
    int* arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP directive with different clauses */
    #pragma omp parallel sections private(product)
    {
        #pragma omp section
        {
            product = 1;
            for (int j = 0; j < size/2; j++) {
                product *= arr[j];
            }
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            #pragma omp parallel for reduction(+:local_sum)
            for (int k = size/2; k < size; k++) {
                local_sum += arr[k];
            }
            sum += local_sum;
        }
    }
    
    /* OpenMP critical section */
    #pragma omp parallel
    {
        #pragma omp critical
        {
            sum += 1000;
        }
    }
    
    free(arr);
    return sum + product;
}

/* Pattern 2: Vector operations function */
__attribute__((noinline))
int vector_pattern_function(void) {
#ifdef __GNUC__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    
    v4sf fvec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fvec2 = {0.5f, 0.5f, 0.5f, 0.5f};
    v4sf fvec3 = fvec1 * fvec2;
    
    /* Use vectors in operations */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += vec3[i];
        result += (int)fvec3[i];
    }
    
    return result;
#else
    return 42; /* Fallback for non-GCC compilers */
#endif
}

/* Pattern 3: C++ polymorphism function */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern_function(void) {
    DerivedClass derived;
    AnotherDerived another;
    BaseClass* base1 = &derived;
    BaseClass* base2 = &another;
    
    /* Use polymorphism to trigger BINFO lookups */
    int result = base1->get_value() + base2->get_value();
    
    /* Cast operations that may involve BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base1);
    if (derived_ptr) {
        result += derived_ptr->derived_data;
    }
    
    return result;
}
#endif

/* Pattern 1: Identifier operations function */
__attribute__((noinline))
int identifier_pattern_function(void) {
    /* Operations on global identifiers */
    int* addr1 = &global_var1;
    int* addr2 = &global_var2;
    float* addr3 = &global_var3;
    
    /* sizeof operations on identifiers */
    size_t size1 = sizeof(global_var1);
    size_t size2 = sizeof(global_var2);
    size_t size3 = sizeof(global_var3);
    size_t size4 = sizeof(global_var4);
    size_t size5 = sizeof(global_var5);
    
    /* Use identifiers in expressions */
    int result = global_var1 + global_var2;
    result += (int)global_var3;
    result += (int)global_var4;
    result += global_var5;
    
    /* Address arithmetic */
    result += *(addr1 + 0);
    result += (int)(addr3 - addr3);
    
    return result + size1 + size2 + size3 + size4 + size5;
}

/* Main function that combines all patterns */
int main(int argc, char** argv) {
    volatile int final_result = 0; /* volatile to prevent optimization */
    
    /* Call all pattern functions */
    final_result += identifier_pattern_function();
    final_result += vector_pattern_function();
    
#ifdef __cplusplus
    final_result += binfo_pattern_function();
#endif
    
    final_result += ssa_pattern_function(100);
    final_result += block_pattern_function(50);
    final_result += constructor_pattern_function();
    
    /* Only call OpenMP function if OpenMP is supported */
#ifdef _OPENMP
    final_result += omp_pattern_function(1000);
#endif
    
    /* Use the result to prevent dead code elimination */
    printf("Final checksum: %d\n", final_result);
    
    /* Also use command line arguments as identifiers */
    if (argc > 1) {
        for (int i = 0; i < argc; i++) {
            printf("Arg %d: %s\n", i, argv[i]);
        }
    }
    
    return final_result != 0 ? 0 : 1;
}
