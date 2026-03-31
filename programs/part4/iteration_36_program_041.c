/* tree_coverage.c - Comprehensive test to cover tree_kind switch cases */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* ==================== 1. IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3;
float special_float_var;
char char_identifier;

typedef int my_type_1, my_type_2, my_type_3;
typedef double dbl_type_1, dbl_type_2;
typedef struct { int x; } struct_type_id;

/* Function with label identifiers */
void func_with_labels(void) {
    label_1: ;
    label_2: ;
    label_3: ;
    goto label_1;
}

/* More identifiers in different scopes */
namespace my_namespace {
    int ns_var;
    void ns_func() {}
}

/* ==================== 2. TREE_VEC generation ==================== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned g, signed char h, int* i, void* j);

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4];
float another_array[5][6][7][2];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Function with complex return type */
int (*func_ptr_array[10])(int, float, double);

/* ==================== 3. TREE_BINFO generation (C++ only) ==================== */
#ifdef __cplusplus
/* Base classes */
struct Base1 {
    int base1_data;
    virtual void base1_func() {}
    virtual ~Base1() {}
};

struct Base2 {
    float base2_data;
    virtual void base2_func() {}
    virtual ~Base2() {}
};

/* Single inheritance */
struct Derived1 : public Base1 {
    int derived1_data;
    void base1_func() override {}
};

/* Multiple inheritance */
struct Derived2 : public Base1, public Base2 {
    double derived2_data;
    void base1_func() override {}
    void base2_func() override {}
};

/* Virtual inheritance */
struct VirtualBase {
    int virtual_data;
    virtual void virt_func() = 0;
    virtual ~VirtualBase() {}
};

struct VirtDerived1 : virtual public VirtualBase {
    void virt_func() override {}
};

struct VirtDerived2 : virtual public VirtualBase {
    void virt_func() override {}
};

struct Diamond : public VirtDerived1, public VirtDerived2 {
    void virt_func() override {}
};

/* Template with inheritance */
template<typename T>
struct TemplateBase {
    T data;
};

template<typename T>
struct TemplateDerived : public TemplateBase<T> {
    T extra_data;
};
#endif

/* ==================== 4. SSA_NAME generation ==================== */
/* Function with complex control flow to generate SSA names */
int ssa_generator(int x) {
    volatile int y = x;  /* volatile prevents optimization */
    int z = 0;
    
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    /* Loop with SSA */
    for (int i = 0; i < y; ++i) {
        z += i;
        if (z > 100) {
            z = z / 2;
        }
    }
    
    /* Nested loops */
    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k < j; ++k) {
            z += k * j;
        }
    }
    
    /* Switch statement */
    switch (z) {
        case 0: y = 1; break;
        case 1: y = 2; break;
        default: y = 3; break;
    }
    
    return z + y;
}

/* Another SSA-intensive function */
float complex_ssa(float a, float b) {
    volatile float x = a;
    float y = b;
    float z = 0.0f;
    
    for (int i = 0; i < 100; i++) {
        x = x * 1.1f;
        y = y + 0.5f;
        z = z + x * y;
        
        if (z > 1000.0f) {
            z = z / 2.0f;
            y = y - 1.0f;
        }
    }
    
    return z;
}

/* ==================== 5. BLOCK generation ==================== */
/* Function with nested blocks */
void block_generator(void) {
    /* Outer block */
    int outer_var = 0;
    
    {
        /* Inner block 1 */
        int inner_var_1 = 1;
        {
            /* Deeper block */
            int deeper_var = inner_var_1 * 2;
            outer_var += deeper_var;
        }
    }
    
    {
        /* Inner block 2 */
        int inner_var_2 = 2;
        if (outer_var > 0) {
            /* Conditional block */
            int conditional_var = inner_var_2 * 3;
            outer_var += conditional_var;
        }
    }
    
    /* Loop with block */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * i;
        outer_var += loop_var;
    }
    
    /* Label address taking */
    void* label_ptr = 0;
    my_label: 
    label_ptr = &&my_label;
    
    /* Use label pointer to prevent optimization */
    if (label_ptr) {
        goto *label_ptr;
    }
}

/* More block examples */
void another_block_func(int n) {
    /* Try-catch in C++ */
    #ifdef __cplusplus
    try {
        int try_var = n * 2;
        if (try_var > 100) {
            throw try_var;
        }
    } catch (int e) {
        int catch_var = e / 2;
    }
    #endif
    
    /* Multiple scopes */
    for (int i = 0; i < n; i++) {
        int scope1 = i;
        {
            int scope2 = scope1 + 1;
            {
                int scope3 = scope2 * 2;
            }
        }
    }
}

/* ==================== 6. CONSTRUCTOR generation ==================== */
/* Struct with initializers */
struct ComplexStruct {
    int a;
    double b;
    float c;
    char d;
    int* e;
};

/* Union with initializers */
union DataUnion {
    int i;
    float f;
    double d;
    char str[16];
};

/* Array with complex initializer */
int init_array[10] = {0, 1, 2, [7] = 7, [9] = 9};

/* Struct initializers */
struct ComplexStruct cs1 = {1, 2.0, 3.0f, 'A', NULL};
struct ComplexStruct cs2 = {.a = 10, .b = 20.5, .c = 30.5f, .d = 'B'};
struct ComplexStruct cs3 = {.b = 100.0, .a = 50, .d = 'C', .c = 75.0f};

/* Nested struct */
struct OuterStruct {
    struct ComplexStruct inner;
    int extra;
};

struct OuterStruct os1 = {{1, 2.0, 3.0f, 'D', NULL}, 100};
struct OuterStruct os2 = {.inner = {.a = 5, .b = 6.0}, .extra = 200};

/* Union initializers */
union DataUnion du1 = {.i = 42};
union DataUnion du2 = {.f = 3.14159f};
union DataUnion du3 = {.d = 2.71828};

/* Array of structs with initializer */
struct ComplexStruct struct_array[3] = {
    {1, 1.0, 1.0f, 'X'},
    {2, 2.0, 2.0f, 'Y'},
    {3, 3.0, 3.0f, 'Z'}
};

/* Multi-dimensional array initializer */
int md_array[2][3] = {{1, 2, 3}, {4, 5, 6}};
int md_array_partial[3][4] = {[0][0] = 1, [1][1] = 2, [2][2] = 3};

/* ==================== 7. OMP_CLAUSE generation ==================== */
/* OpenMP parallel region with various clauses */
void openmp_test(int size) {
    int i;
    int sum = 0;
    int private_var = 0;
    int shared_var = 0;
    int reduction_sum = 0;
    int arr[1000];
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        arr[i] = i;
    }
    
    /* Various OpenMP pragmas */
    #pragma omp parallel private(private_var) shared(shared_var, arr) firstprivate(size)
    {
        private_var = omp_get_thread_num();
        shared_var += private_var;
    }
    
    /* Parallel for with reduction */
    #pragma omp parallel for reduction(+:reduction_sum) schedule(static, 10)
    for (i = 0; i < 1000; i++) {
        reduction_sum += arr[i];
    }
    
    /* SIMD directive */
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (i = 0; i < 1000; i++) {
        sum += arr[i] * 2;
    }
    
    /* Sections */
    #pragma omp parallel sections private(private_var)
    {
        #pragma omp section
        {
            private_var = 1;
        }
        #pragma omp section
        {
            private_var = 2;
        }
    }
    
    /* Task with depend clause */
    int x = 0, y = 0;
    #pragma omp task depend(inout: x) shared(x)
    {
        x = 100;
    }
    
    #pragma omp task depend(in: x) depend(out: y) shared(x, y)
    {
        y = x * 2;
    }
    
    /* Parallel for with collapse */
    int matrix[10][10];
    #pragma omp parallel for collapse(2)
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            matrix[row][col] = row * col;
        }
    }
    
    printf("OpenMP results: reduction_sum=%d, sum=%d\n", reduction_sum, sum);
}

/* ==================== Main driver ==================== */
int main(int argc, char** argv) {
    int result;
    
    /* Use identifiers */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Use TREE_VEC constructs */
    result = complex_func(1, 2L, 'a', 3.0, 4, 5.0f, 6, 'b', &result, NULL);
    multi_dim_array[0][0][0] = 100;
    
    /* Use SSA-intensive functions */
    int ssa_result = ssa_generator(argc);
    float ssa_float = complex_ssa(1.0f, 2.0f);
    
    /* Use block generator */
    block_generator();
    another_block_func(10);
    
    /* Use constructors */
    struct ComplexStruct local_cs = {.a = 999, .b = 888.0, .c = 777.0f, .d = 'L'};
    union DataUnion local_du = {.f = ssa_float};
    
    /* Use initialized arrays */
    result += init_array[0] + md_array[0][0];
    result += cs1.a + cs2.a + cs3.a;
    result += os1.extra + os2.extra;
    result += struct_array[0].a + struct_array[1].a + struct_array[2].a;
    
    /* C++ specific code */
    #ifdef __cplusplus
    Derived1 d1;
    Derived2 d2;
    Diamond d3;
    TemplateDerived<int> td;
    
    Base1* b1 = &d1;
    Base2* b2 = &d2;
    VirtualBase* vb = &d3;
    
    /* Use virtual functions */
    b1->base1_func();
    b2->base2_func();
    vb->virt_func();
    
    /* Casts that might use BINFO */
    Derived1* pd1 = static_cast<Derived1*>(b1);
    Derived2* pd2 = dynamic_cast<Derived2*>(b2);
    #endif
    
    /* Conditionally use OpenMP */
    #ifdef _OPENMP
    if (argc > 1) {
        openmp_test(1000);
    }
    #endif
    
    printf("Final result: %d\n", result + ssa_result);
    return 0;
}

/* Function definitions */
int complex_func(int a, long b, char c, double d, short e,
                 float f, unsigned g, signed char h, int* i, void* j) {
    return a + (int)b + (int)c + (int)d + (int)e + 
           (int)f + (int)g + (int)h + (*i ? *i : 0);
}

/* Additional functions to ensure everything is used */
void ensure_usage(void) {
    /* Use vector types */
    v4si vec1 = {1, 2, 3, 4};
    v8sf vec2 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    /* Use function pointer array */
    func_ptr_array[0] = complex_func;
    
    /* Use namespace in C++ */
    #ifdef __cplusplus
    my_namespace::ns_var = 100;
    my_namespace::ns_func();
    #endif
}
