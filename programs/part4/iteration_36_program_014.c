/* tree_coverage.c - Comprehensive test to cover tree_kind switch cases */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif
#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3;
float special_float_var;
char char_identifier;

typedef int my_type_1, my_type_2, my_type_3;
typedef double double_alias_1, double_alias_2;

struct struct_id_1 { int x; };
union union_id_1 { int i; float f; };
enum enum_id_1 { ENUM_VAL_1, ENUM_VAL_2 };

/* Function with label identifiers */
void func_with_labels(void) {
    label_1: ;
    label_2: ;
    label_3: ;
    goto label_2;
}

/* ==================== TREE_VEC generation ==================== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h);

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4];
float matrix[10][10][5];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Function with variable arguments (creates complex type trees) */
int varargs_func(int count, ...);

/* ==================== CONSTRUCTOR nodes ==================== */
/* Struct initializers */
struct Point {
    int x;
    int y;
    double z;
};

struct Rectangle {
    struct Point top_left;
    struct Point bottom_right;
};

/* Array initializers */
int init_array[5] = {1, 2, 3, 4, 5};
int sparse_array[10] = {[2] = 20, [5] = 50, [9] = 90};

/* Nested initializers */
struct Rectangle rect = {
    .top_left = {.x = 0, .y = 10, .z = 1.5},
    .bottom_right = {.x = 20, .y = 0, .z = 1.5}
};

/* Union initializer */
union Data {
    int i;
    float f;
    char str[20];
};

union Data data = {.f = 3.14f};

/* ==================== BLOCK nodes ==================== */
/* Function with nested blocks */
void function_with_blocks(int param) {
    /* Outer block */
    int outer_var = param;
    
    {
        /* Inner block 1 */
        int inner_var_1 = outer_var * 2;
        {
            /* Deeply nested block */
            int deep_var = inner_var_1 + 10;
            (void)deep_var;
        }
    }
    
    {
        /* Inner block 2 */
        volatile int inner_var_2 = 42;
        (void)inner_var_2;
    }
    
    /* Label address taking */
    void* label_ptr;
    my_label: 
    label_ptr = &&my_label;
    
    if (param > 0) {
        /* Conditional block */
        int cond_var = param * 3;
        (void)cond_var;
    }
    
    /* Loop with block */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * i;
        (void)loop_var;
    }
}

/* ==================== SSA_NAME generation ==================== */
/* Function with complex control flow for SSA */
int ssa_generator(int x) {
    volatile int y = x;  /* volatile prevents optimization */
    int z;
    
    if (y > 0) {
        y = y * 2;
        z = y + 5;
    } else {
        y = y - 3;
        z = y * 2;
    }
    
    for (volatile int i = 0; i < y; ++i) {
        z = z + i;
        if (z > 100) {
            z = z / 2;
        }
    }
    
    switch (z) {
        case 1: y = 10; break;
        case 2: y = 20; break;
        default: y = z * 3;
    }
    
    return y + z;
}

/* ==================== OpenMP clauses ==================== */
/* Function with various OpenMP pragmas */
void openmp_test(int size) {
    int i;
    int sum = 0;
    int* arr = (int*)malloc(size * sizeof(int));
    
    if (!arr) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        arr[i] = i + 1;
    }
    
    /* Various OpenMP constructs with different clauses */
    #pragma omp parallel for private(i) shared(arr, size) reduction(+:sum) schedule(static)
    for (i = 0; i < size; i++) {
        sum += arr[i];
    }
    
    printf("Parallel sum: %d\n", sum);
    
    /* SIMD with reduction */
    sum = 0;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < size; i++) {
        sum += arr[i] * 2;
    }
    
    printf("SIMD sum: %d\n", sum);
    
    /* Sections with private clause */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < size/2; i++) {
                arr[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; i++) {
                arr[i] *= 3;
            }
        }
    }
    
    /* Task with depend clause */
    int x = 0, y = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x + 2; }
        
        #pragma omp task depend(in: y)
        { printf("Task result: y = %d\n", y); }
    }
    
    /* Parallel with collapse */
    int matrix_sum = 0;
    #pragma omp parallel for collapse(2) reduction(+:matrix_sum)
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            matrix_sum += row * col;
        }
    }
    
    free(arr);
}

/* ==================== C++ specific (TREE_BINFO) ==================== */
#ifdef __cplusplus

/* Base classes for inheritance */
struct Base1 {
    int base1_data;
    virtual void base1_func() { base1_data = 1; }
    virtual ~Base1() {}
};

struct Base2 {
    float base2_data;
    virtual void base2_func() { base2_data = 2.0f; }
    virtual ~Base2() {}
};

/* Virtual base for virtual inheritance */
struct VirtualBase {
    int virtual_data;
    VirtualBase() : virtual_data(100) {}
    virtual void virtual_func() {}
    virtual ~VirtualBase() {}
};

/* Single inheritance */
struct Derived1 : public Base1 {
    int derived1_data;
    void base1_func() override { base1_data = 10; }
};

/* Multiple inheritance */
struct Derived2 : public Base1, public Base2 {
    int derived2_data;
    void base1_func() override { base1_data = 20; }
    void base2_func() override { base2_data = 30.0f; }
};

/* Virtual inheritance */
struct Derived3 : public virtual VirtualBase {
    int derived3_data;
    void virtual_func() override { virtual_data = 200; }
};

/* Diamond inheritance with virtual base */
struct Left : virtual VirtualBase {
    int left_data;
};

struct Right : virtual VirtualBase {
    int right_data;
};

struct Bottom : public Left, public Right {
    int bottom_data;
};

/* Templates that create complex type trees */
template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T d) : data(d) {}
    T get_data() { return data; }
};

/* Function template */
template<typename T, typename U>
auto template_func(T a, U b) -> decltype(a + b) {
    return a + b;
}

#endif /* __cplusplus */

/* ==================== Main driver ==================== */
int main(int argc, char** argv) {
    /* Use command line arg to conditionally enable OpenMP */
    int use_openmp = (argc > 1);
    
    /* Reference identifiers to ensure they're used */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Use multi-dimensional array */
    multi_dim_array[0][0][0] = 42;
    
    /* Use struct with initializer */
    struct Point p = {.x = 10, .y = 20, .z = 3.14};
    (void)p;
    
    /* Call function with blocks */
    function_with_blocks(5);
    
    /* Generate SSA names */
    int ssa_result = ssa_generator(10);
    printf("SSA function result: %d\n", ssa_result);
    
    /* Use vector types */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    (void)vec1;
    (void)vec2;
    
    /* Conditional OpenMP execution */
    if (use_openmp) {
        printf("Running with OpenMP\n");
        openmp_test(100);
    } else {
        printf("Running without OpenMP\n");
        /* Still call but without OpenMP pragmas active */
        void openmp_test_simple(int);
        openmp_test_simple(50);
    }
    
    /* C++ specific code */
    #ifdef __cplusplus
    printf("\nC++ Inheritance Tests:\n");
    
    /* Create objects of different inheritance hierarchies */
    Derived1 d1;
    Derived2 d2;
    Derived3 d3;
    Bottom bottom;
    
    /* Use polymorphism */
    Base1* b1 = &d1;
    Base2* b2 = &d2;
    
    b1->base1_func();
    b2->base2_func();
    
    /* Use templates */
    TemplateClass<int> tc_int(42);
    TemplateClass<double> tc_double(3.14);
    
    printf("Template int: %d\n", tc_int.get_data());
    printf("Template double: %f\n", tc_double.get_data());
    
    /* Template function */
    auto result = template_func(10, 20.5);
    printf("Template function result: %f\n", result);
    
    /* Casts that involve binfo */
    Derived1* derived_ptr = static_cast<Derived1*>(b1);
    (void)derived_ptr;
    
    /* Dynamic cast (requires RTTI) */
    Base1* base_ptr = &d2;
    Derived2* derived2_ptr = dynamic_cast<Derived2*>(base_ptr);
    if (derived2_ptr) {
        printf("Dynamic cast successful\n");
    }
    #endif
    
    return 0;
}

/* Simple version without OpenMP pragmas for when OpenMP is disabled */
void openmp_test_simple(int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += i;
    }
    printf("Simple sum: %d\n", sum);
}

/* Implement the complex function */
int complex_func(int a, long b, char c, double d, short e,
                 float f, unsigned int g, signed char h) {
    return a + (int)b + (int)c + (int)d + e + (int)f + (int)g + (int)h;
}

/* Variable arguments function */
#include <stdarg.h>
int varargs_func(int count, ...) {
    va_list args;
    va_start(args, count);
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += va_arg(args, int);
    }
    va_end(args);
    return sum;
}
