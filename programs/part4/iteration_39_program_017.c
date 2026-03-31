/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

/* Prevent inlining to ensure tree nodes are fully built */
#define NOINLINE __attribute__((noinline))

/* Generate many identifiers for IDENTIFIER_NODE */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(identifier_, n)

/* Use volatile to prevent optimization */
volatile int global_trigger = 0;

/* ========== TREE_VEC (Vector Types) ========== */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

NOINLINE void test_tree_vec(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = a + b;  /* Generates TREE_VEC nodes for vector operations */
    v4si d = a * b;
    v4si e = c + d;
    
    /* Use memory barrier to prevent optimization */
    asm volatile("" : : : "memory");
    
    /* Store result to volatile to ensure computation isn't optimized away */
    global_trigger += e[0];
}

/* ========== SSA_NAME ========== */
NOINLINE int test_ssa_name(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex loop with multiple assignments to create SSA form */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = i * 2;
        } else {
            x = i * 3;
        }
        
        if (i % 3 == 0) {
            y = x + i;
        } else {
            y = x - i;
        }
        
        z += y;  /* This creates phi nodes in SSA form */
    }
    
    /* Memory barrier */
    asm volatile("" : : : "memory");
    
    return z;
}

/* ========== BLOCK ========== */
NOINLINE void test_blocks(void) {
    /* Outer block */
    int outer = 10;
    
    {
        /* Nested block 1 */
        int block1_var = 20;
        
        {
            /* Nested block 2 */
            int block2_var = 30;
            global_trigger += block2_var;
            
            {
                /* Deeply nested block 3 */
                int block3_var = 40;
                global_trigger += block3_var;
            }
        }
        
        /* Another block in same scope */
        if (outer > 0) {
            int if_block_var = 50;
            global_trigger += if_block_var;
        } else {
            int else_block_var = 60;
            global_trigger += else_block_var;
        }
        
        /* Loop with block */
        for (int i = 0; i < 5; i++) {
            int loop_block_var = i * 10;
            global_trigger += loop_block_var;
        }
    }
}

/* ========== CONSTRUCTOR ========== */
struct ComplexStruct {
    int a, b, c;
    float x, y, z;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

NOINLINE int test_constructor(void) {
    /* Non-constant initializer */
    int non_const = global_trigger + 1;
    
    /* Aggregate initialization with non-constant expressions */
    struct ComplexStruct s1 = {
        .a = non_const,
        .b = test_ssa_name(3),
        .c = 42,
        .x = 1.5f,
        .y = 2.5f,
        .z = 3.5f
    };
    
    /* Array with non-constant initializers */
    int arr[4] = {
        non_const,
        s1.a + s1.b,
        test_ssa_name(2),
        99
    };
    
    /* Nested struct initialization */
    struct NestedStruct ns = {
        .inner = {
            .a = arr[0],
            .b = arr[1],
            .c = arr[2],
            .x = 4.0f,
            .y = 5.0f,
            .z = 6.0f
        },
        .extra = 1000
    };
    
    /* Designated initializer for array */
    int sparse[10] = { [2] = non_const, [5] = s1.c, [9] = 999 };
    
    asm volatile("" : : : "memory");
    
    return s1.a + arr[0] + ns.inner.b + sparse[2];
}

/* ========== OMP_CLAUSE ========== */
#ifdef _OPENMP
NOINLINE int test_omp_clauses(void) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* OpenMP with multiple clauses to generate OMP_CLAUSE nodes */
    #pragma omp parallel for private(i) shared(arr) reduction(+:sum) schedule(dynamic) num_threads(4)
    for (int i = 0; i < 100; i++) {
        sum += arr[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel sections private(i) firstprivate(sum)
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 0; i < 50; i++) {
                section_sum += i;
            }
            global_trigger += section_sum;
        }
        
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 50; i < 100; i++) {
                section_sum += i;
            }
            global_trigger += section_sum;
        }
    }
    
    /* Task with clauses */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task depend(out: arr[0]) if(1) final(0) mergeable
            {
                arr[0] = sum;
            }
        }
    }
    
    asm volatile("" : : : "memory");
    
    return sum;
}
#endif

/* ========== C++ for TREE_BINFO ========== */
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

NOINLINE int test_binfo(void) {
    DerivedClass d;
    BaseClass* b = &d;  /* This should generate BINFO nodes for inheritance */
    
    /* Use virtual call */
    int result = b->method();
    
    /* Access through reference */
    BaseClass& br = d;
    result += br.method();
    
    /* Array of base pointers */
    BaseClass* arr[3];
    arr[0] = &d;
    
    asm volatile("" : : : "memory");
    
    return result + d.base_data + d.derived_data;
}
#endif

/* ========== Many IDENTIFIER_NODE ========== */
NOINLINE void test_identifiers(void) {
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
    
    /* Use them in expressions */
    int result = 
        MAKE_ID(0) + MAKE_ID(1) * MAKE_ID(2) - 
        MAKE_ID(3) / MAKE_ID(4) + 
        MAKE_ID(5) | MAKE_ID(6) & 
        MAKE_ID(7) ^ MAKE_ID(8) % 
        MAKE_ID(9);
    
    /* Function calls with different names */
    extern int func_a(void);
    extern int func_b(void);
    extern int func_c(void);
    extern int func_d(void);
    extern int func_e(void);
    
    /* Complex expression with many identifiers */
    global_trigger = 
        MAKE_ID(0) + MAKE_ID(1) + 
        MAKE_ID(2) + MAKE_ID(3) + 
        MAKE_ID(4) + MAKE_ID(5) + 
        MAKE_ID(6) + MAKE_ID(7) + 
        MAKE_ID(8) + MAKE_ID(9) + 
        result;
    
    asm volatile("" : : : "memory");
}

/* ========== Main function ========== */
int main(void) {
    int total = 0;
    
    /* Test all tree node types */
    test_tree_vec();
    total += test_ssa_name(100);
    test_blocks();
    total += test_constructor();
    
    #ifdef _OPENMP
    total += test_omp_clauses();
    #endif
    
    #ifdef __cplusplus
    total += test_binfo();
    #endif
    
    test_identifiers();
    
    /* Use results to prevent optimization */
    asm volatile("" : : "r"(total), "r"(global_trigger) : "memory");
    
    return total + global_trigger;
}

/* External function declarations for identifier test */
int func_a(void) { return 1; }
int func_b(void) { return 2; }
int func_c(void) { return 3; }
int func_d(void) { return 4; }
int func_e(void) { return 5; }
