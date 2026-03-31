/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3, unique_var_4, unique_var_5;
float another_var_1, another_var_2, another_var_3;
char char_var_1, char_var_2;

/* Type identifiers */
typedef int my_type_1;
typedef float my_type_2;
typedef char my_type_3;
typedef long my_type_4;
typedef double my_type_5;

/* Function identifiers */
void func_1(void);
int func_2(int);
float func_3(float, float);
double func_4(double, int, char);

/* Label identifiers (for goto) */
void label_test(void) {
label_1:
    int x = 1;
label_2:
    x++;
label_3:
    if (x < 10) goto label_1;
}

/* ==================== TREE_VEC generation ==================== */
/* Multi-dimensional arrays */
int multi_dim_array[2][3][4][5];
float matrix[10][20];
char cube[5][5][5];

/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e,
                 float f, unsigned g, signed char h, int* i, float* j);

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Function with variable arguments (also creates TREE_VEC) */
int varargs_func(int count, ...);

/* ==================== BLOCK generation ==================== */
/* Nested blocks and lexical scopes */
void block_test(void) {
    /* Outer block variable */
    int outer = 0;
    
    {
        /* Inner block 1 */
        int inner1 = 1;
        {
            /* Inner block 2 */
            int inner2 = 2;
            outer = inner1 + inner2;
        }
    }
    
    /* Another block with different scope */
    {
        float block_float = 3.14f;
        {
            char block_char = 'A';
            outer += (int)block_float + block_char;
        }
    }
    
    /* Label address taking creates blocks */
    void* label_ptr;
    my_label:
        label_ptr = &&my_label;
    
    /* Loop blocks */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * 2;
        outer += loop_var;
    }
    
    /* Switch statement blocks */
    switch (outer) {
        case 1: {
            int case_var = 100;
            break;
        }
        case 2: {
            int case_var = 200;
            break;
        }
        default: {
            int case_var = 300;
            break;
        }
    }
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
    float value;
    char name[20];
};

/* Union with initializers */
union Number {
    int i;
    float f;
    double d;
};

/* Array initializers */
void constructor_test(void) {
    /* Struct initializers */
    struct Point p1 = {1, 2, 3};
    struct Point p2 = {.x = 4, .y = 5, .z = 6};
    struct Point p3 = {.y = 8, .x = 7, .z = 9};
    
    /* Array initializers */
    int arr1[5] = {1, 2, 3, 4, 5};
    int arr2[10] = {[0] = 10, [5] = 50, [9] = 90};
    int arr3[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
    
    /* Union initializers */
    union Number n1 = {.i = 42};
    union Number n2 = {.f = 3.14f};
    union Number n3 = {.d = 2.71828};
    
    /* Nested initializers */
    struct Data data1 = {1, 3.14f, "test"};
    struct Data data2 = {.id = 2, .value = 2.718f, .name = "example"};
    
    /* Partial initializers */
    int partial[100] = {[10] = 100, [20] = 200, [99] = 999};
}

/* ==================== SSA_NAME generation ==================== */
/* Functions with complex control flow for SSA */
int ssa_test_1(int x) {
    int y = x;
    volatile int vol = 10; /* Prevent optimization */
    
    if (y > 0) {
        y = y * 2;
        vol = y;
    } else {
        y = y - 3;
        vol = y;
    }
    
    for (int i = 0; i < y; ++i) {
        vol += i;
        if (vol > 100) {
            y += vol;
            break;
        }
    }
    
    while (y < 1000) {
        y *= 2;
        vol--;
        if (vol < 0) {
            goto early_exit;
        }
    }
    
    return y;
    
early_exit:
    return -1;
}

float ssa_test_2(float a, float b) {
    float result = a + b;
    volatile float v = 0.0f;
    
    for (int i = 0; i < 100; i++) {
        if (i % 2 == 0) {
            result += v;
            v += 0.5f;
        } else {
            result -= v;
            v -= 0.25f;
        }
        
        switch (i % 3) {
            case 0: result *= 1.1f; break;
            case 1: result /= 1.1f; break;
            case 2: result = -result; break;
        }
    }
    
    do {
        result += 1.0f;
        v -= 0.1f;
    } while (v > -10.0f);
    
    return result;
}

/* ==================== OpenMP Clause generation ==================== */
void omp_test(void) {
    int i;
    int sum = 0;
    int arr[1000];
    int private_var = 0;
    float reduction_sum = 0.0f;
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        arr[i] = i + 1;
    }
    
    /* Various OpenMP pragmas with different clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(arr, sum) private(private_var)
    {
        #pragma omp for schedule(static, 10) nowait
        for (i = 0; i < 1000; i++) {
            arr[i] *= 2;
        }
        
        #pragma omp barrier
        
        #pragma omp for reduction(+:sum)
        for (i = 0; i < 1000; i++) {
            sum += arr[i];
        }
    }
    
    /* SIMD with reduction */
    #pragma omp simd reduction(+:reduction_sum) simdlen(8)
    for (i = 0; i < 1000; i++) {
        reduction_sum += arr[i] * 0.1f;
    }
    
    /* Sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 500; i++) {
                arr[i] += 1;
            }
        }
        
        #pragma omp section
        {
            for (i = 500; i < 1000; i++) {
                arr[i] -= 1;
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
        { sum += y; }
    }
    
    /* Parallel for with collapse */
    int matrix2d[100][100];
    #pragma omp parallel for collapse(2) schedule(dynamic)
    for (int row = 0; row < 100; row++) {
        for (int col = 0; col < 100; col++) {
            matrix2d[row][col] = row * col;
        }
    }
}

/* ==================== C++ specific for TREE_BINFO ==================== */
#ifdef __cplusplus

/* Base classes for inheritance */
class Base1 {
public:
    int base1_data;
    virtual void base1_func() { base1_data = 1; }
    virtual ~Base1() {}
};

class Base2 {
public:
    float base2_data;
    virtual void base2_func() { base2_data = 2.0f; }
    virtual ~Base2() {}
};

/* Simple inheritance */
class Derived1 : public Base1 {
public:
    int derived1_data;
    void base1_func() override { base1_data = 10; }
};

/* Multiple inheritance */
class Derived2 : public Base1, public Base2 {
public:
    char derived2_data;
    void base1_func() override { base1_data = 20; }
    void base2_func() override { base2_data = 20.0f; }
};

/* Virtual inheritance */
class VirtualBase {
public:
    int virtual_data;
    virtual void virtual_func() { virtual_data = 100; }
};

class Derived3 : virtual public VirtualBase {
public:
    void virtual_func() override { virtual_data = 200; }
};

class Derived4 : virtual public VirtualBase {
public:
    void virtual_func() override { virtual_data = 300; }
};

/* Diamond inheritance with virtual base */
class Diamond : public Derived3, public Derived4 {
public:
    void virtual_func() override { virtual_data = 400; }
};

/* Template class for additional complexity */
template<typename T>
class TemplateBase {
public:
    T template_data;
    virtual T get_data() { return template_data; }
};

class Derived5 : public TemplateBase<int> {
public:
    int get_data() override { return template_data * 2; }
};

void cpp_test(void) {
    /* Create objects of different derived classes */
    Derived1 d1;
    Derived2 d2;
    Derived3 d3;
    Derived4 d4;
    Diamond diamond;
    Derived5 d5;
    
    /* Use base class pointers (triggers binfo lookups) */
    Base1* b1_ptr = &d1;
    Base1* b2_ptr = &d2;
    Base2* b3_ptr = &d2;
    VirtualBase* v_ptr = &diamond;
    
    /* Call virtual functions */
    b1_ptr->base1_func();
    b2_ptr->base1_func();
    b3_ptr->base2_func();
    v_ptr->virtual_func();
    
    /* Dynamic casts (use binfo) */
    Derived2* casted = dynamic_cast<Derived2*>(b2_ptr);
    if (casted) {
        casted->derived2_data = 'X';
    }
    
    /* Typeid expressions */
    const std::type_info& ti1 = typeid(*b1_ptr);
    const std::type_info& ti2 = typeid(*b2_ptr);
}

#endif /* __cplusplus */

/* ==================== Main driver ==================== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Use identifiers */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Use complex function prototypes */
    my_type_1 mt1 = 100;
    my_type_2 mt2 = 200.0f;
    
    /* Test blocks */
    block_test();
    
    /* Test constructors */
    constructor_test();
    
    /* Test SSA functions */
    result += ssa_test_1(42);
    result += (int)ssa_test_2(10.0f, 20.0f);
    
    /* Test OpenMP if enabled */
    #ifdef _OPENMP
    omp_test();
    #endif
    
    /* Test C++ features if in C++ mode */
    #ifdef __cplusplus
    cpp_test();
    #endif
    
    /* Use multi-dimensional arrays */
    multi_dim_array[0][0][0][0] = result;
    matrix[5][10] = result * 1.5f;
    
    /* Use vector types */
    v4si vec1 = {1, 2, 3, 4};
    v8sf vec2 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    return result > 0 ? 0 : 1;
}

/* Additional complex function definitions */
int complex_func(int a, long b, char c, double d, short e,
                 float f, unsigned g, signed char h, int* i, float* j) {
    return a + (int)b + c + (int)d + e + (int)f + g + h + *i + (int)*j;
}

int varargs_func(int count, ...) {
    /* Simple implementation */
    return count * 2;
}

void func_1(void) {
    label_1: ;
}

int func_2(int x) {
    return x * 2;
}

float func_3(float a, float b) {
    return a + b;
}

double func_4(double a, int b, char c) {
    return a + b + c;
}
