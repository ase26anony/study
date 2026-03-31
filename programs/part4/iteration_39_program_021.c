/* test_tree_coverage.c - Comprehensive test to trigger specific tree node cases */

/* Prevent function inlining to ensure tree nodes are preserved */
#define NOINLINE __attribute__((noinline))

/* Generate many unique identifiers */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Use volatile to prevent optimization */
volatile int global_volatile = 0;

/* ==================== IDENTIFIER_NODE and BLOCK coverage ==================== */
NOINLINE void test_identifiers_and_blocks(int iterations) {
    /* Generate many unique identifiers */
    int MAKE_ID(0) = 1;
    int MAKE_ID(1) = 2;
    int MAKE_ID(2) = 3;
    int MAKE_ID(3) = 4;
    int MAKE_ID(4) = 5;
    int MAKE_ID(5) = 6;
    int MAKE_ID(6) = 7;
    int MAKE_ID(7) = 8;
    int MAKE_ID(8) = 9;
    int MAKE_ID(9) = 10;
    
    /* Nested blocks creating BLOCK nodes */
    {
        int block_local_1 = MAKE_ID(0) + MAKE_ID(1);
        {
            int block_local_2 = block_local_1 * 2;
            {
                int block_local_3 = block_local_2 / 3;
                global_volatile += block_local_3;
            }
        }
    }
    
    /* More blocks in loops */
    for (int i = 0; i < iterations; i++) {
        /* Inner block */
        {
            int loop_block_var = i * 2;
            if (loop_block_var > 10) {
                /* Another nested block */
                {
                    int inner_if_var = loop_block_var - 5;
                    global_volatile += inner_if_var;
                }
            }
        }
    }
}

/* ==================== TREE_VEC coverage ==================== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

NOINLINE v4si test_vector_operations(v4si a, v4si b) {
    /* Various vector operations to generate TREE_VEC nodes */
    v4si result1 = a + b;
    v4si result2 = a * b;
    v4si result3 = result1 - result2;
    
    /* Vector comparisons */
    v4si mask = a > b;
    v4si result4 = result3 & mask;
    
    /* Mix with scalar operations */
    for (int i = 0; i < 4; i++) {
        result4[i] += i;  /* Vector element access */
    }
    
    return result4;
}

/* ==================== SSA_NAME coverage ==================== */
NOINLINE int test_ssa_formation(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex control flow to induce SSA form */
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            x = i * 2;
        } else if (i % 3 == 1) {
            x = i * 3;
        } else {
            x = i * 4;
        }
        
        /* Phi node will be created for x */
        y += x;
        
        /* Another SSA variable with multiple assignments */
        if (y > 100) {
            z = y - 50;
        } else {
            z = y + 50;
        }
        
        /* Use volatile to prevent optimization */
        asm volatile("" : "+r" (z) : : "memory");
    }
    
    /* More SSA complexity */
    int w = 0;
    for (int i = 0; i < n; i++) {
        w = (w * 3 + i) % 17;
        if (w > 8) {
            w = w / 2;
        }
    }
    
    return y + z + w;
}

/* ==================== CONSTRUCTOR coverage ==================== */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE struct NestedStruct test_aggregate_init(int val1, float val2, double val3) {
    /* Non-constant aggregate initializations creating CONSTRUCTOR nodes */
    
    /* Struct with designated initializer */
    struct ComplexStruct s1 = {
        .a = val1 * 2,
        .b = val2 + 1.0f,
        .c = val3 / 2.0,
        .d = 'A' + (val1 % 26)
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        val1,
        val1 + 1,
        val1 * 2,
        test_ssa_formation(3)  /* Function call in initializer */
    };
    
    /* Nested struct initialization */
    struct NestedStruct nested = {
        .inner = {
            .a = arr[0] + arr[1],
            .b = s1.b * 2.0f,
            .c = s1.c - val3,
            .d = s1.d + 1
        },
        .extra = global_volatile
    };
    
    /* Union with constructor */
    union MixedUnion {
        int i;
        float f;
        struct ComplexStruct cs;
    } u = { .cs = s1 };
    
    /* Use all to prevent optimization */
    asm volatile("" : : "r" (&arr), "r" (&u) : "memory");
    
    return nested;
}

/* ==================== OMP_CLAUSE coverage ==================== */
#ifdef _OPENMP
NOINLINE int test_omp_clauses(int size) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel for private(size) shared(arr) reduction(+:sum) \
        schedule(dynamic) num_threads(4) if(size > 1000)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* OpenMP sections with different clauses */
    #pragma omp parallel
    {
        #pragma omp sections nowait
        {
            #pragma omp section
            {
                int local = sum;
                for (int i = 0; i < 10; i++) {
                    local += i;
                }
                #pragma omp atomic
                sum += local;
            }
            
            #pragma omp section
            {
                int local2 = sum * 2;
                #pragma omp critical
                {
                    sum = local2 / 3;
                }
            }
        }
    }
    
    /* OpenMP task with firstprivate/lastprivate */
    int task_var = 0;
    #pragma omp parallel
    #pragma omp single
    {
        for (int i = 0; i < 5; i++) {
            #pragma omp task firstprivate(i) shared(task_var)
            {
                task_var += i * 2;
            }
        }
        #pragma omp taskwait
    }
    
    return sum + task_var;
}
#endif

/* ==================== TREE_BINFO coverage (requires C++ or LTO) ==================== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual void method1() { global_volatile++; }
    virtual ~BaseClass() {}
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method1() override { global_volatile += 2; }
    int derived_data;
};

NOINLINE void test_binfo_nodes() {
    DerivedClass derived;
    BaseClass* base_ptr = &derived;
    
    /* Virtual call to generate BINFO nodes */
    base_ptr->method1();
    
    /* Type casting */
    DerivedClass* derived_ptr = dynamic_cast<DerivedClass*>(base_ptr);
    if (derived_ptr) {
        derived_ptr->derived_data = 42;
    }
}
#else
/* For C-only compilation, use LTO-friendly constructs */
struct BaseStruct {
    int type_id;
    void (*print)(struct BaseStruct*);
};

struct DerivedStruct {
    struct BaseStruct base;
    int extra_data;
};

NOINLINE void print_derived(struct BaseStruct* b) {
    struct DerivedStruct* d = (struct DerivedStruct*)b;
    global_volatile += d->extra_data;
}

NOINLINE void test_binfo_nodes(void) {
    struct DerivedStruct d = {
        .base = { .type_id = 1, .print = print_derived },
        .extra_data = 100
    };
    
    /* Call through function pointer - may generate BINFO-like nodes with LTO */
    d.base.print(&d.base);
    
    /* Complex type hierarchy simulation */
    struct BaseStruct* ptrs[3];
    struct DerivedStruct derived_array[3];
    
    for (int i = 0; i < 3; i++) {
        derived_array[i].base.type_id = i;
        derived_array[i].base.print = print_derived;
        derived_array[i].extra_data = i * 10;
        ptrs[i] = &derived_array[i].base;
    }
    
    /* Indirect calls that LTO may analyze */
    for (int i = 0; i < 3; i++) {
        ptrs[i]->print(ptrs[i]);
    }
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test 1: Identifiers and Blocks */
    test_identifiers_and_blocks(argc > 1 ? 10 : 5);
    
    /* Test 2: Vector operations */
    v4si vec_a = {1, 2, 3, 4};
    v4si vec_b = {5, 6, 7, 8};
    v4si vec_result = test_vector_operations(vec_a, vec_b);
    result += vec_result[0];
    
    /* Test 3: SSA formation */
    result += test_ssa_formation(argc + 10);
    
    /* Test 4: Aggregate initialization */
    struct NestedStruct ns = test_aggregate_init(argc, 3.14f, 2.71828);
    result += ns.inner.a + (int)ns.inner.b;
    
    /* Test 5: BINFO nodes */
    test_binfo_nodes();
    
    /* Test 6: OpenMP clauses (if supported) */
    #ifdef _OPENMP
    result += test_omp_clauses(argc * 100);
    #endif
    
    /* Final result to prevent optimization */
    asm volatile("" : "+r" (result) : : "memory");
    
    /* Print to ensure execution */
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}
