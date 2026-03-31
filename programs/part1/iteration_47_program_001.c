/* Compile with: g++ -O2 -g -fopenmp -fdump-tree-all -xc++ tree_coverage.cc -o tree_coverage */

/* Force generation of IDENTIFIER_NODE trees */
#define CONCAT(a, b) a##b
#define UNIQUE_VAR(base) CONCAT(base, __LINE__)

/* Many distinct identifiers for IDENTIFIER_NODE coverage */
#define DECLARE_VARS \
    int UNIQUE_VAR(var); \
    float UNIQUE_VAR(fvar); \
    double UNIQUE_VAR(dvar); \
    char UNIQUE_VAR(cvar); \
    long UNIQUE_VAR(lvar); \
    short UNIQUE_VAR(svar); \
    unsigned UNIQUE_VAR(uvar); \
    signed UNIQUE_VAR(sigvar);

/* Complex class hierarchy for TREE_BINFO */
class Base1 {
public:
    virtual ~Base1() {}
    virtual void method1() = 0;
    int base1_data;
};

class Base2 {
public:
    virtual ~Base2() {}
    virtual void method2() = 0;
    float base2_data;
};

class VirtualBase {
public:
    virtual ~VirtualBase() {}
    virtual void vmethod() = 0;
    double vdata;
};

class Derived : public Base1, public Base2, virtual public VirtualBase {
public:
    virtual ~Derived() override {}
    virtual void method1() override { base1_data = 1; }
    virtual void method2() override { base2_data = 2.0f; }
    virtual void vmethod() override { vdata = 3.0; }
    
    /* Member pointer for additional BINFO usage */
    int Derived::* member_ptr;
    
    int derived_data;
};

/* Function with many parameters for TREE_VEC generation */
int complex_function_with_many_params(
    int p1, float p2, double p3, char p4, long p5,
    short p6, unsigned p7, signed p8, int p9, float p10,
    double p11, char p12, long p13, short p14, unsigned p15,
    int p16, float p17, double p18, char p19, long p20
) {
    /* Nested blocks for BLOCK nodes */
    {
        int block_var1 = 1;
        {
            int block_var2 = 2;
            {
                int block_var3 = 3;
                /* Use all block variables */
                block_var1 = block_var2 + block_var3;
            }
        }
    }
    
    /* Complex control flow for SSA_NAME generation */
    int ssa_var = 0;
    for (int i = 0; i < 100; ++i) {
        if (i % 2 == 0) {
            ssa_var = ssa_var + i * 2;
        } else {
            ssa_var = ssa_var - i;
        }
        
        /* Self-referential assignment for SSA */
        ssa_var = ssa_var * 2 - ssa_var / 3;
    }
    
    /* Variable Length Array for TREE_VEC */
    int vla_size = 50;
    int vla[vla_size];
    
    /* Initialize VLA with values */
    for (int i = 0; i < vla_size; ++i) {
        vla[i] = i * ssa_var;
    }
    
    /* CONSTRUCTOR nodes - aggregate initializers */
    struct Point {
        int x;
        int y;
        int z;
    };
    
    /* Various constructor forms */
    int array_constructor[5] = {1, 2, 3, 4, 5};
    struct Point point_constructor = {10, 20, 30};
    struct Point designated_constructor = {.x = 100, .y = 200, .z = 300};
    
    /* Compound literal */
    struct Point* ptr = &(struct Point){.x = 1000, .y = 2000, .z = 3000};
    
    /* Complex struct with nested initializers */
    struct Nested {
        int a;
        struct Point p;
        int b[3];
    };
    
    struct Nested nested_constructor = {
        42,
        {1, 2, 3},
        {4, 5, 6}
    };
    
    /* Use __label__ for identifier nodes */
    __label__ label1, label2, label3;
    
    /* Use builtins with type names (identifiers) */
    if (__builtin_types_compatible_p(int, typeof(ssa_var))) {
        ssa_var += 100;
    }
    
    /* Complex expression with choose_expr */
    ssa_var = __builtin_choose_expr(
        sizeof(int) == 4,
        ssa_var * 2,
        ssa_var / 2
    );
    
    return ssa_var + p1 + array_constructor[0] + point_constructor.x;
}

/* OpenMP function for OMP_CLAUSE generation */
void openmp_kernel(int* data, int size) {
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(private_var) shared(data) reduction(+:sum) \
        schedule(dynamic) num_threads(4) if(size > 1000)
    for (int i = 0; i < size; ++i) {
        private_var = i * 2;
        data[i] = private_var;
        sum += data[i];
    }
    
    /* Target offloading with data mapping */
    #pragma omp target teams distribute parallel for \
        map(tofrom: data[0:size]) map(to: size) \
        device(0) depend(inout: data)
    for (int i = 0; i < size; ++i) {
        data[i] = data[i] * 3;
    }
    
    /* Task with dependencies */
    int task_var = 0;
    #pragma omp task depend(inout: task_var) priority(high)
    {
        task_var = sum;
    }
    
    #pragma omp taskwait
}

/* Function with extreme SSA complexity */
int ssa_stress_test(int n) {
    int a = 1, b = 2, c = 3, d = 4, e = 5;
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        /* Complex web of dependencies */
        if (i % 3 == 0) {
            a = b + c;
            b = c * d;
        } else if (i % 3 == 1) {
            c = d - e;
            d = e + a;
        } else {
            e = a * b;
            a = c + d;
        }
        
        /* Phi node creation */
        int temp = (i % 2 == 0) ? a : b;
        temp = (i % 3 == 0) ? temp + c : temp - d;
        
        /* Multiple redefinitions */
        result = result + temp;
        result = result * 2 - result / 3;
        result = result ^ (result << 2);
    }
    
    return result;
}

/* Main driver that uses all constructs */
int main() {
    /* Generate many identifiers */
    DECLARE_VARS
    DECLARE_VARS  /* More identifiers */
    
    /* Use the identifiers */
    var1 = 1; fvar2 = 2.0f; dvar3 = 3.0; cvar4 = 'A';
    lvar5 = 5L; svar6 = 6; uvar7 = 7U; sigvar8 = 8;
    
    /* C++ class hierarchy usage for BINFO */
    Derived* d = new Derived();
    Base1* b1 = d;
    Base2* b2 = d;
    VirtualBase* vb = d;
    
    /* Dynamic cast requiring RTTI/BINFO */
    Derived* d2 = dynamic_cast<Derived*>(b1);
    if (d2) {
        d2->method1();
        d2->method2();
        d2->vmethod();
    }
    
    /* Member pointer access */
    int Derived::* ptr = &Derived::derived_data;
    d->*ptr = 42;
    
    /* Call function with many parameters (TREE_VEC) */
    int result = complex_function_with_many_params(
        1, 2.0f, 3.0, 'A', 5L, 6, 7U, 8, 9, 10.0f,
        11.0, 'B', 13L, 14, 15U, 16, 17.0f, 18.0, 'C', 20L
    );
    
    /* SSA stress test */
    int ssa_result = ssa_stress_test(1000);
    
    /* OpenMP computation */
    const int data_size = 10000;
    int* data = new int[data_size];
    
    openmp_kernel(data, data_size);
    
    /* Verify computation */
    int final_sum = 0;
    for (int i = 0; i < data_size; ++i) {
        final_sum += data[i];
    }
    
    /* Use all results to prevent optimization */
    int final_result = result + ssa_result + final_sum + d->derived_data;
    
    delete[] data;
    delete d;
    
    return final_result % 256;  /* Return non-zero to indicate execution */
}
