/* tree_coverage.c - Comprehensive test to cover GCC tree node kinds */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3, unique_var_4, unique_var_5;
float special_float_var;
double precision_double_var;
char char_identifier;
long long_identifier;

/* Type identifiers */
typedef int my_type_1;
typedef float my_type_2;
typedef double my_type_3;
typedef char my_type_4;
typedef long my_type_5;

/* Function identifiers */
void func_1(void);
int func_2(int);
float func_3(float, float);
double func_4(double, int, char);

/* Label identifiers */
void label_test(void) {
    label_1: ;
    label_2: ;
    label_3: ;
    label_4: ;
    goto label_2;
}

/* ==================== TREE_VEC generation ==================== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned g, signed char h, int i, long long j);

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4];
float another_array[5][6][7][2];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* ==================== CONSTRUCTOR nodes ==================== */
/* Struct initializers */
struct ComplexStruct {
    int a;
    double b;
    char c[10];
    float d[3][3];
};

struct NestedStruct {
    struct ComplexStruct inner;
    int x;
    double y;
};

/* Array initializers */
int initialized_array[5] = {1, 2, 3, [4] = 5};
int sparse_array[10] = {[2] = 20, [5] = 50, [9] = 90};

/* Struct with designated initializer */
struct ComplexStruct cs1 = { .a = 1, .b = 2.0, .c = "test", .d = {{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}, {7.0, 8.0, 9.0}} };

/* Union initializer */
union DataUnion {
    int i;
    float f;
    char str[20];
};

union DataUnion du1 = { .f = 3.14f };
union DataUnion du2 = { .i = 42 };
union DataUnion du3 = { .str = "initializer" };

/* Nested initializer */
struct NestedStruct ns1 = {
    .inner = { .a = 10, .b = 20.5, .c = "nested", .d = {{1.1, 2.2, 3.3}} },
    .x = 100,
    .y = 200.5
};

/* ==================== BLOCK nodes ==================== */
/* Function with nested blocks */
void block_intensive_function(int param) {
    /* Outer block */
    int outer_var = param;
    
    {
        /* Inner block 1 */
        int inner_var_1 = outer_var * 2;
        {
            /* Deeper block */
            int deeper_var = inner_var_1 + 10;
            (void)deeper_var;
        }
    }
    
    {
        /* Inner block 2 */
        float float_var = 3.14f;
        {
            /* Another deep block */
            double double_var = float_var * 2.0;
            (void)double_var;
        }
    }
    
    /* Loop with block */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * i;
        if (loop_var > 5) {
            int if_block_var = loop_var + 1;
            (void)if_block_var;
        }
    }
    
    /* Switch with blocks */
    switch (param) {
        case 1: {
            int case1_var = 100;
            (void)case1_var;
            break;
        }
        case 2: {
            int case2_var = 200;
            (void)case2_var;
            break;
        }
        default: {
            int default_var = 300;
            (void)default_var;
        }
    }
}

/* Label address taking for BLOCK nodes */
void label_address_function(void) {
    void* label_ptr;
    
    /* Take address of labels */
    label_ptr = &&label_a;
    goto *label_ptr;
    
label_a:
    {
        int block_in_label = 1;
        (void)block_in_label;
    }
    
    label_ptr = &&label_b;
    if (unique_var_1 > 0) {
        goto *label_ptr;
    }
    
label_b:
    {
        float another_block = 2.0f;
        (void)another_block;
    }
}

/* ==================== SSA_NAME generation ==================== */
/* Function with complex control flow for SSA */
int ssa_intensive_function(int x) {
    volatile int y = x;  /* volatile to prevent optimization */
    int z = 0;
    
    /* Conditional with multiple assignments */
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    /* Loop with SSA */
    for (int i = 0; i < y; ++i) {
        z = z + i;
        if (z > 100) {
            y = y - 1;  /* Modify y in loop */
        }
    }
    
    /* Another conditional */
    int result = (z > 50) ? z : y;
    
    /* Switch to create more SSA */
    switch (result) {
        case 0 ... 10:
            result = result * 2;
            break;
        case 11 ... 20:
            result = result / 2;
            break;
        default:
            result = result + 100;
    }
    
    return result;
}

/* Another SSA-intensive function */
float complex_ssa_function(float a, float b) {
    volatile float x = a;
    volatile float y = b;
    float accum = 0.0f;
    
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            x = x + y;
            accum = accum + x;
        } else {
            y = y - x;
            accum = accum - y;
        }
        
        /* Nested loop for more complexity */
        for (int j = 0; j < 5; j++) {
            float temp = x * j;
            accum = accum + temp;
        }
    }
    
    return accum;
}

/* ==================== C++ specific for TREE_BINFO ==================== */
#ifdef __cplusplus

/* Base classes for inheritance */
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

/* Virtual base for virtual inheritance */
struct VirtualBase {
    int virtual_data;
    virtual void virtual_func() {}
    virtual ~VirtualBase() {}
};

/* Single inheritance */
struct Derived1 : public Base1 {
    int derived1_data;
    void base1_func() override {}
};

/* Multiple inheritance */
struct Derived2 : public Base1, public Base2 {
    int derived2_data;
    void base1_func() override {}
    void base2_func() override {}
};

/* Virtual inheritance */
struct Derived3 : public virtual VirtualBase {
    int derived3_data;
    void virtual_func() override {}
};

/* Diamond inheritance with virtual base */
struct Derived4 : public Derived1, public Derived3 {
    int derived4_data;
    void base1_func() override {}
    void virtual_func() override {}
};

/* Complex inheritance hierarchy */
struct DeepBase { int x; virtual void deep_func() {} };
struct Mid1 : virtual DeepBase { int y; };
struct Mid2 : virtual DeepBase { int z; };
struct DeepDerived : Mid1, Mid2 {
    int w;
    void deep_func() override {}
};

/* Template class for additional complexity */
template<typename T>
class TemplateBase {
public:
    T data;
    virtual void template_func() {}
    virtual ~TemplateBase() {}
};

class ConcreteDerived : public TemplateBase<int> {
public:
    int extra;
    void template_func() override {}
};

#endif /* __cplusplus */

/* ==================== OpenMP for OMP_CLAUSE ==================== */
void openmp_test(int size) {
    int i;
    double sum = 0.0;
    int array[1000];
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        array[i] = i;
    }
    
    /* Test various OpenMP pragmas */
    
    /* 1. Parallel for with private and shared clauses */
    #pragma omp parallel for private(i) shared(array, sum) schedule(static)
    for (i = 0; i < 1000; i++) {
        #pragma omp atomic
        sum += array[i];
    }
    
    /* 2. Parallel region with reduction */
    #pragma omp parallel reduction(+:sum)
    {
        int thread_id = omp_get_thread_num();
        sum += thread_id;
    }
    
    /* 3. Sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 500; i++) {
                array[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 500; i < 1000; i++) {
                array[i] /= 2;
            }
        }
    }
    
    /* 4. SIMD with reduction */
    sum = 0.0;
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < 1000; i++) {
        sum += array[i];
    }
    
    /* 5. Task with depend clause */
    int x = 0, y = 0;
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp task depend(out: x)
        { x = 1; }
        
        #pragma omp task depend(in: x) depend(out: y)
        { y = x + 1; }
        
        #pragma omp task depend(in: y)
        { sum += y; }
    }
    
    /* 6. Collapse clause for nested loops */
    #pragma omp parallel for collapse(2) private(i)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            int idx = i * 10 + j;
            if (idx < 1000) {
                array[idx] = i + j;
            }
        }
    }
    
    /* 7. Ordered clause */
    #pragma omp parallel for ordered schedule(dynamic)
    for (i = 0; i < 100; i++) {
        #pragma omp ordered
        {
            sum += i;
        }
    }
    
    (void)sum; /* Prevent unused variable warning */
}

/* ==================== Main driver function ==================== */
int main(int argc, char *argv[]) {
    /* Use command line arg to conditionally enable OpenMP */
    int use_openmp = (argc > 1);
    
    /* Exercise IDENTIFIER_NODEs */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Exercise TREE_VEC nodes */
    int val = multi_dim_array[0][0][0];
    v4si vec1 = {1, 2, 3, 4};
    
    /* Exercise CONSTRUCTOR nodes */
    struct ComplexStruct local_cs = { .a = 5, .b = 10.5 };
    int local_array[3] = {1, 2, 3};
    
    /* Exercise BLOCK nodes */
    block_intensive_function(10);
    label_address_function();
    
    /* Exercise SSA_NAME nodes */
    int ssa_result = ssa_intensive_function(42);
    float ssa_float_result = complex_ssa_function(1.0f, 2.0f);
    
    /* Exercise C++ specific nodes if in C++ mode */
    #ifdef __cplusplus
    Derived1 d1;
    Derived2 d2;
    Derived3 d3;
    Derived4 d4;
    DeepDerived dd;
    ConcreteDerived cd;
    
    Base1* ptr1 = &d1;
    Base2* ptr2 = &d2;
    VirtualBase* ptr3 = &d3;
    
    ptr1->base1_func();
    ptr2->base2_func();
    ptr3->virtual_func();
    
    /* Dynamic cast for inheritance checks */
    Derived1* derived_ptr = dynamic_cast<Derived1*>(ptr1);
    (void)derived_ptr;
    #endif
    
    /* Exercise OpenMP if enabled */
    if (use_openmp) {
        openmp_test(1000);
    }
    
    /* Use all variables to prevent optimization */
    return unique_var_1 + unique_var_2 + val + ssa_result + 
           (int)ssa_float_result + local_cs.a + local_array[0];
}
