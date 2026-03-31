/* test_tree_coverage.c - Comprehensive test for GCC tree node coverage */

/* Prevent excessive optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* For TREE_VEC - vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* For CONSTRUCTOR - complex structs */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* For IDENTIFIER_NODE - generate many unique identifiers */
#define GEN_ID(n) identifier_##n
#define GEN_FUNC(n) func_##n

/* Helper to prevent optimization */
static volatile int volatile_sink;

/* Function to use many identifiers */
NOINLINE static int GEN_FUNC(identifiers)(void) {
    /* Generate many IDENTIFIER_NODE instances */
    int GEN_ID(a) = 1;
    int GEN_ID(b) = 2;
    int GEN_ID(c) = 3;
    int GEN_ID(d) = 4;
    int GEN_ID(e) = 5;
    int GEN_ID(f) = 6;
    int GEN_ID(g) = 7;
    int GEN_ID(h) = 8;
    int GEN_ID(i) = 9;
    int GEN_ID(j) = 10;
    
    /* Use in complex expression */
    int result = GEN_ID(a) + GEN_ID(b) * GEN_ID(c) - GEN_ID(d) / GEN_ID(e);
    result += GEN_ID(f) | GEN_ID(g) & GEN_ID(h) ^ GEN_ID(i) % GEN_ID(j);
    
    /* Create nested blocks for BLOCK nodes */
    {
        int block_local_1 = result * 2;
        {
            int block_local_2 = block_local_1 + 3;
            {
                int block_local_3 = block_local_2 - 4;
                result = block_local_3;
            }
        }
    }
    
    /* Another block with different scope */
    if (result > 0) {
        int if_block_var = result * 3;
        result = if_block_var;
    } else {
        int else_block_var = result / 2;
        result = else_block_var;
    }
    
    volatile_sink = result;
    return result;
}

/* Function for TREE_VEC and SSA_NAME */
NOINLINE static void GEN_FUNC(vector_ssa)(void) {
    /* TREE_VEC - vector operations */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4si vec3 = {9, 10, 11, 12};
    
    /* Complex vector expressions */
    v4si result_vec = vec1 + vec2 * vec3 - vec1 / (vec2 + 1);
    
    /* SSA_NAME - complex control flow with variable modifications */
    int ssa_var = 0;
    int i;
    
    /* Loop with multiple assignments to create SSA form */
    for (i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            ssa_var = ssa_var + i * 2;
        } else if (i % 3 == 1) {
            ssa_var = ssa_var - i;
        } else {
            ssa_var = ssa_var * (i + 1);
        }
        
        /* Nested block inside loop */
        {
            int loop_block_var = ssa_var % 7;
            ssa_var += loop_block_var;
        }
    }
    
    /* More SSA complexity */
    int x = 10, y = 20, z = 30;
    for (i = 0; i < 50; i++) {
        if (x > y) {
            z = x - y;
            x = z * 2;
        } else {
            z = y - x;
            y = z / 2;
        }
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    volatile_sink = ssa_var + x + y + z;
    volatile_sink += result_vec[0];
}

/* Function for CONSTRUCTOR nodes */
NOINLINE static struct NestedStruct GEN_FUNC(aggregate_init)(int p1, int p2, float p3) {
    /* Non-constant initializers for CONSTRUCTOR nodes */
    struct NestedStruct ns = {
        .inner = {
            .a = p1 * 2,
            .b = p2 + p1,
            .c = p1 % (p2 + 1),
            .f = p3 * 2.0f,
            .d = (double)p1 / (p2 + 1.0)
        },
        .extra = p1 ^ p2
    };
    
    /* Array with non-constant initializer */
    int dynamic_array[4] = {
        p1,
        p2,
        p1 + p2,
        p1 * p2
    };
    
    /* Struct array with designated initializers */
    struct ComplexStruct cs_array[2] = {
        { .a = p1, .b = p2, .c = p1-p2, .f = p3, .d = p3*2 },
        { .a = p2, .b = p1, .c = p2-p1, .f = p3/2, .d = p3/4 }
    };
    
    ns.extra += dynamic_array[0] + cs_array[1].a;
    return ns;
}

/* Function for OMP_CLAUSE nodes */
#ifdef _OPENMP
#include <omp.h>
NOINLINE static int GEN_FUNC(omp_test)(int* data, int n) {
    int sum = 0;
    int i;
    
    /* OpenMP with multiple clauses for OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) shared(data, n) reduction(+:sum) \
            schedule(dynamic, 4) num_threads(4) if(n > 1000)
    for (i = 0; i < n; i++) {
        /* Nested block in OpenMP region */
        {
            int local_calc = data[i] * (i + 1);
            sum += local_calc % 7;
        }
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(i) shared(data, n, max_val) \
            default(none)
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (i = 0; i < n/2; i++) {
                section_sum += data[i];
            }
            #pragma omp critical
            {
                max_val = section_sum > max_val ? section_sum : max_val;
            }
        }
        
        #pragma omp section
        {
            int section_sum = 0;
            for (i = n/2; i < n; i++) {
                section_sum += data[i];
            }
            #pragma omp critical
            {
                max_val = section_sum > max_val ? section_sum : max_val;
            }
        }
    }
    
    /* OpenMP task with clauses */
    int task_result = 0;
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: task_result) priority(high)
            {
                task_result = sum * 2;
            }
        }
    }
    
    volatile_sink = sum + max_val + task_result;
    return sum;
}
#endif

/* For TREE_BINFO - C++ class hierarchy */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual int method() { return 1; }
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

class SecondDerived : public DerivedClass {
public:
    virtual int method() override { return 3; }
    int second_data;
};

NOINLINE static int GEN_FUNC(cpp_test)(void) {
    BaseClass* obj1 = new DerivedClass();
    BaseClass* obj2 = new SecondDerived();
    DerivedClass* obj3 = new SecondDerived();
    
    int result = obj1->method() + obj2->method() + obj3->method();
    
    /* Use dynamic_cast to trigger BINFO usage */
    DerivedClass* casted = dynamic_cast<DerivedClass*>(obj2);
    if (casted) {
        result += casted->derived_data;
    }
    
    delete obj1;
    delete obj2;
    delete obj3;
    
    volatile_sink = result;
    return result;
}
#endif

/* Main orchestrator */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test IDENTIFIER_NODE and BLOCK */
    result += GEN_FUNC(identifiers)();
    
    /* Test TREE_VEC and SSA_NAME */
    GEN_FUNC(vector_ssa)();
    
    /* Test CONSTRUCTOR */
    struct NestedStruct ns = GEN_FUNC(aggregate_init)(argc, 42, 3.14f);
    result += ns.inner.a + ns.extra;
    
    /* Test OMP_CLAUSE */
#ifdef _OPENMP
    int data[100];
    for (int i = 0; i < 100; i++) {
        data[i] = i * (argc + 1);
    }
    result += GEN_FUNC(omp_test)(data, 100);
#endif
    
    /* Test TREE_BINFO (C++ only) */
#ifdef __cplusplus
    result += GEN_FUNC(cpp_test)();
#endif
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
    
    /* Use result to affect return value */
    return result % 256;
}
