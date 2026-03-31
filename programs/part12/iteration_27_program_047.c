/* test_tree_kind_coverage.c - Coverage for GCC's tree.cc get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind_coverage.c -o test */
/* For C++ nodes: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind_coverage.c -o test */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ========== IDENTIFIER_NODE ========== */
/* Variable and function names create IDENTIFIER_NODE */
int some_unique_identifier_1;
void function_with_identifier(void) {
    int local_identifier = 42;
    use(&local_identifier);
}

/* ========== SSA_NAME ========== */
/* Complex arithmetic with loops forces SSA form */
int ssa_name_generator(int n) {
    int a = 0, b = 1, c;
    for (int i = 0; i < n; ++i) {
        c = a + b;          /* Creates SSA_NAME for phi nodes */
        a = b;
        b = c;
        if (c % 2 == 0) {
            a = a * 2;      /* More SSA opportunities */
        }
    }
    return b;
}

/* ========== BLOCK ========== */
/* Nested blocks with local variables */
void block_generator(void) {
    int outer = 0;
    use(&outer);
    
    { /* BLOCK 1 */
        int inner1 = 1;
        use(&inner1);
        
        { /* BLOCK 2 */
            int inner2 = 2;
            use(&inner2);
            
            { /* BLOCK 3 */
                int inner3 = 3;
                use(&inner3);
            }
        }
    }
}

/* ========== CONSTRUCTOR ========== */
/* Aggregate initializers */
struct my_struct {
    int a;
    float b;
    char c;
};

void constructor_generator(void) {
    /* Array constructor */
    int arr[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct my_struct s1 = {.a = 1, .b = 2.0f, .c = 'x'};
    
    /* Nested struct constructor */
    struct nested {
        struct my_struct inner;
        int extra;
    } n1 = {{5, 6.0f, 'y'}, 100};
    
    use(arr);
    use(&s1);
    use(&n1);
}

/* ========== TREE_VEC ========== */
/* Using GCC statement expressions (GNU extension) */
#ifdef __GNUC__
void tree_vec_generator(void) {
    /* Statement expression creates TREE_VEC */
    int result = ({
        int x = 5;
        int y = 10;
        x + y;
    });
    
    /* Another TREE_VEC example with multiple statements */
    int arr[3] = (int[3]){ [0] = 1, [1] = 2, [2] = 3 };
    
    use(&result);
    use(arr);
}
#endif

/* ========== OMP_CLAUSE ========== */
#ifdef _OPENMP
void omp_clause_generator(int n) {
    int i;
    int sum = 0;
    int private_var = 0;
    
    #pragma omp parallel for private(i) reduction(+:sum) firstprivate(private_var)
    for (i = 0; i < n; i++) {
        sum += i;
        #pragma omp atomic
        private_var++;
    }
    
    #pragma omp parallel sections private(i)
    {
        #pragma omp section
        {
            i = 1;
        }
        #pragma omp section
        {
            i = 2;
        }
    }
    
    use(&sum);
    use(&private_var);
}
#endif

/* ========== C++ Specific: TREE_BINFO ========== */
#ifdef __cplusplus
class BaseClass {
public:
    virtual ~BaseClass() {}
    virtual void method() = 0;
    int base_data;
};

class DerivedClass : public BaseClass {
public:
    virtual void method() override {
        base_data = 42;
    }
    int derived_data;
};

class MultipleBase1 {
public:
    virtual ~MultipleBase1() {}
    int data1;
};

class MultipleBase2 {
public:
    virtual ~MultipleBase2() {}
    int data2;
};

class MultipleDerived : public MultipleBase1, public MultipleBase2 {
public:
    int derived_data;
};

void tree_binfo_generator(void) {
    DerivedClass d;
    d.method();
    
    MultipleDerived md;
    md.data1 = 1;
    md.data2 = 2;
    md.derived_data = 3;
    
    BaseClass* bp = &d;
    bp->method();
    
    use(&d);
    use(&md);
    use(&bp);
}
#endif

/* ========== Main driver ========== */
int main(int argc, char** argv) {
    int n = 100;
    
    /* Ensure all generators are called */
    function_with_identifier();
    
    int fib = ssa_name_generator(n);
    printf("SSA test result: %d\n", fib);
    
    block_generator();
    constructor_generator();
    
    #ifdef __GNUC__
    tree_vec_generator();
    #endif
    
    #ifdef _OPENMP
    omp_clause_generator(n);
    printf("OpenMP enabled\n");
    #endif
    
    #ifdef __cplusplus
    tree_binfo_generator();
    printf("C++ mode enabled\n");
    #endif
    
    /* Complex control flow to engage more middle-end passes */
    int result = 0;
    for (int i = 0; i < 1000; i++) {
        if (i % 3 == 0) {
            result += i * 2;
        } else if (i % 3 == 1) {
            result -= i;
        } else {
            result *= (i % 10) + 1;
        }
        
        /* Switch statement for more tree variety */
        switch (i % 5) {
            case 0: result += 1; break;
            case 1: result -= 2; break;
            case 2: result *= 3; break;
            case 3: result /= 4; break;
            case 4: result %= 5; break;
        }
    }
    
    printf("Final result: %d\n", result);
    return 0;
}
