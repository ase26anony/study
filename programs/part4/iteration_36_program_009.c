/* tree_coverage_test.c - Comprehensive test for GCC tree node coverage */

/* Enable OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ========== IDENTIFIER_NODE generation ========== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3;
static int static_var_1, static_var_2;
extern int extern_var_1;

/* Type identifiers */
typedef int my_type_1;
typedef long my_type_2;
typedef double my_type_3;
typedef my_type_1 my_nested_type;

/* Function identifiers */
void func_1(void);
int func_2(int);
double func_3(double, double);

/* Label identifiers */
void label_test(void) {
    label_1: ;
    label_2: ;
    goto label_1;
}

/* ========== TREE_VEC generation ========== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h);

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4];
int (*func_ptr_array[5])(int, int);

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* ========== CONSTRUCTOR nodes ========== */
/* Struct initializers */
struct S { 
    int a; 
    double b;
    char c[10];
};

struct T {
    struct S s;
    int x;
};

/* Array initializers */
int arr_init[5] = {1, 2, 3, [4] = 5};
int sparse_arr[10] = {[2] = 20, [5] = 50, [9] = 90};

/* Struct with designated initializers */
struct S s1 = { .a = 1, .b = 2.0, .c = "hello" };
struct S s2 = { 1, 3.14, "world" };

/* Nested initializers */
struct T t1 = { .s = { .a = 10, .b = 20.5, .c = "nested" }, .x = 100 };

/* Union initializer */
union U { 
    int i; 
    float f; 
    double d;
} u1 = { .f = 3.14f };

/* ========== BLOCK nodes ========== */
/* Nested blocks */
void block_test(void) {
    /* Outer block */
    int outer_var = 10;
    
    {
        /* Inner block 1 */
        int inner_var_1 = 20;
        {
            /* Deeply nested block */
            int deep_var = inner_var_1 + outer_var;
        }
    }
    
    {
        /* Inner block 2 */
        int inner_var_2 = 30;
        volatile int vol_var = 40; /* Prevent optimization */
    }
    
    /* Label address taking */
    void* label_ptr;
    my_label_1: ;
    label_ptr = &&my_label_1;
    
    if (label_ptr) {
        goto my_label_2;
    }
    
    my_label_2: ;
}

/* ========== SSA_NAME generation ========== */
/* Function with complex control flow for SSA */
int ssa_test_function(int x) {
    int y = x;
    volatile int z = 0; /* Prevent optimization */
    
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    /* Loop for more SSA complexity */
    int sum = 0;
    for (int i = 0; i < y; ++i) {
        sum += i;
        if (sum > 100) {
            sum = sum / 2;
        }
    }
    
    /* Nested loops */
    for (int j = 0; j < 10; ++j) {
        for (int k = 0; k < j; ++k) {
            sum += j * k;
        }
    }
    
    return sum + z;
}

/* ========== OpenMP clauses ========== */
void omp_test(void) {
    int i;
    int n = 100;
    int arr[100];
    int sum = 0;
    int product = 1;
    
    /* Initialize array */
    for (i = 0; i < n; ++i) {
        arr[i] = i + 1;
    }
    
    /* Test various OpenMP pragmas */
    #pragma omp parallel for private(i) shared(arr, n) schedule(static)
    for (i = 0; i < n; ++i) {
        arr[i] = arr[i] * 2;
    }
    
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < n; ++i) {
        sum += arr[i];
    }
    
    #pragma omp parallel sections
    {
        #pragma omp section
        {
            product = 1;
            for (i = 0; i < n/2; ++i) {
                product *= arr[i];
            }
        }
        
        #pragma omp section
        {
            int local_sum = 0;
            for (i = n/2; i < n; ++i) {
                local_sum += arr[i];
            }
            #pragma omp atomic
            sum += local_sum;
        }
    }
    
    #pragma omp simd reduction(+:sum)
    for (i = 0; i < n; ++i) {
        sum += arr[i] % 10;
    }
    
    #pragma omp task depend(inout: sum)
    {
        sum = sum * 2;
    }
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task
            {
                sum += 100;
            }
        }
    }
}

/* ========== C++ specific (TREE_BINFO) ========== */
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
    virtual void base1_func() override {}
    virtual ~Derived1() {}
};

/* Multiple inheritance */
struct Derived2 : public Base1, public Base2 {
    double derived2_data;
    virtual void base1_func() override {}
    virtual void base2_func() override {}
    virtual ~Derived2() {}
};

/* Virtual inheritance */
struct VirtualBase {
    int virtual_data;
    virtual void virtual_func() {}
    virtual ~VirtualBase() {}
};

struct Derived3 : virtual public VirtualBase {
    int derived3_data;
    virtual void virtual_func() override {}
    virtual ~Derived3() {}
};

struct Derived4 : virtual public VirtualBase {
    int derived4_data;
    virtual void virtual_func() override {}
    virtual ~Derived4() {}
};

/* Diamond inheritance */
struct Diamond : public Derived3, public Derived4 {
    int diamond_data;
    virtual void virtual_func() override {}
    virtual ~Diamond() {}
};

/* Template classes */
template<typename T>
class TemplateBase {
public:
    T data;
    virtual void process() = 0;
    virtual ~TemplateBase() {}
};

template<typename T, typename U>
class TemplateDerived : public TemplateBase<T> {
public:
    U extra_data;
    virtual void process() override {}
    virtual ~TemplateDerived() {}
};

#endif /* __cplusplus */

/* ========== Main driver ========== */
int main(int argc, char** argv) {
    /* Use identifiers */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Use complex function */
    int result = complex_func(1, 2L, 'a', 3.14, 4, 5.0f, 6, 'b');
    
    /* Use multi-dimensional array */
    multi_dim_array[0][1][2] = 42;
    
    /* Use vector types */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Use constructors */
    struct S local_s = { .a = 100, .b = 200.5, .c = "local" };
    int local_arr[] = {10, 20, 30, 40};
    
    /* Generate blocks */
    block_test();
    
    /* Generate SSA names */
    int ssa_result = ssa_test_function(argc);
    
    /* Use OpenMP if enabled */
    #ifdef _OPENMP
    omp_test();
    #endif
    
    /* C++ specific tests */
    #ifdef __cplusplus
    {
        /* Create objects with inheritance */
        Derived1 d1;
        Derived2 d2;
        Diamond d3;
        
        /* Use polymorphism */
        Base1* b1 = &d1;
        Base2* b2 = &d2;
        VirtualBase* vb = &d3;
        
        /* Call virtual functions */
        b1->base1_func();
        b2->base2_func();
        vb->virtual_func();
        
        /* Dynamic casts */
        Derived1* pd1 = dynamic_cast<Derived1*>(b1);
        Derived2* pd2 = dynamic_cast<Derived2*>(b2);
        
        /* Template instantiation */
        TemplateDerived<int, double> td;
        td.process();
    }
    #endif
    
    return ssa_result + result + multi_dim_array[0][1][2];
}

/* Implementation of complex function */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h) {
    return a + (int)b + (int)c + (int)d + e + (int)f + (int)g + (int)h;
}

/* Function implementations */
void func_1(void) {
    /* Empty but referenced */
}

int func_2(int x) {
    return x * 2;
}

double func_3(double x, double y) {
    return x + y;
}
