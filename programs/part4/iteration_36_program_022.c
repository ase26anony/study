/* tree_coverage.c - Comprehensive test for GCC tree node coverage */

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
char* string_identifier;

typedef int my_type_1, my_type_2, my_type_3;
typedef double precision_type;
typedef struct { int x; } struct_type_id;

enum color_id { RED_ID, GREEN_ID, BLUE_ID };

void func_1(void) { 
    func_label_1: ;
    int local_func_var;
}

void func_2(int param_id) {
    static int static_func_var;
    register int reg_var;
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

/* Function with variable arguments */
int varargs_func(int count, ...);

/* ==================== BLOCK generation ==================== */
void block_generator(void) {
    /* Level 1 block */
    {
        int block_var_1;
        double block_double;
        
        /* Level 2 nested block */
        {
            char nested_char;
            static int static_in_block;
            
            /* Level 3 deeply nested */
            {
                volatile int volatile_inner = 42;
                goto *block_generator_label_ptr;
            }
        }
    }
    
    /* Another sibling block */
    {
        float block_float;
        
        /* Label address taking for BLOCK/LABEL_DECL */
        void* my_label_ptr = &&my_special_label;
        block_generator_label_ptr = my_label_ptr;
        
        if (block_float > 0) {
            goto my_special_label;
        }
        
        my_special_label:
        printf("Reached label\n");
    }
    
    /* Loop blocks */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * 2;
        if (loop_var > 5) {
            int if_block_var = loop_var + 1;
        }
    }
    
    /* Switch statement blocks */
    switch (block_var_1) {
        case 1: {
            int case_1_var = 100;
            break;
        }
        case 2: {
            double case_2_var = 3.14;
            break;
        }
        default: {
            char default_char = 'X';
        }
    }
}

/* Global pointer for label addresses */
void* block_generator_label_ptr;

/* ==================== CONSTRUCTOR generation ==================== */
struct ComplexStruct {
    int a;
    double b;
    char c;
    float d[3];
    struct {
        int nested_x;
        char nested_y;
    } inner;
};

union MixedUnion {
    int i;
    float f;
    double d;
    char str[8];
};

/* Various initializers */
struct ComplexStruct cs1 = { 1, 2.0, 'A', {1.0f, 2.0f, 3.0f}, {100, 'Z'} };
struct ComplexStruct cs2 = { .a = 42, .b = 3.14159, .c = 'B', 
                             .d = {4.0f, 5.0f, 6.0f}, .inner = {200, 'Y'} };

int array_init[10] = {0, 1, 2, [7] = 7, [9] = 9};
int sparse_array[100] = {[10] = 100, [50] = 500, [99] = 999};

union MixedUnion mu1 = { .f = 3.14f };
union MixedUnion mu2 = { .i = 255 };
union MixedUnion mu3 = { .d = 2.71828 };

struct Nested {
    struct {
        int a;
        int b;
    } s;
    int c;
} nested_init = { {1, 2}, 3 };

/* Zero initialization */
struct ComplexStruct zero_struct = {0};

/* ==================== SSA_NAME generation ==================== */
int ssa_generator(int x) {
    volatile int y = x;  /* volatile prevents optimization */
    int z = 0;
    
    /* Complex control flow for SSA */
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    /* Loop creates phi nodes */
    for (volatile int i = 0; i < y; ++i) {
        if (i % 2 == 0) {
            z += i;
        } else {
            z -= i;
        }
        
        /* Nested loop */
        for (int j = 0; j < 5; j++) {
            z += j;
        }
    }
    
    /* Another conditional */
    int w = (z > 100) ? z : 100;
    
    /* Switch with multiple cases */
    switch (w) {
        case 100:
            w += 10;
            break;
        case 200:
            w += 20;
            break;
        default:
            w += 5;
    }
    
    return w + z + y;
}

/* ==================== C++ Specific (TREE_BINFO) ==================== */
#ifdef __cplusplus

/* Base classes */
struct Base1 {
    virtual void vfunc1() {}
    int base1_data;
};

struct Base2 {
    virtual void vfunc2() {}
    double base2_data;
};

/* Virtual base */
struct VirtualBase {
    virtual void vbase_func() {}
    int vbase_data;
};

/* Single inheritance */
struct Derived1 : public Base1 {
    void vfunc1() override {}
    int derived1_data;
};

/* Multiple inheritance */
struct Derived2 : public Base1, public Base2 {
    void vfunc1() override {}
    void vfunc2() override {}
    int derived2_data;
};

/* Virtual inheritance */
struct Derived3 : public virtual VirtualBase {
    void vbase_func() override {}
    int derived3_data;
};

/* Diamond inheritance */
struct MostDerived : public Derived1, public Derived2 {
    void vfunc1() override {}
    void vfunc2() override {}
    int most_derived_data;
};

/* Templates also generate interesting trees */
template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T d) : data(d) {}
    T get() const { return data; }
};

#endif /* __cplusplus */

/* ==================== OpenMP (OMP_CLAUSE) ==================== */
void openmp_test(int size) {
    int i;
    double sum = 0.0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        array[i] = i + 1;
    }
    
    /* Various OpenMP pragmas to generate different clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(array, sum, size) default(none)
    {
        int local_sum = 0;
        
        /* Parallel for with schedule clause */
        #pragma omp for schedule(static, 4) nowait
        for (i = 0; i < size; i++) {
            local_sum += array[i];
        }
        
        #pragma omp atomic
        sum += local_sum;
    }
    
    printf("OpenMP sum: %f\n", sum);
    
    /* SIMD loop */
    sum = 0.0;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (i = 0; i < size; i++) {
        sum += array[i] * 0.5;
    }
    
    printf("SIMD sum: %f\n", sum);
    
    /* Sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < size/2; i++) {
                array[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; i++) {
                array[i] *= 3;
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
        { y = x + 1; }
        
        #pragma omp task depend(in: y)
        { printf("Task result: %d\n", y); }
    }
    
    /* Collapsed nested loops */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            array[i * 10 + j] = i + j;
        }
    }
    
    free(array);
}

/* ==================== Main driver ==================== */
int main(int argc, char** argv) {
    printf("Tree coverage test program\n");
    
    /* Use identifiers */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Use TREE_VEC related constructs */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Call function with complex signature */
    int result = complex_func(1, 2L, 'c', 4.0, 5, 6.0f, 7U, 'd');
    
    /* Access multi-dimensional array */
    multi_dim_array[0][1][2] = 42;
    
    /* Generate blocks */
    block_generator();
    
    /* Use constructors/initializers */
    struct ComplexStruct local_cs = { 
        .a = 10, 
        .b = 20.0, 
        .c = 'X',
        .d = {1.1f, 2.2f, 3.3f},
        .inner = { .nested_x = 30, .nested_y = 'Y' }
    };
    
    /* Generate SSA names */
    int ssa_result = ssa_generator(100);
    printf("SSA result: %d\n", ssa_result);
    
    /* C++ specific tests */
    #ifdef __cplusplus
    Derived1 d1;
    Derived2 d2;
    Derived3 d3;
    MostDerived md;
    
    Base1* b1_ptr = &d1;
    Base2* b2_ptr = &d2;
    
    TemplateClass<int> tc(42);
    printf("Template value: %d\n", tc.get());
    
    /* Dynamic cast for RTTI */
    Derived1* casted = dynamic_cast<Derived1*>(b1_ptr);
    #endif
    
    /* Conditionally run OpenMP tests */
    int use_openmp = 0;
    if (argc > 1) {
        use_openmp = atoi(argv[1]);
    }
    
    if (use_openmp) {
        #ifdef _OPENMP
        printf("Running with OpenMP\n");
        openmp_test(1000);
        #else
        printf("OpenMP not supported\n");
        #endif
    } else {
        printf("OpenMP disabled\n");
    }
    
    /* Use initialized arrays */
    for (int i = 0; i < 10; i++) {
        printf("array_init[%d] = %d\n", i, array_init[i]);
    }
    
    return 0;
}

/* Implementation of declared functions */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h) {
    return a + (int)b + (int)c + (int)d + e + (int)f + (int)g + (int)h;
}

/* Additional function to ensure various tree nodes are created */
void extra_coverage(void) {
    /* Take address of labels */
    void* label_array[] = { &&label_a, &&label_b, &&label_c };
    
    label_a:
    printf("Label A\n");
    goto *label_array[1];
    
    label_b:
    printf("Label B\n");
    goto *label_array[2];
    
    label_c:
    printf("Label C\n");
    
    /* Array with designators in middle */
    int complex_array[20] = { [0] = 1, [10] = 2, [19] = 3, [5] = 4 };
    
    /* Nested struct initializer */
    struct {
        struct {
            int deep_a;
            int deep_b;
        } level1;
        int top;
    } deep_struct = { { 1, 2 }, 3 };
}
