/* test_tree_coverage.c */
/* Compile with: gcc -O1 -fopenmp -c test_tree_coverage.c -o test_tree_coverage.o */
/* For C++ coverage: g++ -O2 -fopenmp -c test_tree_coverage.cpp -o test_tree_coverage.o */

/* 1. Generate IDENTIFIER_NODE trees */
/* Create numerous distinct identifiers */
int unique_var_1, unique_var_2, unique_var_3, unique_var_4;
float special_float_1, special_float_2;
char char_identifier_1, char_identifier_2;

/* Type identifiers */
typedef int my_type_1;
typedef long my_type_2;
typedef double my_type_3;
typedef unsigned my_type_4;

/* Function identifiers */
void func_1(void);
int func_2(int);
double func_3(double, double);

/* Label identifiers */
void label_func(void) {
    label_1: ;
    label_2: ;
    label_3: ;
}

/* 2. Create TREE_VEC nodes */
/* Complex function prototypes with many parameters */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned g, signed char h, 
                 long long i, void* j, int* k);

/* Multi-dimensional arrays */
int multi_dim_array[2][3][4][5];
float another_array[10][20][30];

/* Vector types using attribute */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* 3. TREE_BINFO nodes (C++ only) */
#ifdef __cplusplus
struct Base1 {
    int base1_data;
    virtual void base1_func() {}
};

struct Base2 {
    double base2_data;
    virtual void base2_func() {}
};

struct Derived : public Base1, public Base2 {
    char derived_data;
    void base1_func() override {}
    void base2_func() override {}
};

struct VirtualBase {
    int vdata;
};

struct VirtDerived : virtual VirtualBase {
    int derived_vdata;
};

struct MultiDerived : public Derived, virtual VirtualBase {
    int multi_data;
};
#endif

/* 4. Generate SSA_NAME nodes */
int ssa_generating_func(int x) {
    volatile int y = x;  /* Prevent optimization */
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
            z = z / 2;
        }
    }
    
    int j = 0;
    while (j < z) {
        j += (y % 2) + 1;
        switch (j) {
            case 1: z += 1; break;
            case 2: z += 2; break;
            default: z += 3;
        }
    }
    
    return z;
}

/* Another SSA-heavy function */
double complex_ssa(double a, double b) {
    double x = a;
    double y = b;
    
    for (int i = 0; i < 100; i++) {
        x = x * y + i;
        y = y - x / (i + 1);
        
        if (x > y) {
            double temp = x;
            x = y;
            y = temp;
        }
    }
    
    return x + y;
}

/* 5. Produce BLOCK nodes */
void block_generating_func(void) {
    /* Outer block */
    int outer_var = 0;
    
    {
        /* Inner block 1 */
        int inner_var_1 = 1;
        {
            /* Nested inner block */
            int deeply_nested = inner_var_1 * 2;
            outer_var += deeply_nested;
        }
    }
    
    {
        /* Inner block 2 */
        float float_var = 3.14f;
        {
            /* Another nested block */
            double double_var = float_var * 2.0;
            outer_var += (int)double_var;
        }
    }
    
    /* Label address taking */
    void* label_ptr;
    
    block_label_1:
    label_ptr = &&block_label_2;
    outer_var++;
    
    if (outer_var > 10) {
        goto *label_ptr;
    }
    
    block_label_2:
    {
        /* Block with label */
        int final_var = outer_var * 2;
        outer_var = final_var;
    }
}

/* 6. Create CONSTRUCTOR nodes */
/* Struct initializers */
struct ComplexStruct {
    int a;
    double b;
    float c;
    char d;
    int* e;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int outer;
    float arr[3];
};

/* Array initializers */
int init_array[10] = {1, 2, 3, [7] = 8, [9] = 10};
float float_array[5] = {1.1f, 2.2f, [4] = 5.5f};

/* Union initializer */
union DataUnion {
    int i;
    float f;
    double d;
    char str[16];
};

/* Designated initializers */
struct ComplexStruct cs1 = { .a = 1, .b = 2.0, .c = 3.0f, .d = 'X' };
struct ComplexStruct cs2 = { 10, 20.0, 30.0f, 'Y', &init_array[0] };

/* Nested designated initializer */
struct NestedStruct ns = {
    .inner = { .a = 100, .b = 200.0, .c = 300.0f, .d = 'Z' },
    .outer = 400,
    .arr = { 1.0f, 2.0f, 3.0f }
};

/* Union initializers */
union DataUnion du1 = { .i = 42 };
union DataUnion du2 = { .f = 3.14159f };
union DataUnion du3 = { .d = 2.71828 };

/* Array with mixed initialization */
int mixed_array[20] = { [0] = 1, [10] = 2, [19] = 3, 4, 5 };

/* 7. Generate OMP_CLAUSE nodes */
void openmp_functions(void) {
    int i, j, k;
    int sum = 0;
    int product = 1;
    float float_sum = 0.0f;
    int array[1000];
    int matrix[100][100];
    
    /* Initialize arrays */
    for (i = 0; i < 1000; i++) {
        array[i] = i;
    }
    
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix[i][j] = i * j;
        }
    }
    
    /* Various OpenMP pragmas with different clauses */
    
    /* Parallel region with multiple clauses */
    #pragma omp parallel private(i) shared(array, sum) default(none) num_threads(4)
    {
        #pragma omp for reduction(+:sum) schedule(static, 10) nowait
        for (i = 0; i < 1000; i++) {
            sum += array[i];
        }
        
        #pragma omp single
        {
            sum = sum * 2;
        }
    }
    
    /* Parallel for with collapse */
    #pragma omp parallel for private(i, j) collapse(2) reduction(*:product) \
            schedule(dynamic) if(1000 > 100)
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            product *= (matrix[i][j] + 1);
        }
    }
    
    /* SIMD pragma */
    #pragma omp simd reduction(+:float_sum) simdlen(8) aligned(array:32)
    for (i = 0; i < 1000; i++) {
        float_sum += array[i] * 0.5f;
    }
    
    /* Sections */
    #pragma omp parallel sections private(i, j, k)
    {
        #pragma omp section
        {
            for (i = 0; i < 500; i++) {
                array[i] += sum;
            }
        }
        
        #pragma omp section
        {
            for (j = 500; j < 1000; j++) {
                array[j] += product;
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
        { y = x + 1; }
        
        #pragma omp task depend(in: y)
        { sum += y; }
    }
    
    /* Critical section */
    #pragma omp critical
    {
        sum += 1000;
    }
    
    /* Barrier */
    #pragma omp barrier
    
    /* Master section */
    #pragma omp master
    {
        product += 100;
    }
    
    /* Atomic operation */
    #pragma omp atomic
    sum += product;
}

/* Main driver function */
int main(int argc, char** argv) {
    /* Use all the generated constructs */
    
    /* 1. Use identifiers */
    unique_var_1 = 10;
    unique_var_2 = unique_var_1 * 2;
    
    /* 2. Use TREE_VEC related constructs */
    int result = complex_func(1, 2L, 'a', 3.0, 4, 5.0f, 6, 'b', 7LL, 0, &unique_var_1);
    multi_dim_array[0][1][2][3] = result;
    
    /* 3. Use C++ inheritance (if compiled as C++) */
    #ifdef __cplusplus
    Derived d;
    d.base1_data = 100;
    d.base2_data = 200.0;
    d.derived_data = 'D';
    
    Base1* b1_ptr = &d;
    Base2* b2_ptr = &d;
    
    b1_ptr->base1_func();
    b2_ptr->base2_func();
    
    VirtDerived vd;
    vd.derived_vdata = 300;
    vd.vdata = 400;
    
    MultiDerived md;
    md.multi_data = 500;
    #endif
    
    /* 4. Generate SSA names */
    int ssa_result = ssa_generating_func(argc);
    double complex_result = complex_ssa(ssa_result * 1.0, ssa_result * 2.0);
    
    /* 5. Use blocks */
    block_generating_func();
    
    /* 6. Use constructors */
    struct ComplexStruct local_cs = { 
        .a = ssa_result, 
        .b = complex_result, 
        .c = 99.9f, 
        .d = 'L' 
    };
    
    ns.inner.a += local_cs.a;
    du1.i = local_cs.a;
    
    /* 7. Use OpenMP */
    openmp_functions();
    
    /* Use all variables to prevent optimization */
    volatile int prevent_opt = unique_var_1 + unique_var_2 + ssa_result + 
                               (int)complex_result + local_cs.a + ns.outer;
    
    return prevent_opt % 256;
}

/* Function definitions */
int complex_func(int a, long b, char c, double d, short e, 
                 float f, unsigned g, signed char h, 
                 long long i, void* j, int* k) {
    return a + (int)b + (int)c + (int)d + e + (int)f + g + (int)h + (int)i + (int)(long)j + *k;
}

void func_1(void) {
    /* Empty but referenced */
}

int func_2(int x) {
    return x * 2;
}

double func_3(double a, double b) {
    return a + b;
}
