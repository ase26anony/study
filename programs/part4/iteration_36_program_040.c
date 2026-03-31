/* tree_coverage_test.c - Comprehensive test to cover tree_kind switch cases */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ========== IDENTIFIER_NODE generation ========== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3, unique_var_4;
static int static_unique_1, static_unique_2;
extern int extern_unique_1;

typedef int my_type_1, my_type_2, my_type_3;
typedef long my_long_type_1, my_long_type_2;
typedef struct my_struct_type my_struct_type;
typedef enum { ENUM_VAL_1, ENUM_VAL_2, ENUM_VAL_3 } my_enum_type;

/* Function with label for identifier generation */
void func_with_labels(void) {
    label_1: ;
    label_2: ;
    label_3: ;
    goto label_2;
}

/* ========== TREE_VEC generation ========== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h);
void another_func(int p1, int p2, int p3, int p4, int p5,
                  int p6, int p7, int p8, int p9, int p10);

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4];
int another_array[5][6][7][2];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* ========== BLOCK generation ========== */
/* Functions with nested blocks */
void block_generator(void) {
    /* Outer block variable */
    int outer_var = 0;
    
    /* First nested block */
    {
        int block_var_1 = 1;
        {
            int inner_block_var = 2;
            outer_var += inner_block_var;
        }
    }
    
    /* Second nested block */
    {
        int block_var_2 = 3;
        {
            int another_inner = 4;
            block_var_2 += another_inner;
        }
    }
    
    /* Loop with block */
    for (int i = 0; i < 10; i++) {
        int loop_block_var = i * 2;
        outer_var += loop_block_var;
    }
    
    /* Conditional with block */
    if (outer_var > 0) {
        int cond_block_var = 100;
        outer_var += cond_block_var;
    } else {
        int else_block_var = 200;
        outer_var += else_block_var;
    }
}

/* Function with label address taken */
void label_address_func(void) {
    void* label_ptr;
    
    /* Take address of label */
    label_ptr = &&my_label;
    
    /* Use the label pointer to potentially jump */
    if (label_ptr) {
        /* Do something */
    }
    
my_label:
    return;
}

/* ========== CONSTRUCTOR generation ========== */
/* Struct with initializers */
struct ComplexStruct {
    int a;
    double b;
    char c;
    float d;
    short e;
};

/* Union with initializers */
union MyUnion {
    int i;
    float f;
    double d;
    char c[8];
};

/* Array with complex initializer */
int initialized_array[10] = {1, 2, 3, [7] = 8, [9] = 10};

/* Struct initialization */
struct ComplexStruct cs1 = {1, 2.0, 'a', 3.0f, 4};
struct ComplexStruct cs2 = {.a = 5, .b = 6.0, .c = 'b', .d = 7.0f, .e = 8};

/* Union initialization */
union MyUnion u1 = {.i = 42};
union MyUnion u2 = {.f = 3.14f};
union MyUnion u3 = {.d = 2.71828};

/* Nested struct initialization */
struct OuterStruct {
    struct ComplexStruct inner;
    int extra;
};

struct OuterStruct os = {{10, 20.0, 'c', 30.0f, 40}, 50};

/* ========== SSA_NAME generation ========== */
/* Function with complex control flow for SSA */
int ssa_generator(int x) {
    volatile int y = x;  /* volatile prevents optimization */
    int z = 0;
    
    /* Conditional with multiple assignments */
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    /* Loop with SSA opportunities */
    for (volatile int i = 0; i < y; ++i) {
        z += i * 2;
        if (z > 100) {
            z = z / 2;
        }
    }
    
    /* Another conditional */
    switch (z % 3) {
        case 0:
            y = z * 2;
            break;
        case 1:
            y = z + 10;
            break;
        default:
            y = z - 5;
            break;
    }
    
    return y + z;
}

/* Another SSA-intensive function */
int complex_ssa(int a, int b) {
    int x = a;
    int y = b;
    int z = 0;
    
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            x = x + y;
        } else {
            y = y - x;
        }
        z = z + (x * y);
        
        /* Nested loop for more SSA */
        for (int j = 0; j < 10; j++) {
            z += j;
        }
    }
    
    return z;
}

/* ========== OpenMP clauses generation ========== */
#ifdef _OPENMP
void omp_test_function(int size) {
    int i;
    int sum = 0;
    int private_var = 0;
    static int shared_var = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Test various OpenMP clauses */
    
    /* 1. parallel for with private and shared */
    #pragma omp parallel for private(i) private(private_var) shared(arr, shared_var) schedule(static)
    for (i = 0; i < 100; i++) {
        private_var = arr[i];
        #pragma omp atomic
        shared_var += private_var;
    }
    
    /* 2. parallel with reduction */
    #pragma omp parallel reduction(+:sum)
    {
        int local_sum = 0;
        #pragma omp for
        for (i = 0; i < 100; i++) {
            local_sum += arr[i];
        }
        sum += local_sum;
    }
    
    /* 3. sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 50; i++) {
                arr[i] *= 2;
            }
        }
        
        #pragma omp section
        {
            for (i = 50; i < 100; i++) {
                arr[i] /= 2;
            }
        }
    }
    
    /* 4. simd with reduction */
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* 5. task with depend */
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
    
    /* 6. parallel with collapse */
    int matrix[10][10];
    #pragma omp parallel for collapse(2)
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            matrix[row][col] = row * col;
        }
    }
}
#endif

/* ========== C++ specific code for TREE_BINFO ========== */
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

/* Multiple inheritance with virtual base */
struct Derived4 : public Base1, public virtual VirtualBase {
    int derived4_data;
    void base1_func() override {}
    void virtual_func() override {}
};

/* Deep inheritance hierarchy */
struct DeepBase { int a; virtual void f() {} };
struct DeepMid1 : virtual DeepBase { int b; void f() override {} };
struct DeepMid2 : virtual DeepBase { int c; void f() override {} };
struct DeepDerived : DeepMid1, DeepMid2 { 
    int d; 
    void f() override {} 
};

/* Template class with inheritance */
template<typename T>
struct TemplateBase {
    T data;
    virtual void process() {}
};

template<typename T>
struct TemplateDerived : TemplateBase<T> {
    T extra;
    void process() override {}
};

void test_cpp_inheritance() {
    /* Create objects of different derived types */
    Derived1 d1;
    Derived2 d2;
    Derived3 d3;
    Derived4 d4;
    
    /* Use base class pointers (triggers binfo lookups) */
    Base1* b1_ptr = &d1;
    Base2* b2_ptr = &d2;
    VirtualBase* vb_ptr = &d3;
    
    /* Dynamic casts (use binfo) */
    Derived1* casted1 = dynamic_cast<Derived1*>(b1_ptr);
    Base1* casted2 = dynamic_cast<Base1*>(b2_ptr);
    
    /* Call virtual functions */
    b1_ptr->base1_func();
    b2_ptr->base2_func();
    vb_ptr->virtual_func();
    
    /* Test template inheritance */
    TemplateDerived<int> td;
    TemplateBase<int>* tb_ptr = &td;
    tb_ptr->process();
    
    /* Deep hierarchy test */
    DeepDerived dd;
    DeepBase* db_ptr = &dd;
    db_ptr->f();
}

#endif /* __cplusplus */

/* ========== Main driver function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use identifiers */
    unique_var_1 = 1;
    unique_var_2 = 2;
    func_with_labels();
    
    /* Use complex functions */
    result += complex_func(1, 2L, 'a', 3.0, 4, 5.0f, 6, 'b');
    
    /* Use multi-dimensional arrays */
    multi_dim_array[0][0][0] = 42;
    another_array[1][2][3][0] = 24;
    
    /* Generate blocks */
    block_generator();
    label_address_func();
    
    /* Use constructors */
    result += cs1.a + cs2.e;
    result += u1.i;
    result += os.extra;
    result += initialized_array[7];
    
    /* Generate SSA */
    result += ssa_generator(argc);
    result += complex_ssa(argc, result);
    
    /* Use vector types */
    v4si vec1 = {1, 2, 3, 4};
    v8sf vec2 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    result += vec1[0];
    
#ifdef _OPENMP
    /* Test OpenMP if enabled */
    omp_test_function(100);
#endif
    
#ifdef __cplusplus
    /* Test C++ inheritance if in C++ mode */
    test_cpp_inheritance();
#endif
    
    return result;
}

/* Implementation of declared functions */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h) {
    return a + (int)b + (int)c + (int)d + e + (int)f + (int)g + (int)h;
}
