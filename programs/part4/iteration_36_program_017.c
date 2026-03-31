/* tree_coverage_test.c - Comprehensive test to cover tree_kind switch cases */

/* For OpenMP support */
#include <stdio.h>
#include <stdlib.h>

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3;
float special_var_4;
char* string_var_5;

typedef int my_type_1;
typedef long my_type_2;
typedef double my_type_3;
typedef char my_type_4;

struct struct_id_1 {
    int member_a;
    float member_b;
};

enum enum_id_1 {
    ENUM_VAL_1,
    ENUM_VAL_2,
    ENUM_VAL_3
};

/* Function with label identifiers */
void func_with_labels(void) {
    label_1: ;
    label_2: ;
    label_3: ;
    goto label_2;
}

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

/* ==================== BLOCK nodes generation ==================== */
void block_generator(void) {
    /* Level 1 block */
    int block_var_1 = 10;
    
    {
        /* Level 2 nested block */
        int block_var_2 = 20;
        volatile int block_var_3 = 30;
        
        {
            /* Level 3 nested block */
            int block_var_4 = 40;
            static int block_var_5 = 50;
        }
    }
    
    /* Another block at level 1 */
    {
        double block_var_6 = 3.14;
        char block_var_7 = 'A';
    }
    
    /* Block with label address taken */
    void* label_ptr;
    my_label_1: 
    label_ptr = &&my_label_1;
    
    /* Conditional block */
    if (block_var_1 > 0) {
        int if_block_var = 100;
    } else {
        int else_block_var = 200;
    }
    
    /* Loop blocks */
    for (int i = 0; i < 10; i++) {
        int for_block_var = i * 2;
    }
    
    while (block_var_1-- > 0) {
        int while_block_var = 999;
    }
}

/* ==================== CONSTRUCTOR nodes generation ==================== */
struct Aggregate1 {
    int x;
    double y;
    char z[10];
};

union Union1 {
    int as_int;
    float as_float;
    double as_double;
};

/* Constructor initializers */
struct Aggregate1 agg1 = { .x = 42, .y = 3.14159, .z = "hello" };
struct Aggregate1 agg2 = { 1, 2.0, "test" };

int array_init_1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int array_init_2[20] = { [0] = 100, [10] = 200, [19] = 300 };
int array_init_3[5][3] = { {1, 2, 3}, {4, 5, 6}, {7, 8, 9} };

union Union1 u1 = { .as_float = 1.5f };
union Union1 u2 = { 42 };  /* First member (int) */

struct Nested {
    struct Aggregate1 inner;
    int extra;
} nested1 = { {10, 20.5, "nested"}, 30 };

/* ==================== SSA_NAME nodes generation ==================== */
int ssa_generator(int input) {
    volatile int x = input;  /* Prevent optimization */
    int y = 0;
    int z = 0;
    
    /* Complex control flow to generate SSA */
    if (x > 0) {
        y = x * 2;
        for (int i = 0; i < y; i++) {
            z += i;
            if (z > 100) {
                z = z / 2;
            }
        }
    } else {
        y = -x;
        while (y > 0) {
            z += y;
            y--;
        }
    }
    
    /* Switch statement for more SSA complexity */
    switch (z) {
        case 0: y = 1; break;
        case 1: y = 2; break;
        case 2: y = 3; break;
        default: y = z * 2; break;
    }
    
    return y + z;
}

/* ==================== OpenMP (OMP_CLAUSE) generation ==================== */
void omp_test(int size) {
    int i;
    int sum = 0;
    int arr[1000];
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        arr[i] = i;
    }
    
    /* Various OpenMP pragmas with different clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(arr, sum) num_threads(4)
    {
        int local_sum = 0;
        
        /* For loop with schedule clause */
        #pragma omp for schedule(static, 16) nowait
        for (i = 0; i < 1000; i++) {
            local_sum += arr[i];
        }
        
        #pragma omp atomic
        sum += local_sum;
    }
    
    /* SIMD loop with reduction */
    int simd_sum = 0;
    #pragma omp simd reduction(+:simd_sum) simdlen(8)
    for (i = 0; i < 1000; i++) {
        simd_sum += arr[i];
    }
    
    /* Sections */
    int section_result = 0;
    #pragma omp parallel sections private(i) shared(section_result)
    {
        #pragma omp section
        {
            for (i = 0; i < 250; i++) {
                section_result += arr[i];
            }
        }
        
        #pragma omp section
        {
            for (i = 250; i < 500; i++) {
                section_result += arr[i];
            }
        }
    }
    
    /* Task with depend clause */
    int task_x = 0, task_y = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: task_x)
        { task_x = 1; }
        
        #pragma omp task depend(in: task_x) depend(out: task_y)
        { task_y = task_x + 1; }
        
        #pragma omp task depend(in: task_y)
        { sum += task_y; }
    }
    
    printf("OpenMP sum: %d, SIMD sum: %d\n", sum, simd_sum);
}

/* ==================== C++ specific (TREE_BINFO) ==================== */
#ifdef __cplusplus

/* Base classes for inheritance */
class Base1 {
public:
    virtual void virt_func1() { }
    int base_data1;
};

class Base2 {
public:
    virtual void virt_func2() { }
    float base_data2;
};

/* Single inheritance */
class Derived1 : public Base1 {
public:
    void virt_func1() override { }
    int derived_data1;
};

/* Multiple inheritance */
class Derived2 : public Base1, public Base2 {
public:
    void virt_func1() override { }
    void virt_func2() override { }
    double derived_data2;
};

/* Virtual inheritance */
class VirtualBase {
public:
    int virtual_data;
};

class Derived3 : virtual public VirtualBase {
public:
    int extra_data;
};

class Derived4 : virtual public VirtualBase {
public:
    int more_data;
};

/* Diamond inheritance */
class Diamond : public Derived3, public Derived4 {
public:
    void diamond_func() { 
        virtual_data = 42;  /* Access virtual base member */
    }
};

void cpp_test() {
    Derived1 d1;
    Derived2 d2;
    Diamond d3;
    
    Base1* b1 = &d1;
    Base2* b2 = &d2;
    
    /* Dynamic casts for RTTI */
    Derived1* pd1 = dynamic_cast<Derived1*>(b1);
    if (pd1) {
        pd1->virt_func1();
    }
    
    /* Virtual function calls */
    b1->virt_func1();
    b2->virt_func2();
    
    /* Access through virtual base */
    d3.diamond_func();
}

#endif /* __cplusplus */

/* ==================== Main driver ==================== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use identifiers */
    unique_var_1 = 10;
    my_type_1 type_var = 20;
    
    /* Call function with labels */
    func_with_labels();
    
    /* Use blocks */
    block_generator();
    
    /* Use constructors */
    result += agg1.x;
    result += array_init_1[0];
    result += u1.as_int;
    
    /* Generate SSA */
    result += ssa_generator(argc);
    
    /* Use multi-dimensional arrays */
    multi_dim_array_1[0][0][0] = result;
    
    /* Use vector types if available */
    #ifdef __VECTOR_TYPES_SUPPORTED__
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    #endif
    
    /* Conditionally use OpenMP */
    #ifdef _OPENMP
    omp_test(1000);
    #endif
    
    /* C++ specific tests */
    #ifdef __cplusplus
    cpp_test();
    #endif
    
    /* Use complex function prototype */
    if (argc > 1) {
        result = complex_func_1(1, 2L, 'a', 3.0, 4, 5.0f);
    }
    
    return result;
}

/* Implementation of complex function */
int complex_func_1(int a, long b, char c, double d, short e, float f) {
    return a + (int)b + (int)c + (int)d + e + (int)f;
}

void complex_func_2(int p1, int p2, int p3, int p4, int p5, 
                    int p6, int p7, int p8, int p9, int p10) {
    volatile int sum = p1 + p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9 + p10;
}
