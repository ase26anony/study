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
    virtual int method() { return 42; }
    virtual ~BaseClass() {}
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 84; }
};
#endif

/* Pattern 4: SSA_NAME - Functions with loops for SSA generation */
__attribute__((noinline))
int ssa_pattern1(int n) {
    int x = 0;
    int y = 1;
    for (int i = 0; i < n; ++i) {
        x = x + i;
        y = y * (i + 1);
    }
    return x + y;
}

__attribute__((noinline))
int ssa_pattern2(int n) {
    int sum = 0;
    int prod = 1;
    for (int i = 1; i <= n; ++i) {
        sum += i;
        prod *= i;
        if (sum > 1000) {
            sum = sum / 2;
        }
    }
    return sum * prod;
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
    
    /* GCC statement expression (creates a block) */
    int stmt_expr = ({
        int x = 5;
        int y = 10;
        x * y + 15;
    });
    
    result += stmt_expr;
    
    /* Labels and goto (involves block nodes) */
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
    };
    
    struct ComplexStruct cs = {
        .id = 1001,
        .values = {1.1f, 2.2f, 3.3f, 4.4f},
        .name = "test_struct"
    };
    
    /* Array initializer */
    int matrix[3][3] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    /* Compound literal */
    int* arr = (int[5]){10, 20, 30, 40, 50};
    
    /* Nested initializer */
    struct Nested {
        struct {
            int x, y;
        } point;
        int data[2];
    } nested = { .point = {.x = 1, .y = 2}, .data = {3, 4} };
    
    return cs.id + matrix[1][1] + arr[2] + nested.point.x;
}

/* Pattern 7: OMP_CLAUSE - OpenMP directives */
__attribute__((noinline))
int omp_pattern(int size) {
    int sum = 0;
    int* data = (int*)__builtin_alloca(size * sizeof(int));
    
    for (int i = 0; i < size; i++) {
        data[i] = i + 1;
    }
    
    /* Multiple OpenMP clauses */
    #pragma omp parallel for private(i) shared(data) reduction(+:sum) schedule(static)
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    
    /* Another OpenMP directive with different clauses */
    int max_val = 0;
    #pragma omp parallel for reduction(max:max_val)
    for (int i = 0; i < size; i++) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    
    /* OpenMP sections */
    int section_result = 0;
    #pragma omp parallel sections private(i) reduction(+:section_result)
    {
        #pragma omp section
        {
            for (int i = 0; i < size/2; i++) {
                section_result += data[i];
            }
        }
        
        #pragma omp section
        {
            for (int i = size/2; i < size; i++) {
                section_result += data[i];
            }
        }
    }
    
    return sum + max_val + section_result;
}

/* Vector operations function */
__attribute__((noinline))
int vector_pattern() {
#ifdef __GNUC__
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;
    v4si d = a * b;
    
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {0.5f, 1.5f, 2.5f, 3.5f};
    v4sf f3 = f1 * f2;
    
    /* Extract elements to force usage */
    int result = 0;
    for (int i = 0; i < 4; i++) {
        result += c[i] + d[i] + (int)f3[i];
    }
    return result;
#else
    return 0;
#endif
}

/* C++ specific patterns */
#ifdef __cplusplus
__attribute__((noinline))
int binfo_pattern() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call - involves BINFO */
    int result = base_ptr->method();
    
    /* Dynamic cast - involves BINFO */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        result += derived_ptr->method();
    }
    
    /* Multiple inheritance scenario */
    class Base2 {
    public:
        virtual int method2() { return 21; }
        virtual ~Base2() {}
    };
    
    class MultiDerived : public BaseClass, public Base2 {
    public:
        virtual int method() override { return 126; }
        virtual int method2() override { return 127; }
    };
    
    MultiDerived md;
    BaseClass* bc_ptr = &md;
    Base2* b2_ptr = &md;
    
    result += bc_ptr->method() + b2_ptr->method2();
    
    return result;
}
#endif

/* Main function that combines all patterns */
int main(int argc, char** argv) {
    volatile int final_result = 0;  /* volatile to prevent optimization */
    
    /* Use identifiers in various ways */
    final_result += global_var1;
    final_result += (int)global_var2;
    final_result += global_var3;
    final_result += (int)global_var4;
    
    /* Take addresses of identifiers */
    int* ptr1 = &global_var1;
    float* ptr2 = &global_var2;
    
    /* Use sizeof on identifiers */
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
    /* Call BINFO pattern (C++ only) */
    final_result += binfo_pattern();
#endif
    
    /* Call OpenMP pattern */
    final_result += omp_pattern(1000);
    
    /* Complex expression with multiple identifiers */
    int complex_expr = global_var1 * 2 + 
                      (int)(global_var2 / 2.0f) - 
                      global_var3 + 
                      (int)(global_var4 * 3.0);
    
    final_result += complex_expr;
    
    /* Additional identifier usage patterns */
    extern int external_func(int);  /* External declaration */
    (void)external_func;  /* Reference to suppress warning */
    
    /* Array of pointers to different identifiers */
    void* ptr_array[] = {&global_var1, &global_var2, &global_var3, &global_var4};
    final_result += (int)((long)ptr_array[0] % 1000);
    
#ifdef __cplusplus
    std::cout << "Final result: " << final_result << std::endl;
#else
    printf("Final result: %d\n", final_result);
#endif
    
    return final_result != 0 ? 0 : 1;
}

#ifdef __cplusplus
}  /* extern "C" */

/* Dummy external function */
int external_func(int x) {
    return x * 2;
}
#endif
