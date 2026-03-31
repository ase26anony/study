/* tree_coverage.c - Comprehensive test to cover GCC tree node kinds */

/* For OpenMP support */
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
void label_func(void) {
    label_1: ;
    label_2: ;
    goto label_1;
}

/* ========== TREE_VEC generation ========== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned int g, signed char h);
                
/* Multi-dimensional arrays */
int multi_array_1[2][3];
int multi_array_2[2][3][4];
int multi_array_3[2][3][4][5];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Function with complex return type */
int (*func_ptr_array[5])(int, int, int, int);

/* ========== SSA_NAME generation ========== */
/* Functions with complex control flow to generate SSA */
int ssa_generator_1(int x) {
    volatile int y = x;  /* Prevent optimization */
    int z;
    
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    for (int i = 0; i < y; ++i) {
        z += i;
        if (z > 100) {
            z = z / 2;
        }
    }
    
    int j = 0;
    while (j < 10) {
        z += j * j;
        j++;
    }
    
    do {
        z--;
    } while (z > 0);
    
    return z;
}

double ssa_generator_2(double a, double b) {
    double result = 0.0;
    volatile double temp = a;  /* Prevent optimization */
    
    for (int i = 0; i < 100; i++) {
        if (temp > b) {
            result += temp;
            temp = temp / 2.0;
        } else {
            result -= b;
            temp = temp * 2.0;
        }
        
        switch (i % 4) {
            case 0: result += 1.0; break;
            case 1: result -= 1.0; break;
            case 2: result *= 1.5; break;
            case 3: result /= 1.5; break;
        }
    }
    
    return result;
}

/* ========== BLOCK generation ========== */
/* Nested blocks and label addresses */
void block_generator(void) {
    /* Level 1 block */
    int block_var_1 = 1;
    {
        /* Level 2 block */
        int block_var_2 = 2;
        {
            /* Level 3 block */
            int block_var_3 = 3;
            block_var_1 = block_var_2 + block_var_3;
        }
        
        /* Another block at level 2 */
        {
            int another_block_var = 42;
            block_var_2 = another_block_var;
        }
    }
    
    /* Label address taking */
    void* label_ptr;
    my_label_1: label_ptr = &&my_label_1;
    my_label_2: label_ptr = &&my_label_2;
    
    /* Switch creates blocks */
    switch (block_var_1) {
        case 1: {
            int case_var_1 = 10;
            break;
        }
        case 2: {
            int case_var_2 = 20;
            break;
        }
        default: {
            int default_var = 30;
            break;
        }
    }
}

/* ========== CONSTRUCTOR generation ========== */
/* Struct initializers */
struct ComplexStruct {
    int a;
    double b;
    char c;
    float d[3];
};

union ComplexUnion {
    int i;
    float f;
    double d;
    char str[8];
};

/* Constructor nodes from initializers */
struct ComplexStruct global_struct = { 
    .a = 1, 
    .b = 2.0, 
    .c = 'X', 
    .d = {1.0f, 2.0f, 3.0f} 
};

union ComplexUnion global_union = { .f = 3.14f };

int global_array[10] = {1, 2, 3, [7] = 8, [9] = 10};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

struct NestedStruct nested = {
    .inner = { .a = 5, .b = 6.0, .c = 'Y', .d = {4.0f, 5.0f, 6.0f} },
    .extra = 100
};

/* Array with designated initializers */
int designated_array[20] = {
    [0] = 1,
    [5] = 2,
    [10] = 3,
    [15] = 4,
    [19] = 5
};

/* ========== C++ specific: TREE_BINFO generation ========== */
#ifdef __cplusplus

/* Simple inheritance */
class BaseClass1 {
public:
    int base_data1;
    virtual void base_func1() {}
};

class DerivedClass1 : public BaseClass1 {
public:
    int derived_data1;
    virtual void base_func1() override {}
};

/* Multiple inheritance */
class BaseClass2 {
public:
    int base_data2;
    virtual void base_func2() {}
};

class BaseClass3 {
public:
    int base_data3;
    virtual void base_func3() {}
};

class MultipleDerived : public BaseClass2, public BaseClass3 {
public:
    int derived_data2;
    virtual void base_func2() override {}
    virtual void base_func3() override {}
};

/* Virtual inheritance */
class VirtualBase {
public:
    int virtual_data;
    virtual void virtual_func() {}
};

class VirtualDerived1 : virtual public VirtualBase {
public:
    int derived_virtual1;
};

class VirtualDerived2 : virtual public VirtualBase {
public:
    int derived_virtual2;
};

class DiamondDerived : public VirtualDerived1, public VirtualDerived2 {
public:
    int diamond_data;
    virtual void virtual_func() override {}
};

/* Templates (may generate additional tree nodes) */
template<typename T>
class TemplateBase {
public:
    T template_data;
    virtual void template_func(T val) {}
};

class ConcreteDerived : public TemplateBase<int> {
public:
    int concrete_data;
    virtual void template_func(int val) override {}
};

#endif /* __cplusplus */

/* ========== OMP_CLAUSE generation ========== */
void omp_test_function(int size) {
    int i;
    double sum = 0.0;
    int* array = 0;
    
    if (size > 0) {
        array = (int*)__builtin_alloca(size * sizeof(int));
        for (i = 0; i < size; i++) {
            array[i] = i;
        }
    }
    
    /* Various OpenMP pragmas with different clauses */
    
    /* parallel with multiple clauses */
    #pragma omp parallel private(i) shared(array, size, sum) default(none)
    {
        #pragma omp for schedule(static, 4) nowait
        for (i = 0; i < size; i++) {
            #pragma omp atomic
            sum += array[i];
        }
    }
    
    /* sections */
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < size/2; i++) {
                sum += 1.0;
            }
        }
        
        #pragma omp section
        {
            for (i = size/2; i < size; i++) {
                sum += 2.0;
            }
        }
    }
    
    /* simd with reduction */
    #pragma omp simd reduction(+:sum) simdlen(8)
    for (i = 0; i < size; i++) {
        sum += array[i] * 0.5;
    }
    
    /* task with depend clause */
    int x = 0, y = 0;
    #pragma omp task depend(inout: x) shared(x)
    {
        x = 1;
    }
    
    #pragma omp task depend(in: x) depend(out: y) shared(x, y)
    {
        y = x + 1;
    }
    
    /* critical */
    #pragma omp critical
    {
        sum += 100.0;
    }
    
    /* barrier */
    #pragma omp barrier
    
    /* master */
    #pragma omp master
    {
        sum *= 2.0;
    }
    
    /* single */
    #pragma omp single
    {
        sum += 50.0;
    }
}

/* ========== Main driver ========== */
int main(int argc, char** argv) {
    int test_size = 100;
    
    /* Use identifiers */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Use multi-dimensional arrays */
    multi_array_1[0][0] = 1;
    multi_array_2[0][0][0] = 2;
    multi_array_3[0][0][0][0] = 3;
    
    /* Generate SSA nodes */
    int ssa_result = ssa_generator_1(42);
    double ssa_double = ssa_generator_2(3.14, 2.71);
    
    /* Generate blocks */
    block_generator();
    
    /* Use constructors */
    struct ComplexStruct local_struct = {
        .a = 10,
        .b = 20.0,
        .c = 'Z',
        .d = {10.0f, 11.0f, 12.0f}
    };
    
    int local_array[5] = {1, 2, [3] = 4, [4] = 5};
    
    /* C++ specific tests */
    #ifdef __cplusplus
    DerivedClass1 derived1;
    MultipleDerived multiple;
    DiamondDerived diamond;
    ConcreteDerived concrete;
    
    BaseClass1* base_ptr = &derived1;
    base_ptr->base_func1();
    
    VirtualBase* virtual_ptr = &diamond;
    virtual_ptr->virtual_func();
    #endif
    
    /* OpenMP test */
    #ifdef _OPENMP
    omp_test_function(test_size);
    #else
    /* Fallback for non-OpenMP compilation */
    double sum = 0.0;
    for (int i = 0; i < test_size; i++) {
        sum += i;
    }
    #endif
    
    /* Use function pointers */
    int (*func_ptr)(int, int, int, int);
    
    /* Use vector types */
    v4si vec1 = {1, 2, 3, 4};
    v8sf vec2 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    
    return (ssa_result + (int)sum + local_struct.a + vec1[0]) % 256;
}
