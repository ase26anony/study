/* tree_coverage.c - Comprehensive coverage of GCC tree node types */

/* For OpenMP support */
#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE coverage ==================== */

/* Multiple distinct identifiers in global scope */
int unique_var_1, unique_var_2, unique_var_3;
float special_float_var;
char *string_pointer;

/* Type identifiers */
typedef int my_type_1;
typedef long my_type_2;
typedef double my_type_3;
typedef char* string_type;

/* Function identifiers */
void func_1(void);
int func_2(int);
double func_3(double, double);

/* Label identifiers in functions */
void label_heavy_func(void) {
    label_1: ;
    label_2: ;
    label_3: ;
    goto label_1;
}

/* ==================== TREE_VEC coverage ==================== */

/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h) {
    return a + b + c;
}

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4];
float matrix[10][10];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Function with array parameters (creates TREE_VEC for parameter list) */
void process_matrix(int rows, int cols, int matrix[rows][cols]) {
    /* Process matrix */
}

/* ==================== CONSTRUCTOR coverage ==================== */

/* Struct with initializer */
struct ComplexStruct {
    int id;
    double value;
    char name[32];
    float *pointer;
};

/* Array with designated initializer */
int designated_array[10] = {1, 2, [5] = 50, [9] = 100};

/* Nested struct initializer */
struct Inner {
    int x, y;
};

struct Outer {
    struct Inner inner;
    double data;
};

/* Union initializer */
union DataUnion {
    int i;
    float f;
    char str[20];
};

/* Complex constructor with nested designators */
struct NestedInit {
    struct {
        int a;
        int b;
    } inner;
    int outer;
};

/* ==================== BLOCK coverage ==================== */

/* Function with multiple nested blocks */
void block_heavy_function(int param) {
    /* Outer block */
    int outer_var = param;
    
    {
        /* First inner block */
        int inner_var_1 = outer_var * 2;
        {
            /* Nested block */
            int deeply_nested = inner_var_1 + 10;
        }
    }
    
    {
        /* Second inner block */
        float float_var = 3.14f;
        {
            /* Another nested block */
            double double_var = 2.71828;
        }
    }
    
    /* Loop with block */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * i;
        if (loop_var > 5) {
            int conditional_var = loop_var / 2;
        }
    }
    
    /* Label address taking */
    void *label_ptr;
    my_label: 
    label_ptr = &&my_label;
    
    /* Switch with blocks */
    switch (param) {
        case 1: {
            int case_var = 100;
            break;
        }
        case 2: {
            int case_var = 200;
            break;
        }
        default: {
            int default_var = 300;
        }
    }
}

/* ==================== SSA_NAME coverage ==================== */

/* Function with complex control flow for SSA generation */
int ssa_generator(int input) {
    volatile int x = input;  /* Prevent optimization */
    int y = 0;
    int z = 0;
    
    /* Complex conditional */
    if (x > 0) {
        y = x * 2;
        if (y > 10) {
            z = y - 5;
        } else {
            z = y + 5;
        }
    } else {
        y = x / 2;
        z = y * 3;
    }
    
    /* Loop with phi node potential */
    int sum = 0;
    for (int i = 0; i < y; i++) {
        sum += i;
        if (sum > 100) {
            sum = sum / 2;  /* Creates additional SSA names */
        }
    }
    
    /* Another loop with break */
    int prod = 1;
    for (int j = 0; j < z; j++) {
        prod *= j;
        if (prod > 1000) {
            break;
        }
    }
    
    return sum + prod + z;
}

/* ==================== C++ specific (TREE_BINFO) coverage ==================== */
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
struct DerivedSingle : public Base1 {
    int derived_data;
    void base1_func() override {}
};

/* Multiple inheritance */
struct DerivedMultiple : public Base1, public Base2 {
    double derived_multiple_data;
    void base1_func() override {}
    void base2_func() override {}
};

/* Virtual inheritance */
struct VirtualBase {
    int virtual_data;
    virtual void virtual_func() {}
};

struct DerivedVirtual : virtual public VirtualBase {
    void virtual_func() override {}
};

/* Deep inheritance hierarchy */
struct Level1 { int a; virtual void f1() {} };
struct Level2 : public Level1 { int b; virtual void f2() {} };
struct Level3 : public Level2 { int c; virtual void f3() {} };

/* Template class with inheritance */
template<typename T>
struct TemplateBase {
    T data;
    virtual void process() {}
};

template<typename T>
struct TemplateDerived : public TemplateBase<T> {
    void process() override {}
};

#endif /* __cplusplus */

/* ==================== OpenMP (OMP_CLAUSE) coverage ==================== */

/* Function with various OpenMP pragmas */
void openmp_coverage(int size, float *data) {
    int i;
    float sum = 0.0f;
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(data, sum, size) default(none)
    {
        #pragma omp for reduction(+:sum) schedule(static)
        for (i = 0; i < size; i++) {
            sum += data[i];
        }
        
        /* Sections */
        #pragma omp sections
        {
            #pragma omp section
            {
                /* Section 1 work */
            }
            #pragma omp section
            {
                /* Section 2 work */
            }
        }
    }
    
    /* SIMD loop */
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (i = 0; i < size; i++) {
        sum += data[i] * 2.0f;
    }
    
    /* Task with depend clause */
    float task_result = 0.0f;
    #pragma omp task depend(inout: task_result)
    {
        task_result = sum / size;
    }
    
    #pragma omp taskwait
    
    /* Parallel for with collapse */
    #pragma omp parallel for collapse(2) private(i)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            /* Nested loop work */
        }
    }
    
    /* Critical section */
    #pragma omp critical
    {
        sum += 1.0f;
    }
}

/* ==================== Main driver ==================== */

int main(int argc, char **argv) {
    /* Use command line arg to conditionally enable OpenMP */
    int use_openmp = (argc > 1);
    
    /* Initialize constructors */
    struct ComplexStruct cs = { 
        .id = 1, 
        .value = 3.14159, 
        .name = "test",
        .pointer = NULL 
    };
    
    union DataUnion du = { .f = 2.71828f };
    
    struct NestedInit ni = {
        .inner = { .a = 10, .b = 20 },
        .outer = 30
    };
    
    /* Use multi-dimensional array */
    multi_dim_array[0][0][0] = 42;
    
    /* Call function with complex prototype */
    int result = complex_func(1, 2L, 'a', 3.14, 4, 5.0f, 6U, 7);
    
    /* Generate SSA names */
    int ssa_result = ssa_generator(result);
    
    /* Use block-heavy function */
    block_heavy_function(ssa_result);
    
    /* Use label function */
    label_heavy_func();
    
    /* Conditional OpenMP execution */
    if (use_openmp) {
        float *data = (float*)malloc(100 * sizeof(float));
        for (int i = 0; i < 100; i++) {
            data[i] = (float)i;
        }
        openmp_coverage(100, data);
        free(data);
    }
    
    #ifdef __cplusplus
    /* C++ specific code for TREE_BINFO */
    DerivedSingle ds;
    DerivedMultiple dm;
    DerivedVirtual dv;
    
    Base1 *bp1 = &ds;
    Base1 *bp2 = &dm;
    VirtualBase *vbp = &dv;
    
    /* Use dynamic_cast to trigger BINFO lookups */
    DerivedSingle *dsp = dynamic_cast<DerivedSingle*>(bp1);
    #endif
    
    return 0;
}

/* Additional functions to ensure tree building */

/* Function with TREE_VEC in return type (complex type) */
int (*function_returning_pointer(int x))(int, int) {
    static int (*func_ptr)(int, int) = NULL;
    return func_ptr;
}

/* Variable with complex type */
int (*(*complex_var)(int))(int, int) = function_returning_pointer;

/* Anonymous struct */
struct {
    int anonymous_field;
} anonymous_instance;
