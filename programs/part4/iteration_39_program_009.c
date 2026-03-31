/* test_tree_coverage.c - Comprehensive test to trigger all tree node types */
#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization */
#define NOOPT __attribute__((noinline))
#define VOLATILE volatile

/* For TREE_VEC - vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* For IDENTIFIER_NODE - generate many unique identifiers */
#define GEN_ID(n) identifier_##n
#define USE_ID(n) int GEN_ID(n) = n;

/* For BLOCK - nested scopes */
#define NESTED_BLOCK(level) { \
    VOLATILE int block_var_##level = level; \
    asm volatile("" : : : "memory"); \
}

/* For CONSTRUCTOR - aggregate initialization */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* For SSA_NAME - complex control flow */
NOOPT int ssa_test_function(int n) {
    VOLATILE int x = 0;
    int i;
    
    /* This creates SSA form due to phi nodes */
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = x + i * 2;
        } else {
            x = x - i;
        }
        
        /* Nested block for BLOCK nodes */
        NESTED_BLOCK(i);
    }
    
    /* Another SSA opportunity */
    int y = 0;
    for (i = 0; i < n; i++) {
        y = (y * 3 + i) % 100;
        if (y > 50) {
            y = y / 2;
        } else {
            y = y * 2;
        }
    }
    
    return x + y;
}

/* For TREE_VEC - vector operations */
NOOPT v4si vector_test(v4si a, v4si b) {
    /* Multiple vector operations */
    v4si c = a + b;
    v4si d = a * b;
    v4si e = c - d;
    v4si f = e << 2;
    
    /* Use volatile to prevent optimization */
    VOLATILE v4si result = f;
    asm volatile("" : : : "memory");
    
    return result;
}

/* For CONSTRUCTOR - complex initialization */
NOOPT struct NestedStruct constructor_test(int val) {
    /* Non-constant initializer with function calls */
    int computed = ssa_test_function(val % 10);
    
    /* Designated initializer with non-constant expressions */
    struct NestedStruct ns = {
        .inner = {
            .a = computed,
            .b = val * 2,
            .c = val + computed,
            .f = (float)val / 3.0f,
            .d = (double)computed * 1.5
        },
        .extra = computed % 100
    };
    
    /* Array with non-constant initializer */
    int arr[4] = { computed, val, computed * 2, val % 7 };
    
    /* Prevent optimization */
    asm volatile("" : : "r"(ns.inner.a), "r"(arr[0]) : "memory");
    
    return ns;
}

/* For OMP_CLAUSE - OpenMP constructs */
#ifdef _OPENMP
#include <omp.h>

NOOPT double omp_test(int size) {
    VOLATILE double sum = 0.0;
    int i;
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(i) shared(size) reduction(+:sum) schedule(dynamic) num_threads(4)
    for (i = 0; i < size; i++) {
        /* Nested block inside parallel region */
        {
            VOLATILE double local = i * 1.5;
            sum += local / (i + 1);
        }
    }
    
    /* Another OpenMP construct with different clauses */
    VOLATILE int max_val = 0;
    #pragma omp parallel sections private(i) reduction(max:max_val)
    {
        #pragma omp section
        {
            for (i = 0; i < 100; i++) {
                int temp = i * i;
                if (temp > max_val) max_val = temp;
            }
        }
        
        #pragma omp section
        {
            for (i = 100; i < 200; i++) {
                int temp = i * 2;
                if (temp > max_val) max_val = temp;
            }
        }
    }
    
    return sum + max_val;
}
#endif

/* For IDENTIFIER_NODE - generate many identifiers */
NOOPT void identifier_test(void) {
    /* Generate and use many unique identifiers */
    USE_ID(0); USE_ID(1); USE_ID(2); USE_ID(3); USE_ID(4);
    USE_ID(5); USE_ID(6); USE_ID(7); USE_ID(8); USE_ID(9);
    
    VOLATILE int result = 0;
    result += GEN_ID(0); result += GEN_ID(1); result += GEN_ID(2);
    result += GEN_ID(3); result += GEN_ID(4); result += GEN_ID(5);
    result += GEN_ID(6); result += GEN_ID(7); result += GEN_ID(8);
    result += GEN_ID(9);
    
    /* More identifiers in nested scopes */
    {
        int local_a = 1, local_b = 2, local_c = 3;
        int local_d = 4, local_e = 5, local_f = 6;
        result += local_a + local_b + local_c + local_d + local_e + local_f;
    }
    
    asm volatile("" : : "r"(result) : "memory");
}

/* C++ specific for TREE_BINFO */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

NOOPT int binfo_test() {
    DerivedClass d;
    BaseClass* b = &d;
    
    /* Use virtual function to ensure binfo is needed */
    VOLATILE int result = b->method();
    
    /* Multiple inheritance-like access */
    result += d.base_data;
    result += d.derived_data;
    
    asm volatile("" : : "r"(result) : "memory");
    return result;
}
#endif

/* Main function that ties everything together */
int main(int argc, char** argv) {
    VOLATILE int seed = (argc > 1) ? atoi(argv[1]) : 42;
    int result = 0;
    
    /* Test IDENTIFIER_NODE and BLOCK */
    identifier_test();
    
    /* Test SSA_NAME and BLOCK */
    result += ssa_test_function(seed);
    
    /* Test TREE_VEC */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = vector_test(vec_a, vec_b);
    
    /* Extract result from vector */
    VOLATILE int* vp = (VOLATILE int*)&vec_result;
    result += vp[0] + vp[1] + vp[2] + vp[3];
    
    /* Test CONSTRUCTOR */
    struct NestedStruct ns = constructor_test(seed);
    result += ns.inner.a + ns.inner.b + ns.inner.c + ns.extra;
    
    /* Test OMP_CLAUSE */
    #ifdef _OPENMP
    double omp_result = omp_test(seed % 100 + 50);
    result += (int)omp_result;
    #endif
    
    /* Test TREE_BINFO (C++ only) */
    #ifdef __cplusplus
    result += binfo_test();
    #endif
    
    /* Deeply nested blocks for more BLOCK nodes */
    {
        int a = 1;
        {
            int b = 2;
            {
                int c = 3;
                {
                    int d = 4;
                    result += a + b + c + d;
                    {
                        int e = 5;
                        result += e;
                    }
                }
            }
        }
    }
    
    /* Array constructor with non-constant size */
    int dyn = seed % 10 + 5;
    int arr[dyn];
    for (int i = 0; i < dyn; i++) {
        arr[i] = i * seed;
        result += arr[i];
    }
    
    printf("Result: %d\n", result);
    return result % 256;
}
