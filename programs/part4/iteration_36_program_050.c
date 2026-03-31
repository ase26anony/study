/* tree_coverage_test.c - Comprehensive test to cover tree_kind switch cases */

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ==================== IDENTIFIER_NODE generation ==================== */
/* Create numerous distinct identifiers in various contexts */

/* Global variable identifiers */
int unique_var_1, unique_var_2, unique_var_3;
static int static_unique_var_4;
volatile int volatile_unique_var_5;

/* Type identifiers via typedef */
typedef int my_type_1;
typedef my_type_1 my_type_2;
typedef unsigned long my_type_3;

/* Struct/union identifiers */
struct struct_id_1 {
    int member_1;
    double member_2;
};

union union_id_1 {
    int int_member;
    float float_member;
};

/* Enum identifiers */
enum enum_id_1 {
    ENUM_CONST_1,
    ENUM_CONST_2,
    ENUM_CONST_3
};

/* Function identifiers */
void func_1(void);
int func_2(int param_1, double param_2);
static void static_func_3(void);

/* ==================== TREE_VEC generation ==================== */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned g, signed char h, 
                 long long i, unsigned short j);

/* Multi-dimensional arrays */
int multi_dim_array_1[2][3][4];
int multi_dim_array_2[5][6];
static char multi_dim_array_3[3][3][3][3];

/* Vector types using GCC extension */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Function returning array pointer */
int (*func_returning_array_ptr(void))[10];

/* ==================== BLOCK generation ==================== */
/* Functions with nested blocks and label addresses */
void block_generator_1(void) {
    /* Outer block variable */
    int outer_var = 0;
    
    /* First nested block */
    {
        int block_var_1 = 1;
        {
            int deeply_nested = 2;
            outer_var += deeply_nested;
        }
    }
    
    /* Second nested block */
    {
        int block_var_2 = 3;
        static int static_in_block = 4;
        outer_var += block_var_2 + static_in_block;
    }
    
    /* Loop creates blocks */
    for (int i = 0; i < 10; i++) {
        int loop_var = i * 2;
        outer_var += loop_var;
    }
    
    /* Conditional blocks */
    if (outer_var > 0) {
        int if_block_var = 5;
        outer_var += if_block_var;
    } else {
        int else_block_var = 6;
        outer_var += else_block_var;
    }
    
    /* Switch creates blocks */
    switch (outer_var) {
        case 0: {
            int case_block_var = 7;
            break;
        }
        default: {
            int default_block_var = 8;
            break;
        }
    }
}

/* Function with computed goto using label addresses */
void label_address_generator(void) {
    void* label_ptr;
    int choice = 0;
    
    /* Take addresses of labels */
    label_ptr = &&label_a;
    
    if (choice == 0) {
        goto *label_ptr;
    }
    
label_a:
    {
        int label_a_var = 10;
    }
    
label_b:
    {
        int label_b_var = 20;
        label_ptr = &&label_c;
    }
    
label_c: ;
}

/* ==================== CONSTRUCTOR generation ==================== */
/* Struct initializers */
struct point {
    int x;
    int y;
    double z;
};

struct rectangle {
    struct point top_left;
    struct point bottom_right;
    int id;
};

/* Array initializers */
int array_init_1[5] = {1, 2, 3, 4, 5};
int array_init_2[10] = {[0]=1, [5]=50, [9]=100};
int array_init_3[3][2] = {{1,2}, {3,4}, {5,6}};

/* Union initializers */
union data {
    int i;
    float f;
    char str[20];
};

/* Complex designated initializers */
struct rectangle rect = {
    .top_left = {.x = 0, .y = 10, .z = 1.5},
    .bottom_right = {.x = 100, .y = 0, .z = 1.5},
    .id = 1
};

union data data_item = {.f = 3.14159f};

/* Nested initializers */
struct nested {
    int a;
    struct {
        int b;
        int c;
    } inner;
    int d;
} nested_var = {1, {2, 3}, 4};

/* ==================== SSA_NAME generation ==================== */
/* Functions with complex control flow to generate SSA form */
int ssa_generator_1(int x) {
    volatile int y = x;  /* volatile prevents optimization */
    int z = 0;
    
    /* Conditional creates phi nodes */
    if (y > 0) {
        y = y * 2;
        z = y + 1;
    } else {
        y = y - 1;
        z = y * 3;
    }
    
    /* Loop creates more phi nodes */
    for (int i = 0; i < y; ++i) {
        volatile int loop_var = i;
        z += loop_var;
        
        /* Nested condition in loop */
        if (z > 100) {
            z = z / 2;
        }
    }
    
    /* Another loop with break/continue */
    int j = 0;
    while (j < 10) {
        if (z > 200) {
            break;
        }
        z += j;
        j++;
        
        if (j % 2 == 0) {
            continue;
        }
        z -= 1;
    }
    
    return z;
}

int ssa_generator_2(int a, int b) {
    volatile int x = a;
    volatile int y = b;
    int result = 0;
    
    /* Complex conditional chain */
    if (x > y) {
        result = x - y;
    } else if (x < y) {
        result = y - x;
    } else {
        result = x * y;
    }
    
    /* Do-while loop */
    int counter = 0;
    do {
        result += counter;
        counter++;
        
        /* Switch inside loop */
        switch (result % 4) {
            case 0: result += 1; break;
            case 1: result += 2; break;
            case 2: result += 3; break;
            default: result += 4; break;
        }
    } while (counter < 5);
    
    return result;
}

/* ==================== C++ specific: TREE_BINFO generation ==================== */
#ifdef __cplusplus

/* Simple inheritance */
class Base1 {
public:
    int base1_data;
    virtual void base1_func() {}
    virtual ~Base1() {}
};

class Base2 {
public:
    float base2_data;
    virtual void base2_func() {}
    virtual ~Base2() {}
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
    double derived2_data;
    void base1_func() override {}
    void base2_func() override {}
};

/* Virtual inheritance */
class VirtualBase {
public:
    int virtual_data;
};

class Derived3 : virtual public VirtualBase {
public:
    int derived3_data;
};

class Derived4 : virtual public VirtualBase {
public:
    int derived4_data;
};

/* Diamond inheritance */
class Diamond : public Derived3, public Derived4 {
public:
    int diamond_data;
};

/* Templates with inheritance */
template<typename T>
class TemplateBase {
public:
    T template_data;
};

template<typename T>
class TemplateDerived : public TemplateBase<T> {
public:
    T additional_data;
};

#endif /* __cplusplus */

/* ==================== OpenMP: OMP_CLAUSE generation ==================== */
void openmp_test(int size) {
    int i;
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        arr[i] = i + 1;
    }
    
    /* Test various OpenMP constructs */
    
    /* 1. Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(arr, sum) default(none)
    {
        int local_sum = 0;
        
        /* 2. Parallel for with schedule clause */
        #pragma omp for schedule(static, 4) nowait
        for (i = 0; i < 100; i++) {
            local_sum += arr[i];
        }
        
        /* 3. Critical section */
        #pragma omp critical
        {
            sum += local_sum;
        }
    }
    
    /* 4. Parallel sections */
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
    
    /* 5. Reduction clause */
    sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* 6. SIMD with linear clause */
    #pragma omp simd linear(i:1) reduction(+:sum)
    for (i = 0; i < 100; i++) {
        sum += arr[i] * 2;
    }
    
    /* 7. Task with depend clause */
    int x = 0, y = 0;
    #pragma omp task depend(inout: x)
    {
        x = 100;
    }
    
    #pragma omp task depend(in: x) depend(out: y)
    {
        y = x * 2;
    }
    
    /* 8. Collapse clause for nested loops */
    int matrix[10][10];
    #pragma omp parallel for collapse(2)
    for (int row = 0; row < 10; row++) {
        for (int col = 0; col < 10; col++) {
            matrix[row][col] = row * col;
        }
    }
}

/* ==================== Main driver ==================== */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Use identifiers */
    unique_var_1 = 1;
    my_type_2 var2 = 2;
    
    /* Use multi-dimensional arrays */
    multi_dim_array_1[0][0][0] = 10;
    multi_dim_array_2[2][3] = 20;
    
    /* Call functions with complex prototypes */
    result = complex_func(1, 2L, 'a', 3.14, 4, 5.0f, 6, 'b', 7LL, 8);
    
    /* Generate blocks */
    block_generator_1();
    label_address_generator();
    
    /* Use constructors */
    struct point p1 = {.x = 1, .y = 2, .z = 3.0};
    struct point p2 = {10, 20, 30.0};
    
    int arr_init[3] = {[0]=100, [2]=300};
    
    /* Generate SSA names */
    result += ssa_generator_1(argc);
    result += ssa_generator_2(argc, result);
    
    /* C++ specific tests */
    #ifdef __cplusplus
    {
        Derived1 d1;
        Derived2 d2;
        Diamond d3;
        
        Base1* b1 = &d1;
        Base1* b2 = &d2;
        
        b1->base1_func();
        b2->base1_func();
        
        TemplateDerived<int> td;
        td.template_data = 42;
        td.additional_data = 24;
    }
    #endif
    
    /* OpenMP tests - conditionally compiled */
    #ifdef _OPENMP
    openmp_test(100);
    #endif
    
    /* Use vector types */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    
    /* Ensure all code is used to prevent optimization */
    return result + unique_var_1 + var2 + multi_dim_array_1[0][0][0] 
           + p1.x + arr_init[0] + vec1[0];
}

/* Implementation of declared functions */
int complex_func(int a, long b, char c, double d, short e,
                 float f, unsigned g, signed char h,
                 long long i, unsigned short j) {
    return a + b + c + d + e + f + g + h + i + j;
}

void func_1(void) {
    /* Label for identifier generation */
    func1_label: ;
}

int func_2(int param_1, double param_2) {
    return param_1 + (int)param_2;
}

static void static_func_3(void) {
    static int static_local = 0;
    static_local++;
}
