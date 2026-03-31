/* tree_coverage_test.c - Comprehensive test for GCC tree node coverage */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3, unique_var_4;
static int static_unique_1, static_unique_2;
extern int extern_unique_1;

/* Type identifiers */
typedef int my_type_1;
typedef long my_type_2;
typedef double my_type_3;
typedef my_type_1 my_nested_type_1;

/* Function identifiers */
void func_1(void);
int func_2(int);
double func_3(double, double);

/* Label identifiers */
void label_generator(void) {
    label_1: ;
    label_2: ;
    label_3: ;
    goto label_1;
}

/* ==================== TREE_VEC generation ==================== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h, 
                 void* i, const char* j);

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4];
int (*func_ptr_array[5])(int, int);
int (*(*complex_ptr)[10])(void);

/* Vector types using GCC extension */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* ==================== TREE_BINFO generation (C++ only) ==================== */
#ifdef __cplusplus

/* Base classes */
struct Base1 {
    int base1_data;
    virtual void base1_func() {}
};

struct Base2 {
    double base2_data;
    virtual void base2_func() {}
};

/* Single inheritance */
struct Derived1 : public Base1 {
    int derived1_data;
    virtual void base1_func() override {}
};

/* Multiple inheritance */
struct Derived2 : public Base1, public Base2 {
    char derived2_data;
    virtual void base1_func() override {}
    virtual void base2_func() override {}
};

/* Virtual inheritance */
struct VirtualBase {
    int virtual_data;
};

struct Derived3 : virtual public VirtualBase {
    int derived3_data;
};

struct Derived4 : virtual public VirtualBase {
    int derived4_data;
};

struct Diamond : public Derived3, public Derived4 {
    void diamond_func() {
        virtual_data = 42;  // Uses virtual base
    }
};

/* Template with inheritance */
template<typename T>
struct TemplateBase {
    T data;
};

template<typename T>
struct TemplateDerived : TemplateBase<T> {
    T more_data;
};

#endif /* __cplusplus */

/* ==================== SSA_NAME generation ==================== */
/* Functions with complex control flow to generate SSA */
int ssa_generator_1(int x) {
    volatile int y = x;  /* volatile prevents optimization */
    int z = 0;
    
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    for (int i = 0; i < y; ++i) {
        z += i * i;
        if (z > 100) {
            z -= 10;
        }
    }
    
    while (z > 0) {
        z /= 2;
        y++;
    }
    
    do {
        z = z % 7;
        y = y - z;
    } while (y > 0);
    
    return z;
}

double ssa_generator_2(double a, double b) {
    volatile double result = 0.0;
    double temp;
    
    for (int i = 0; i < 100; i++) {
        temp = a * i + b;
        if (temp > result) {
            result = temp;
        } else {
            result = result * 0.99;
        }
        
        switch (i % 4) {
            case 0: result += 1.0; break;
            case 1: result -= 0.5; break;
            case 2: result *= 1.1; break;
            case 3: result /= 1.05; break;
        }
    }
    
    return result;
}

/* ==================== BLOCK generation ==================== */
/* Nested blocks and label addresses */
void block_generator(void) {
    /* Outer block */
    int outer_var = 42;
    
    {
        /* Inner block 1 */
        int inner_var_1 = outer_var * 2;
        static int static_block_var = 100;
        
        {
            /* Deeply nested block */
            int deep_var = inner_var_1 + static_block_var;
            volatile int volatile_deep = deep_var;
        }
    }
    
    {
        /* Inner block 2 */
        char char_var = 'A';
        double double_var = 3.14159;
        
        if (char_var == 'A') {
            /* Conditional block */
            double_var *= 2.0;
        }
    }
    
    /* Label address taking */
    void* label_ptr;
    
    my_label_1:
        label_ptr = &&my_label_1;
    
    my_label_2:
        if (outer_var > 0) {
            goto my_label_1;
        } else {
            goto my_label_2;
        }
    
    /* Loop blocks */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * i;
        if (loop_var % 2 == 0) {
            int even_var = loop_var + 1;
        } else {
            int odd_var = loop_var - 1;
        }
    }
}

/* ==================== CONSTRUCTOR generation ==================== */
/* Struct with initializers */
struct ComplexStruct {
    int a;
    double b;
    char c;
    float d;
    short e;
    long f;
};

/* Union with initializers */
union MixedUnion {
    int as_int;
    float as_float;
    double as_double;
    char as_char[8];
};

/* Array with various initializers */
void constructor_generator(void) {
    /* Struct initializers */
    struct ComplexStruct s1 = { 1, 2.0, 'A', 3.0f, 4, 5L };
    struct ComplexStruct s2 = { .a = 10, .b = 20.5, .c = 'Z', .d = 30.5f, .e = 40, .f = 50L };
    struct ComplexStruct s3 = { .c = 'X', .a = 100, .b = 200.5 };  /* Designated out of order */
    
    /* Array initializers */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[10] = {1, 2, [5] = 10, [9] = 20};  /* Sparse with designators */
    int arr3[] = {[0 ... 4] = 1, [5 ... 9] = 2}; /* Range designators (GCC extension) */
    
    /* Multi-dimensional array initializers */
    int matrix[2][3] = {{1, 2, 3}, {4, 5, 6}};
    int cube[2][2][2] = {{{1, 2}, {3, 4}}, {{5, 6}, {7, 8}}};
    
    /* Union initializers */
    union MixedUnion u1 = { .as_int = 42 };
    union MixedUnion u2 = { .as_float = 3.14159f };
    union MixedUnion u3 = { .as_double = 2.71828 };
    
    /* Nested initializers */
    struct Nested {
        struct ComplexStruct inner;
        int extra;
    } nested = { { 1, 2.0, 'A', 3.0f, 4, 5L }, 100 };
    
    /* Zero initialization */
    struct ComplexStruct zero = {0};
    int zero_arr[10] = {0};
}

/* ==================== OMP_CLAUSE generation ==================== */
/* OpenMP constructs with various clauses */
void omp_generator(int* data, int size) {
    int i;
    int sum = 0;
    int max_val = 0;
    int min_val = 0;
    double product = 1.0;
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel default(none) shared(data, size, sum, max_val) \
                         private(i) reduction(+:sum) reduction(max:max_val)
    {
        #pragma omp for schedule(static, 4) nowait
        for (i = 0; i < size; i++) {
            sum += data[i];
            if (data[i] > max_val) {
                max_val = data[i];
            }
        }
        
        #pragma omp barrier
        
        #pragma omp single copyprivate(min_val)
        {
            min_val = data[0];
            for (i = 1; i < size; i++) {
                if (data[i] < min_val) {
                    min_val = data[i];
                }
            }
        }
    }
    
    /* SIMD loop */
    #pragma omp simd reduction(*:product) simdlen(8) aligned(data:32)
    for (i = 0; i < size; i++) {
        product *= (data[i] + 1.0);
    }
    
    /* Sections */
    #pragma omp parallel sections private(i) firstprivate(product)
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
    
    /* Task with dependencies */
    #pragma omp task depend(inout: data[0:size/2]) priority(10)
    {
        for (i = 0; i < size/2; i++) {
            data[i] += 1;
        }
    }
    
    #pragma omp task depend(inout: data[size/2:size-size/2])
    {
        for (i = size/2; i < size; i++) {
            data[i] -= 1;
        }
    }
    
    #pragma omp taskwait
    
    /* Collapsed nested loops */
    #pragma omp parallel for collapse(2) reduction(+:sum)
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            sum += i * j;
        }
    }
}

/* ==================== Main driver ==================== */
int main(int argc, char** argv) {
    /* Use command line arg to optionally enable OpenMP */
    int use_omp = (argc > 1);
    
    /* Exercise identifier nodes */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    my_type_1 type_var = 42;
    
    /* Exercise TREE_VEC nodes */
    int (*func_ptr)(int, long, char, double, short, float, unsigned int, signed char, void*, const char*);
    func_ptr = complex_func;
    
    int val = multi_dim_array[0][1][2];
    
    /* Exercise SSA nodes */
    int ssa_result = ssa_generator_1(10);
    double ssa_double = ssa_generator_2(1.5, 2.5);
    
    /* Exercise BLOCK nodes */
    block_generator();
    
    /* Exercise CONSTRUCTOR nodes */
    constructor_generator();
    
    /* Exercise OMP_CLAUSE nodes if enabled */
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    if (use_omp) {
        omp_generator(data, 100);
    }
    
    #ifdef __cplusplus
    /* Exercise TREE_BINFO nodes in C++ mode */
    Derived1 d1;
    Derived2 d2;
    Diamond d3;
    
    Base1* b1 = &d1;
    Base2* b2 = &d2;
    
    b1->base1_func();
    b2->base2_func();
    
    TemplateDerived<int> td;
    td.data = 42;
    td.more_data = 84;
    #endif
    
    return ssa_result + (int)ssa_double + data[0];
}

/* Dummy implementation of complex_func */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h, 
                 void* i, const char* j) {
    return a + (int)b + (int)c + (int)d + e + (int)f + (int)g + (int)h;
}
