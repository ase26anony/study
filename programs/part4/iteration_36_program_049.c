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
static int static_unique_var_4;
volatile int volatile_unique_var_5;

typedef int my_type_1;
typedef long my_type_2;
typedef double my_type_3;
typedef my_type_1 my_nested_type_4;

enum my_enum_1 { ENUM_VAL_1, ENUM_VAL_2, ENUM_VAL_3 };
enum my_enum_2 { ANOTHER_ENUM_1, ANOTHER_ENUM_2 };

struct struct_with_labels {
    int field_1;
    double field_2;
    char field_3;
};

/* ==================== TREE_VEC generation ==================== */
/* Complex function prototypes with many parameters */
int complex_func_1(int a, long b, char c, double d, short e, float f);
void complex_func_2(int p1, int p2, int p3, int p4, int p5, 
                    int p6, int p7, int p8, int p9, int p10);

/* Multi-dimensional arrays */
int multi_dim_array_1[2][3][4];
double multi_dim_array_2[5][6][7][2];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Complex type combinations that may generate TREE_VEC */
int (*complex_func_ptr)(int, long, char, double, short, float);
int (*(*nested_func_ptr[2])[3])(int, int);

/* ==================== SSA_NAME and BLOCK generation ==================== */
/* Function with complex control flow for SSA generation */
int ssa_generating_function(int x) {
    int y = x;
    volatile int vol_var = 0; /* Prevent optimization */
    
    /* Nested blocks for BLOCK nodes */
    {
        int block_var_1 = y * 2;
        {
            int block_var_2 = block_var_1 + 1;
            y += block_var_2;
        }
    }
    
    /* Complex control flow */
    if (y > 0) {
        y = y * 2;
        for (int i = 0; i < y; ++i) {
            int loop_var = i * 3;
            y += loop_var % 5;
        }
    } else {
        int else_var = -y;
        while (else_var > 0) {
            y += else_var;
            else_var /= 2;
        }
    }
    
    /* Switch statement for more control flow */
    switch (y % 4) {
        case 0: y += 10; break;
        case 1: y += 20; break;
        case 2: y += 30; break;
        default: y += 40;
    }
    
    /* goto with label address for BLOCK nodes */
    void* label_ptr = &&my_label;
    if (vol_var > 100) {
        goto *label_ptr;
    }
    
    return y;
    
my_label:
    return y * -1;
}

/* Another function with nested blocks */
void block_generating_function(void) {
    /* Multiple nested blocks */
    {
        int block_a = 1;
        {
            int block_b = block_a + 1;
            {
                int block_c = block_b * 2;
                (void)block_c;
            }
        }
    }
    
    /* Loop with internal block */
    for (int i = 0; i < 10; i++) {
        int loop_block_var = i * i;
        (void)loop_block_var;
    }
    
    /* Label with address taken */
    void* ptr = &&another_label;
    (void)ptr;
    
another_label:
    return;
}

/* ==================== CONSTRUCTOR generation ==================== */
/* Struct with initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Data {
    int id;
    double value;
    char name[20];
    struct Point location;
};

/* Union with initializers */
union Mixed {
    int int_val;
    float float_val;
    double double_val;
    char char_val;
};

/* Array with complex initializer */
int initialized_array[10] = {1, 2, 3, [7] = 8, [9] = 10};

/* Struct with designated initializer */
struct Data global_data = {
    .id = 1001,
    .value = 3.14159,
    .name = "test",
    .location = { .x = 10, .y = 20, .z = 30 }
};

/* Union initializer */
union Mixed global_mixed = { .float_val = 2.718f };

/* Nested initializers */
struct Nested {
    struct Point points[3];
    int counts[5];
};

struct Nested nested_init = {
    .points = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} },
    .counts = { [0] = 10, [2] = 20, [4] = 30 }
};

/* ==================== C++ specific: TREE_BINFO generation ==================== */
#ifdef __cplusplus

/* Base classes for inheritance */
class Base1 {
public:
    int base1_data;
    virtual void base1_func() {}
    virtual ~Base1() {}
};

class Base2 {
public:
    double base2_data;
    virtual void base2_func() {}
    virtual ~Base2() {}
};

/* Virtual base class */
class VirtualBase {
public:
    int virtual_data;
    virtual void virtual_func() {}
    virtual ~VirtualBase() {}
};

/* Single inheritance */
class Derived1 : public Base1 {
public:
    int derived1_data;
    void base1_func() override {}
};

/* Multiple inheritance */
class Derived2 : public Base1, public Base2 {
public:
    char derived2_data;
    void base1_func() override {}
    void base2_func() override {}
};

/* Virtual inheritance */
class Derived3 : public virtual VirtualBase {
public:
    float derived3_data;
    void virtual_func() override {}
};

/* Diamond inheritance with virtual base */
class MostDerived : public Derived2, public Derived3 {
public:
    int most_derived_data;
    void base1_func() override {}
    void base2_func() override {}
    void virtual_func() override {}
};

/* Template class that might generate complex trees */
template<typename T>
class TemplateClass : public Base1 {
public:
    T template_data;
    void process(T value) { template_data = value; }
};

#endif /* __cplusplus */

/* ==================== OpenMP: OMP_CLAUSE generation ==================== */
void openmp_test_function(int size) {
    int i;
    double sum = 0.0;
    int* data = (int*)malloc(size * sizeof(int));
    
    if (!data) return;
    
    /* Initialize data */
    for (i = 0; i < size; i++) {
        data[i] = i + 1;
    }
    
    /* Various OpenMP pragmas with different clauses */
    #pragma omp parallel for private(i) shared(data, size) reduction(+:sum) schedule(static)
    for (i = 0; i < size; i++) {
        sum += data[i];
    }
    
    printf("Sum with parallel for: %f\n", sum);
    
    /* Another OpenMP construct */
    sum = 0.0;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:sum) nowait
        for (i = 0; i < size; i++) {
            sum += data[i] * 2;
        }
        
        #pragma omp barrier
        
        #pragma omp single
        {
            printf("Thread completed work\n");
        }
    }
    
    printf("Sum with parallel region: %f\n", sum);
    
    /* SIMD pragma */
    sum = 0.0;
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (i = 0; i < size; i++) {
        sum += data[i] / 2.0;
    }
    
    printf("Sum with SIMD: %f\n", sum);
    
    /* Sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < size/2; i++) {
                data[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; i++) {
                data[i] /= 2;
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
    
    free(data);
}

/* ==================== Main driver function ==================== */
int main(int argc, char** argv) {
    int result;
    
    /* Use identifiers */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Use TREE_VEC related constructs */
    multi_dim_array_1[0][0][0] = 42;
    v4si vec1 = {1, 2, 3, 4};
    (void)vec1;
    
    /* Generate SSA and BLOCK nodes */
    result = ssa_generating_function(10);
    block_generating_function();
    
    /* Use CONSTRUCTOR initialized data */
    struct Data local_data = {
        .id = 2002,
        .value = 1.414,
        .name = "local",
        .location = { .x = result, .y = result * 2, .z = result * 3 }
    };
    
    printf("Local data id: %d, location x: %d\n", 
           local_data.id, local_data.location.x);
    
    /* Use initialized arrays */
    for (int i = 0; i < 10; i++) {
        printf("Array[%d] = %d\n", i, initialized_array[i]);
    }
    
#ifdef __cplusplus
    /* C++ specific: Use inheritance hierarchies */
    Derived1 d1;
    Derived2 d2;
    MostDerived md;
    
    Base1* b1_ptr = &d1;
    Base2* b2_ptr = &d2;
    VirtualBase* vb_ptr = &md;
    
    b1_ptr->base1_func();
    b2_ptr->base2_func();
    vb_ptr->virtual_func();
    
    /* Use template class */
    TemplateClass<int> tc;
    tc.process(42);
#endif
    
    /* Conditionally run OpenMP tests */
    if (argc > 1) {
        int size = 1000;
        if (argc > 2) {
            size = atoi(argv[2]);
        }
        openmp_test_function(size);
    }
    
    return 0;
}

/* Additional complex function definitions */
int complex_func_1(int a, long b, char c, double d, short e, float f) {
    return a + (int)b + (int)c + (int)d + e + (int)f;
}

void complex_func_2(int p1, int p2, int p3, int p4, int p5, 
                    int p6, int p7, int p8, int p9, int p10) {
    int sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;
    printf("Sum of 10 params: %d\n", sum);
}
