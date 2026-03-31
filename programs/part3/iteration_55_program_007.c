/* test_tree_nodes.c - Comprehensive test for tree node coverage */
#ifdef __cplusplus
#include <cstdio>
#include <cstdlib>
#else
#include <stdio.h>
#include <stdlib.h>
#endif

/* Pattern 1: IDENTIFIER_NODE - Global variables and operations */
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

/* Pattern 3: TREE_BINFO - C++ class inheritance */
#ifdef __cplusplus
class BaseClass {
public:
    virtual int method() { return 1; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int extra_method() { return 3; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern1(int n) {
    int x = 0;
    int y = 1;
    /* Loop that forces SSA for x and i */
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * 2;
    }
    
    /* Another loop with different variable */
    int z = x;
    for (int j = 0; j < 10; ++j) {
        z = z - j;
        x = x + z;
    }
    
    return x + y + z;
}

__attribute__((noinline))
int ssa_pattern2(int n) {
    int a = n;
    int b = 1;
    /* Complex loop with multiple assignments */
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            a = a + i;
            b = b - i;
        } else {
            a = a - i;
            b = b + i;
        }
    }
    return a * b;
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
            
            /* Level 3 block with statement expression (GCC extension) */
            result = ({
                int c = 30;
                int d = 40;
                a + b + c + d;
            });
            
            /* Label and goto */
            void* label_ptr = &&my_label;
            goto *label_ptr;
            
            my_label:
            result += 100;
        }
    }
    
    /* Another block with local variables */
    {
        int x = 5, y = 6, z = 7;
        result += x * y * z;
    }
    
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
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test"
    };
    
    /* Array initializer */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Compound literal */
    int sum = 0;
    int* ptr = (int[3]){arr[0], arr[1], arr[2]};
    
    /* Nested initializer */
    struct {
        struct {
            int x;
            int y;
        } point;
        int data[2];
    } nested = {
        .point = {.x = 1, .y = 2},
        .data = {3, 4}
    };
    
    /* Compute result using all constructors */
    sum = cs.id + (int)cs.values[0] + arr[2] + ptr[1] + nested.point.x;
    return sum;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return 0;
    
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
    
    /* OpenMP sections */
    #pragma omp parallel sections private(arr)
    {
        #pragma omp section
        {
            arr[0] = max_val;
        }
        #pragma omp section
        {
            arr[size-1] = sum % 100;
        }
    }
    
    free(arr);
    return sum + max_val;
}

/* Vector operations function */
__attribute__((noinline))
int vector_pattern(void) {
#ifdef __GNUC__
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;          /* Vector addition */
    v4si d = a * b;          /* Vector multiplication */
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;       /* Float vector multiplication */
    
    /* Extract results */
    int result = c[0] + c[1] + c[2] + c[3];
    result += d[0] + d[1];
    result += (int)f3[0] + (int)f3[1];
    
    return result;
#else
    return 42;  /* Fallback for non-GCC */
#endif
}

/* C++ specific pattern for BINFO */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern(void) {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* These operations involve BINFO nodes */
    int result = base_ptr->method();  /* Virtual call */
    result += derived.extra_method(); /* Direct call */
    
    /* Dynamic cast (involves type hierarchy inspection) */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += 10;
    }
    
    return result;
}
#endif

/* Main function that combines all patterns */
int main(int argc, char** argv) {
    volatile int final_result = 0;  /* Prevent optimization */
    
    /* Use identifiers in various ways */
    final_result += global_var1;
    final_result += (int)global_var2;
    final_result += global_var3;
    final_result += static_var;
    
    /* Take address of identifiers */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    final_result += *ptr1;
    final_result += (int)*ptr2;
    
    /* Use sizeof with identifiers */
    final_result += sizeof(global_var1);
    final_result += sizeof(global_var3);
    
    /* Call SSA pattern functions */
    final_result += ssa_pattern1(100);
    final_result += ssa_pattern2(50);
    
    /* Call block pattern */
    final_result += block_pattern();
    
    /* Call constructor pattern */
    final_result += constructor_pattern();
    
    /* Call vector pattern */
    final_result += vector_pattern();
    
#ifdef __cplusplus
    /* Call C++ BINFO pattern */
    final_result += binfo_pattern();
#endif
    
    /* Call OpenMP pattern */
    final_result += omp_pattern(1000);
    
    /* Prevent dead code elimination */
    if (final_result > 0) {
        printf("Final result: %d\n", final_result);
    } else {
        printf("Unexpected result\n");
    }
    
    return 0;
}
