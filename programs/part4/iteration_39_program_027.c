/* test_tree_coverage.c - Comprehensive test to trigger all tree node classification cases */

#include <stdio.h>
#include <stdlib.h>

/* For OpenMP support */
#ifdef _OPENMP
#include <omp.h>
#endif

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Generate multiple identifiers */
#define GEN_IDENTIFIERS(n) \
    int MAKE_ID(n) = n; \
    void MAKE_FUNC(n)(void) { volatile int x = MAKE_ID(n); (void)x; }

/* Instantiate identifiers */
GEN_IDENTIFIERS(1)
GEN_IDENTIFIERS(2)
GEN_IDENTIFIERS(3)
GEN_IDENTIFIERS(4)
GEN_IDENTIFIERS(5)
GEN_IDENTIFIERS(6)
GEN_IDENTIFIERS(7)
GEN_IDENTIFIERS(8)
GEN_IDENTIFIERS(9)
GEN_IDENTIFIERS(10)

/* More complex identifier usage */
int global_counter = 0;
static int static_accumulator = 0;
extern int external_reference;

/* ========== TREE_VEC generation ========== */
/* Use GCC vector extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef short v8hi __attribute__((vector_size(16)));

/* Function using vector operations */
void test_vector_operations(void) {
    volatile v4si a = {1, 2, 3, 4};
    volatile v4si b = {5, 6, 7, 8};
    v4si c, d, e;
    
    /* Various vector operations */
    c = a + b;
    d = a * b;
    e = c - d;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(c), "r"(d), "r"(e) : "memory");
}

/* ========== SSA_NAME generation ========== */
/* Complex function with loops and branches to generate SSA form */
int test_ssa_generation(int n) {
    int i, x, y, z;
    volatile int result = 0;
    
    /* Loop with multiple assignments to create phi nodes */
    for (i = 0; i < n; i++) {
        if (i % 2 == 0) {
            x = i * 2;
            y = x + 1;
        } else {
            x = i / 2;
            y = x - 1;
        }
        
        /* Another level of conditional assignment */
        if (i % 3 == 0) {
            z = y * 3;
        } else if (i % 3 == 1) {
            z = y + 5;
        } else {
            z = y - 2;
        }
        
        result += z;
        
        /* Memory barrier to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* More complex control flow */
    int a = 0, b = 0;
    for (int j = 0; j < n; j++) {
        switch (j % 4) {
            case 0: a = j * 2; break;
            case 1: a = j + 10; break;
            case 2: a = j - 5; break;
            case 3: a = j / 2; break;
        }
        b += a;
    }
    
    return result + b;
}

/* ========== BLOCK generation ========== */
/* Function with deeply nested blocks */
void test_nested_blocks(int depth) {
    /* Outer block with variables */
    int outer_var = 0;
    {
        /* Level 1 nested block */
        int level1 = outer_var + 1;
        volatile int block1_var = level1 * 2;
        
        {
            /* Level 2 nested block */
            int level2 = block1_var;
            {
                /* Level 3 nested block - inside loop */
                for (int i = 0; i < depth; i++) {
                    int loop_var = i;
                    {
                        /* Innermost block */
                        int inner = loop_var * 3;
                        volatile int innermost = inner + level2;
                        (void)innermost;
                    }
                }
            }
            
            /* Another block after the loop */
            int post_loop = 42;
            {
                volatile int final_val = post_loop;
                (void)final_val;
            }
        }
    }
    
    /* Multiple independent blocks */
    if (depth > 0) {
        int if_block_var = depth * 10;
        volatile int if_result = if_block_var;
        (void)if_result;
    } else {
        int else_block_var = 100;
        volatile int else_result = else_block_var;
        (void)else_result;
    }
    
    /* Block in switch statement */
    switch (depth % 3) {
        case 0: {
            int case0_var = 0;
            volatile int case0_result = case0_var;
            (void)case0_result;
            break;
        }
        case 1: {
            int case1_var = 1;
            volatile int case1_result = case1_var;
            (void)case1_result;
            break;
        }
        case 2: {
            int case2_var = 2;
            volatile int case2_result = case2_var;
            (void)case2_result;
            break;
        }
    }
}

/* ========== CONSTRUCTOR generation ========== */
/* Struct for constructor testing */
struct ComplexStruct {
    int a;
    float b;
    double c;
    char d;
    short e;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
};

/* Function using non-constant aggregate initializers */
struct ComplexStruct test_constructor_nodes(int x, float y) {
    /* Use function calls in initializers */
    extern int get_random_value(void);
    extern float compute_float(void);
    
    /* Non-constant struct initialization */
    struct ComplexStruct s1 = {
        .a = get_random_value() + x,
        .b = y * compute_float(),
        .c = (double)x / (y + 1.0f),
        .d = (char)(x % 256),
        .e = (short)(x * 2)
    };
    
    /* Array with non-constant initializers */
    int dynamic_array[4] = {
        x,
        x * 2,
        x + get_random_value(),
        get_random_value() % 100
    };
    
    /* Nested struct initialization */
    struct NestedStruct ns = {
        .inner = {
            .a = dynamic_array[0],
            .b = y,
            .c = s1.c * 2.0,
            .d = s1.d + 1,
            .e = (short)(s1.e / 2)
        },
        .extra = x * 3
    };
    
    /* Designated initializer with mixed expressions */
    struct ComplexStruct s2 = {
        get_random_value(),              /* a */
        y + 1.5f,                        /* b */
        (double)dynamic_array[2] / 3.0,  /* c */
        'A' + (x % 26),                  /* d */
        (short)(ns.extra % 32767)        /* e */
    };
    
    /* Prevent optimization */
    asm volatile("" : : "r"(s1), "r"(s2), "r"(ns), "r"(dynamic_array) : "memory");
    
    return s2;
}

/* ========== OMP_CLAUSE generation ========== */
/* Function with various OpenMP pragmas and clauses */
void test_omp_clauses(int size) {
#ifdef _OPENMP
    int i;
    int sum = 0;
    int* array = (int*)malloc(size * sizeof(int));
    
    if (!array) return;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        array[i] = i;
    }
    
    /* OpenMP parallel region with multiple clauses */
    #pragma omp parallel default(none) shared(array, size) private(i) reduction(+:sum) num_threads(4)
    {
        #pragma omp for schedule(dynamic, 4) nowait
        for (i = 0; i < size; i++) {
            sum += array[i];
        }
        
        /* Nested parallel section */
        #pragma omp sections private(i)
        {
            #pragma omp section
            {
                int local_sum = 0;
                for (i = 0; i < size/2; i++) {
                    local_sum += array[i];
                }
                #pragma omp atomic
                sum += local_sum;
            }
            
            #pragma omp section
            {
                int local_prod = 1;
                for (i = size/2; i < size; i++) {
                    local_prod *= (array[i] + 1);
                }
                #pragma omp critical
                {
                    sum += local_prod % 1000;
                }
            }
        }
        
        /* Task with dependencies */
        #pragma omp task depend(inout: sum) if(size > 1000)
        {
            sum = sum % 1000000;
        }
        
        #pragma omp taskwait
        
        /* Barrier with explicit flush */
        #pragma omp barrier
        #pragma omp flush(sum)
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    int min_val = 0;
    
    #pragma omp parallel for reduction(max:max_val) reduction(min:min_val) \
        collapse(2) ordered
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            int val = x * 10 + y;
            #pragma omp ordered
            {
                if (val > max_val) max_val = val;
                if (val < min_val || min_val == 0) min_val = val;
            }
        }
    }
    
    /* Simd pragma with clauses */
    int simd_array[100];
    
    #pragma omp simd aligned(simd_array: 16) linear(i:1) safelen(8)
    for (i = 0; i < 100; i++) {
        simd_array[i] = i * 2;
    }
    
    free(array);
    
    /* Prevent optimization */
    volatile int dummy = sum + max_val + min_val + simd_array[0];
    (void)dummy;
#endif
}

/* ========== TREE_BINFO generation (using LTO) ========== */
/* Structure with complex type relationships for LTO */
struct TypeInfo {
    int type_id;
    char* type_name;
    struct TypeInfo* base_type;
};

struct ComplexType {
    struct TypeInfo info;
    int data[10];
    struct ComplexType* next;
};

/* Function that uses type information across compilation units */
struct ComplexType* create_complex_type(int id) {
    static struct TypeInfo base_info = {0, "BaseType", NULL};
    
    struct ComplexType* ct = (struct ComplexType*)malloc(sizeof(struct ComplexType));
    if (!ct) return NULL;
    
    ct->info.type_id = id;
    ct->info.type_name = "ComplexType";
    ct->info.base_type = &base_info;
    ct->next = NULL;
    
    for (int i = 0; i < 10; i++) {
        ct->data[i] = id * i;
    }
    
    return ct;
}

/* ========== Main orchestrator ========== */
int main(int argc, char** argv) {
    int result = 0;
    volatile int prevent_opt = 0;
    
    /* Force use of identifiers */
    func_1(); func_2(); func_3();
    prevent_opt += var_1 + var_2 + var_3;
    
    /* Test vector operations */
    test_vector_operations();
    
    /* Test SSA generation with complex control flow */
    result += test_ssa_generation(100);
    
    /* Test nested blocks */
    test_nested_blocks(10);
    
    /* Test constructor nodes */
    struct ComplexStruct cs = test_constructor_nodes(42, 3.14f);
    result += cs.a + (int)cs.b;
    
    /* Test OpenMP clauses */
    test_omp_clauses(1000);
    
    /* Test type information for LTO/BINFO */
    struct ComplexType* ct = create_complex_type(5);
    if (ct) {
        for (int i = 0; i < 10; i++) {
            result += ct->data[i];
        }
        free(ct);
    }
    
    /* Additional complex expression mixing everything */
    {
        /* Block with vector operations */
        v4si v1 = {1, 2, 3, 4};
        v4si v2 = {5, 6, 7, 8};
        v4si v3 = v1 * v2;
        volatile int vsum = v3[0] + v3[1] + v3[2] + v3[3];
        
        /* SSA variable in complex expression */
        int ssa_var = 0;
        for (int i = 0; i < 20; i++) {
            if (i % 2) {
                ssa_var += i * 2;
            } else {
                ssa_var -= i / 2;
            }
        }
        
        /* Constructor in local scope */
        int local_array[] = {ssa_var, vsum, result, argc};
        struct { int a; int b; } local_struct = {local_array[0], local_array[1]};
        
        result += local_struct.a + local_struct.b + ssa_var + vsum;
    }
    
    printf("Final result: %d\n", result);
    
    /* Ensure all code paths are used */
    if (argc > 1) {
        /* Use more identifiers */
        func_4(); func_5(); func_6();
        prevent_opt += var_4 + var_5 + var_6;
        
        /* More OpenMP */
        test_omp_clauses(100);
        
        /* More constructors */
        struct ComplexStruct cs2 = test_constructor_nodes(atoi(argv[1]), 2.718f);
        result += cs2.a;
    }
    
    return result % 256;
}
