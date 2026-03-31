/* tree_coverage.c - Comprehensive test to cover GCC tree node kinds */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3;
static int static_var_1, static_var_2;
volatile int volatile_var_1;

typedef int my_type_1, my_type_2, my_type_3;
typedef long long_type_1;
typedef double double_type_1;

enum my_enum_1 { ENUM_VAL_1, ENUM_VAL_2, ENUM_VAL_3 };
enum my_enum_2 { ANOTHER_ENUM_1, ANOTHER_ENUM_2 };

struct struct_tag_1;
union union_tag_1;

/* Function declarations with various identifiers */
void func_1(void);
int func_2(int param_1, long param_2);
double func_3(char char_param, float float_param);

/* ==================== TREE_VEC generation ==================== */
/* Multi-dimensional arrays create TREE_VEC nodes */
int multi_dim_array_1[2][3][4];
int multi_dim_array_2[5][6];
char char_array_3d[3][4][5];

/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned g, signed char h, int* i, void* j);

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* ==================== BLOCK generation ==================== */
/* Functions with nested blocks and label addresses */
void block_test_function(void) {
    /* Outer block */
    int outer_var = 1;
    
    /* First nested block */
    {
        int inner_var_1 = 2;
        static int static_block_var;
        
        /* Deeper nested block */
        {
            int deeper_var = 3;
            deeper_var += outer_var + inner_var_1;
        }
    }
    
    /* Second nested block */
    {
        int inner_var_2 = 4;
        
        /* Label for address-taking */
        void* label_ptr;
        my_label_1:
        label_ptr = &&my_label_1;
        
        /* Another label */
        another_label:
        goto another_label;
    }
    
    /* Loop with its own block */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * 2;
        if (loop_var > 5) {
            int if_block_var = loop_var + 1;
        }
    }
}

/* ==================== CONSTRUCTOR generation ==================== */
/* Struct with initializers */
struct point_3d {
    int x;
    int y;
    int z;
    double weight;
};

struct complex_struct {
    int id;
    char name[32];
    struct point_3d location;
    float values[8];
};

/* Array and struct initializers */
int int_array[10] = {1, 2, 3, [7] = 8, [9] = 10};
struct point_3d origin = {0, 0, 0, 1.0};
struct point_3d points[3] = {
    {1, 2, 3, 1.5},
    {4, 5, 6, 2.0},
    [2] = {7, 8, 9, 2.5}
};

/* Designated initializers */
struct complex_struct item = {
    .id = 1001,
    .name = "test_item",
    .location = {.x = 10, .y = 20, .z = 30, .weight = 3.14},
    .values = {1.1f, 2.2f, [5] = 5.5f, [7] = 7.7f}
};

/* Union initializer */
union data_union {
    int int_val;
    float float_val;
    double double_val;
    char string_val[16];
};

union data_union data = {.float_val = 3.14159f};

/* ==================== SSA_NAME generation ==================== */
/* Function with complex control flow for SSA */
int ssa_test_function(int input) {
    int x = input;
    int y = 0;
    volatile int vol = 0; /* Prevent optimization */
    
    /* Conditional creates phi nodes */
    if (x > 0) {
        y = x * 2;
        vol = 1;
    } else {
        y = x / 2;
        vol = 2;
    }
    
    /* Loop creates more SSA names */
    int sum = 0;
    for (int i = 0; i < y; i++) {
        int temp = i * i;
        sum += temp;
        
        /* Nested condition */
        if (temp % 2 == 0) {
            sum += 1;
        } else {
            sum -= 1;
        }
    }
    
    /* Another loop with break/continue */
    int j = 0;
    while (1) {
        if (j > 100) break;
        
        int inner_var = j * 3;
        if (inner_var % 7 == 0) {
            continue;
        }
        
        sum += inner_var;
        j++;
    }
    
    /* Switch statement */
    switch (sum % 4) {
        case 0:
            x = sum + 1;
            break;
        case 1:
            x = sum - 1;
            break;
        case 2:
            x = sum * 2;
            break;
        default:
            x = sum / 2;
    }
    
    return x + y + vol;
}

/* ==================== OpenMP Clause generation ==================== */
void openmp_test_function(int size) {
    int i;
    int sum = 0;
    int array[1000];
    
    /* Initialize array */
    for (i = 0; i < 1000; i++) {
        array[i] = i + 1;
    }
    
    /* Various OpenMP pragmas with different clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(array, sum) num_threads(4)
    {
        int local_sum = 0;
        
        /* Parallel for with schedule clause */
        #pragma omp for schedule(static, 16) nowait
        for (i = 0; i < 1000; i++) {
            local_sum += array[i];
        }
        
        #pragma omp atomic
        sum += local_sum;
    }
    
    /* Another parallel region with reduction */
    int max_val = 0;
    int min_val = 1000000;
    
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        private(i) schedule(dynamic)
    for (i = 0; i < 1000; i++) {
        if (array[i] > max_val) max_val = array[i];
        if (array[i] < min_val) min_val = array[i];
    }
    
    /* SIMD loop */
    float float_array[1000];
    float float_sum = 0.0f;
    
    #pragma omp simd reduction(+:float_sum) simdlen(8)
    for (i = 0; i < 1000; i++) {
        float_array[i] = i * 0.1f;
        float_sum += float_array[i];
    }
    
    /* Sections */
    int section_result[3] = {0};
    
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            for (i = 0; i < 333; i++) {
                section_result[0] += array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = 333; i < 666; i++) {
                section_result[1] += array[i];
            }
        }
        
        #pragma omp section
        {
            for (i = 666; i < 1000; i++) {
                section_result[2] += array[i];
            }
        }
    }
    
    /* Task with depend clause */
    int task_x = 0, task_y = 0;
    
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: task_x)
            { task_x = 1; }
            
            #pragma omp task depend(out: task_y)
            { task_y = 2; }
            
            #pragma omp task depend(in: task_x, task_y)
            { sum = task_x + task_y; }
        }
    }
}

/* ==================== C++ specific code for TREE_BINFO ==================== */
#ifdef __cplusplus

/* Base classes for inheritance */
class BaseClass1 {
public:
    int base_data1;
    virtual void base_method1() {}
    virtual ~BaseClass1() {}
};

class BaseClass2 {
public:
    float base_data2;
    virtual void base_method2() {}
    virtual ~BaseClass2() {}
};

/* Virtual base class */
class VirtualBase {
public:
    int virtual_data;
    virtual void virtual_method() {}
    virtual ~VirtualBase() {}
};

/* Single inheritance */
class DerivedSingle : public BaseClass1 {
public:
    int derived_data;
    void base_method1() override {}
};

/* Multiple inheritance */
class DerivedMultiple : public BaseClass1, public BaseClass2 {
public:
    double derived_data_multiple;
    void base_method1() override {}
    void base_method2() override {}
};

/* Virtual inheritance */
class DerivedVirtual : public virtual VirtualBase {
public:
    int derived_virtual_data;
    void virtual_method() override {}
};

/* Diamond inheritance with virtual base */
class MostDerived : public DerivedMultiple, public DerivedVirtual {
public:
    int most_derived_data;
    void base_method1() override {}
    void base_method2() override {}
    void virtual_method() override {}
};

/* Template class */
template<typename T>
class TemplateClass : public BaseClass1 {
public:
    T template_data;
    TemplateClass(T val) : template_data(val) {}
    virtual void template_method() {}
};

/* Function to exercise C++ inheritance */
void cpp_inheritance_test() {
    DerivedSingle ds;
    DerivedMultiple dm;
    DerivedVirtual dv;
    MostDerived md;
    TemplateClass<int> tc(42);
    
    BaseClass1* ptr1 = &ds;
    BaseClass2* ptr2 = &dm;
    VirtualBase* ptr3 = &dv;
    
    /* Dynamic casts */
    DerivedSingle* dsp = dynamic_cast<DerivedSingle*>(ptr1);
    DerivedMultiple* dmp = dynamic_cast<DerivedMultiple*>(ptr2);
    
    /* Virtual calls */
    ptr1->base_method1();
    ptr2->base_method2();
    ptr3->virtual_method();
    
    /* Access through different paths */
    md.base_method1();
    md.base_method2();
    md.virtual_method();
}

#endif /* __cplusplus */

/* ==================== Main driver function ==================== */
int main(int argc, char** argv) {
    /* Use command line arg to conditionally enable features */
    int use_openmp = (argc > 1);
    int use_cpp_features = 0;
    
    /* Exercise identifier nodes */
    unique_var_1 = 1;
    unique_var_2 = unique_var_1 * 2;
    
    /* Exercise TREE_VEC nodes */
    multi_dim_array_1[0][0][0] = 42;
    int val = multi_dim_array_2[2][3];
    
    /* Exercise BLOCK nodes */
    block_test_function();
    
    /* Exercise CONSTRUCTOR nodes */
    struct point_3d pt = {10, 20, 30, 2.5};
    int arr_init[5] = {[1] = 100, [3] = 300};
    
    /* Exercise SSA_NAME nodes */
    int ssa_result = ssa_test_function(argc);
    
    /* Exercise OpenMP clauses if enabled */
    if (use_openmp) {
        openmp_test_function(1000);
    }
    
    #ifdef __cplusplus
    use_cpp_features = 1;
    /* Exercise C++ inheritance (TREE_BINFO nodes) */
    cpp_inheritance_test();
    #endif
    
    /* Use all variables to prevent optimization */
    return unique_var_1 + unique_var_2 + val + ssa_result + 
           pt.x + arr_init[1] + use_openmp + use_cpp_features;
}

/* Additional functions to ensure they're processed */
int complex_func(int a, long b, char c, double d, short e,
                 float f, unsigned g, signed char h, int* i, void* j) {
    return a + b + c + d + e + f + g + h + *i;
}

void func_1(void) {
    /* Label for goto */
    func_1_label:
    goto func_1_label;
}

int func_2(int param_1, long param_2) {
    return param_1 + param_2;
}

double func_3(char char_param, float float_param) {
    return char_param + float_param;
}
