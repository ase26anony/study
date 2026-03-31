/* test_tree_kind.c - Comprehensive test for GCC tree kind coverage */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind.c -o test_tree_kind */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization from removing code */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ==================== IDENTIFIER_NODE ==================== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier_node_example = 42;
void function_identifier_example(void) {
    int local_identifier = global_identifier_node_example;
    use(&local_identifier);
}

/* ==================== SSA_NAME ==================== */
/* Complex arithmetic with loops forces SSA form */
int ssa_name_generator(int n) {
    int a = 0, b = 1, c;
    
    /* This loop creates many SSA_NAME nodes during optimization */
    for (int i = 0; i < n; ++i) {
        c = a + b;      /* Creates SSA_NAME for arithmetic */
        a = b;          /* Creates SSA_NAME for assignment */
        b = c;          /* Creates SSA_NAME for assignment */
        
        /* Conditional creates phi nodes (SSA) */
        if (c % 2 == 0) {
            a = c * 2;
        } else {
            b = c / 2;
        }
    }
    
    /* Multiple uses create more SSA opportunities */
    int result = a + b;
    result = result * result - 1;
    return result;
}

/* ==================== BLOCK ==================== */
/* Nested blocks with local variables */
void block_node_example(void) {
    int outer = 0;
    use(&outer);
    
    {  /* BLOCK node */
        int inner_block_var = 5;
        use(&inner_block_var);
        
        {  /* Another nested BLOCK */
            int deeper = inner_block_var * 2;
            use(&deeper);
            
            {  /* Even deeper BLOCK */
                int deepest = deeper + 1;
                use(&deepest);
            }
        }
    }
    
    /* Switch statement creates multiple blocks */
    switch (outer) {
        case 0: {
            int case_var = 10;
            use(&case_var);
            break;
        }
        case 1: {
            int another_case_var = 20;
            use(&another_case_var);
            break;
        }
        default: {
            int default_var = 30;
            use(&default_var);
        }
    }
}

/* ==================== CONSTRUCTOR ==================== */
/* Aggregate initializers create CONSTRUCTOR nodes */
struct constructor_example {
    int a;
    double b;
    char c[4];
};

void constructor_node_example(void) {
    /* Array constructor */
    int array_constructor[5] = {1, 2, 3, 4, 5};
    use(array_constructor);
    
    /* Struct constructor (C99 designated initializer) */
    struct constructor_example s1 = {.a = 1, .b = 2.0, .c = {'a', 'b', 'c', '\0'}};
    use(&s1);
    
    /* Nested struct constructor */
    struct nested {
        struct constructor_example inner;
        int x;
    } n1 = {{2, 3.0, {'d', 'e', 'f', '\0'}}, 42};
    use(&n1);
    
    /* Zero initializer (also creates constructor) */
    struct constructor_example zero = {0};
    use(&zero);
}

/* ==================== TREE_VEC ==================== */
/* GCC extensions that create TREE_VEC nodes */
#ifdef __GNUC__
void tree_vec_example(void) {
    /* Using statement expression (GCC extension) */
    int vec_result = ({
        int x = 5;
        int y = 10;
        int z;
        
        /* Multiple statements in expression */
        for (int i = 0; i < 3; i++) {
            x += i;
        }
        
        y = x * 2;
        z = y + 1;
        z;  /* Result of expression */
    });
    
    use(&vec_result);
    
    /* Typeof with multiple elements (can create TREE_VEC) */
    typeof(int[3]) array_type;
    use(&array_type);
}
#endif

/* ==================== OMP_CLAUSE ==================== */
/* OpenMP pragmas create OMP_CLAUSE nodes */
#ifdef _OPENMP
void omp_clause_example(void) {
    int i;
    int sum = 0;
    int data[100];
    
    /* Initialize data */
    for (i = 0; i < 100; i++) {
        data[i] = i;
    }
    
    /* OpenMP parallel for with multiple clauses */
    #pragma omp parallel for private(i) reduction(+:sum) schedule(static, 10)
    for (i = 0; i < 100; i++) {
        sum += data[i];
    }
    
    /* Another OpenMP construct with different clauses */
    #pragma omp parallel num_threads(4) default(none) shared(sum, data)
    {
        #pragma omp for nowait
        for (int j = 0; j < 50; j++) {
            data[j] *= 2;
        }
    }
    
    use(&sum);
    use(data);
}
#endif

/* ==================== C++ Specific: TREE_BINFO ==================== */
#ifdef __cplusplus
/* TREE_BINFO nodes are created for C++ class hierarchies */

class BaseClass1 {
public:
    virtual ~BaseClass1() {}
    virtual void method1() = 0;
    int base_data1;
};

class BaseClass2 {
public:
    virtual ~BaseClass2() {}
    virtual void method2() = 0;
    double base_data2;
};

/* Multiple inheritance creates BINFO nodes */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual ~DerivedClass() {}
    virtual void method1() override { base_data1 = 42; }
    virtual void method2() override { base_data2 = 3.14; }
    
    void derived_method() {
        /* Virtual calls through base pointers */
        BaseClass1* b1 = this;
        BaseClass2* b2 = this;
        b1->method1();
        b2->method2();
    }
    
    int derived_data;
};

void tree_binfo_example(void) {
    DerivedClass d;
    d.derived_method();
    
    /* Casting through hierarchy */
    BaseClass1* b1_ptr = &d;
    BaseClass2* b2_ptr = &d;
    
    /* Dynamic cast (requires RTTI, creates BINFO lookups) */
    DerivedClass* d1 = dynamic_cast<DerivedClass*>(b1_ptr);
    DerivedClass* d2 = dynamic_cast<DerivedClass*>(b2_ptr);
    
    use(&d);
    use(b1_ptr);
    use(b2_ptr);
    use(d1);
    use(d2);
}
#endif

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    printf("Testing GCC tree kind coverage...\n");
    
    /* Trigger all examples */
    function_identifier_example();
    
    int ssa_result = ssa_name_generator(20);
    printf("SSA result: %d\n", ssa_result);
    
    block_node_example();
    constructor_node_example();
    
    #ifdef __GNUC__
    tree_vec_example();
    #endif
    
    #ifdef _OPENMP
    omp_clause_example();
    #endif
    
    #ifdef __cplusplus
    tree_binfo_example();
    #endif
    
    /* Complex control flow to keep optimizer interested */
    volatile int flag = rand() % 100;
    if (flag > 50) {
        printf("Flag is high: %d\n", flag);
    } else {
        printf("Flag is low: %d\n", flag);
    }
    
    return 0;
}
