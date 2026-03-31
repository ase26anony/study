/* test_tree_nodes.c - Comprehensive test for GCC tree node coverage */
#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global and local variables with various uses */
int global_var1 = 10;
float global_var2 = 20.5;
char global_var3 = 'A';
static int static_var = 30;
extern int extern_func(int);  /* Forces identifier lookup */

/* Pattern 2: TREE_VEC - Vector extensions */
#ifdef __GNUC__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Pattern 3: TREE_BINFO - C++ class inheritance (only in C++ mode) */
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

class AnotherDerived : public BaseClass {
public:
    virtual int method1() override { return 3; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops that force SSA generation */
__attribute__((noinline))
int ssa_pattern1(int n) {
    int x = 0;
    int y = 1;
    /* Multiple SSA variables in loops */
    for (int i = 0; i < n; ++i) {
        x = x + i;      /* SSA for x */
        y = y * 2;      /* SSA for y */
    }
    
    int z = x;
    for (int j = 0; j < n; ++j) {
        z = z - j;      /* SSA for z */
        x = x + z;      /* More SSA for x */
    }
    
    return x + y + z;
}

__attribute__((noinline))
float ssa_pattern2(int n) {
    float a = 1.0f;
    float b = 2.0f;
    
    /* Nested loops create complex SSA */
    for (int i = 0; i < n; ++i) {
        a = a * 1.5f;
        for (int j = 0; j < i; ++j) {
            b = b + a;
            a = a - 0.1f;
        }
    }
    
    return a + b;
}

/* Pattern 5: BLOCK - Nested blocks and statement expressions */
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
            
            /* GCC statement expression creates a block */
            result += ({ 
                int temp = 5;
                temp * 2; 
            });
        }
        
        /* Another block with different variables */
        {
            float f = 3.14f;
            result += (int)f;
        }
    }
    
    /* Label and address-of-label (creates block nodes) */
    void* target = &&end_label;
    
    {
        int x = 100;
        result += x;
    }
    
end_label:
    return result;
}

/* Pattern 6: CONSTRUCTOR - Structure and array initializers */
__attribute__((noinline))
int constructor_pattern(void) {
    /* Structure with designated initializer */
    struct ComplexStruct {
        int id;
        float values[4];
        char name[16];
        double extra;
    };
    
    struct ComplexStruct cs = {
        .id = 42,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test",
        .extra = 99.99
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += ((int[]){1, 2, 3})[i];
    }
    
    /* Nested structure initializer */
    struct Inner {
        int a, b;
    };
    
    struct Outer {
        struct Inner i;
        int c;
    };
    
    struct Outer o = {
        .i = {.a = 1, .b = 2},
        .c = 3
    };
    
    return cs.id + arr[0] + sum + o.c;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives with various clauses */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP section with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(arr) firstprivate(size) reduction(max:max_val)
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
    
    free(arr);
    return sum + max_val;
}

/* Pattern 2 implementation: Vector operations */
__attribute__((noinline))
#ifdef __GNUC__
int vector_pattern(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c;
    
    /* Various vector operations */
    c = a + b;
    c = c * a;
    c = c - b;
    
    v4sf fa = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf fb = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf fc = fa * fb;
    
    /* Extract elements to ensure computation isn't optimized away */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += c[i];
        result += (int)fc[i];
    }
    
    return result;
}
#else
int vector_pattern(void) {
    /* Fallback for non-GCC compilers */
    int a[4] = {1, 2, 3, 4};
    int b[4] = {5, 6, 7, 8};
    int c[4];
    
    for (int i = 0; i < 4; i++) {
        c[i] = a[i] + b[i];
        c[i] = c[i] * a[i];
        c[i] = c[i] - b[i];
    }
    
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += c[i];
    }
    
    return result;
}
#endif

/* Pattern 3 implementation: C++ polymorphism */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    AnotherDerived another;
    BaseClass* base1 = &derived;
    BaseClass* base2 = &another;
    
    /* Use polymorphism to trigger BINFO lookups */
    int result = base1->method1() + base2->method1();
    
    /* Cast operations that may involve BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base1);
    if (derived_ptr) {
        result += derived_ptr->derived_data;
    }
    
    return result;
}
#endif

/* Main function that calls all patterns */
int main(int argc, char** argv) {
    volatile int result = 0;  /* Prevent optimization */
    
    /* Use global identifiers in various ways */
    result += global_var1;
    result += (int)global_var2;
    result += global_var3;
    result += static_var;
    
    /* Take address of identifiers */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    
    /* Use sizeof on identifiers */
    result += sizeof(global_var1);
    result += sizeof(global_var3);
    
    /* Call SSA pattern functions */
    result += ssa_pattern1(100);
    result += (int)ssa_pattern2(50);
    
    /* Call block pattern */
    result += block_pattern();
    
    /* Call constructor pattern */
    result += constructor_pattern();
    
    /* Call vector pattern */
    result += vector_pattern();
    
    /* Call OpenMP pattern */
    result += omp_pattern(1000);
    
    #ifdef __cplusplus
    /* Call C++ BINFO pattern */
    result += binfo_pattern();
    #endif
    
    /* Use pointer arithmetic with identifiers */
    result += *(ptr1 + 0);
    
    /* Final volatile store to ensure all code is live */
    volatile int final_result = result;
    
    #ifdef __cplusplus
    std::printf("Result: %d\n", final_result);
    #else
    printf("Result: %d\n", final_result);
    #endif
    
    return final_result > 0 ? 0 : 1;
}

/* External function declaration for identifier pattern */
extern int extern_func(int x) {
    return x * 2;
}
