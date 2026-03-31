/* test_tree_kind_coverage.c - Coverage test for GCC's get_kind function */
/* Compile with: gcc -O2 -fopenmp -std=gnu99 test_tree_kind_coverage.c -o test_program */
/* For C++ specific nodes: g++ -O2 -fopenmp -std=gnu++11 test_tree_kind_coverage.c -o test_cpp_program */

#include <stdio.h>
#include <stdlib.h>

/* Helper to prevent optimization from removing code */
static void use(void *p) {
    asm volatile("" : : "r"(p) : "memory");
}

/* ========== IDENTIFIER_NODE coverage ========== */
/* Variable and function names create IDENTIFIER_NODE */
int global_identifier_coverage;
static int static_identifier_coverage;

void function_with_identifiers(void) {
    int local_identifier = 42;
    char *string_identifier = "test";
    use(&local_identifier);
    use(&string_identifier);
}

/* ========== SSA_NAME coverage ========== */
/* Complex arithmetic and loops force SSA form */
int ssa_name_coverage(int n) {
    int a = 0, b = 1, c = 2;
    
    /* Loop with phi nodes */
    for (int i = 0; i < n; ++i) {
        a = a + b * c;
        b = b + i;
        if (i % 2 == 0) {
            c = c - 1;
        } else {
            c = c + 2;
        }
    }
    
    /* Nested loops for more SSA complexity */
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            a = a + i * j;
        }
    }
    
    return a + b + c;
}

/* ========== BLOCK coverage ========== */
/* Nested blocks with local variables */
void block_coverage(void) {
    int outer = 1;
    use(&outer);
    
    {
        /* Inner block 1 */
        int inner1 = 2;
        char inner1_char = 'a';
        use(&inner1);
        use(&inner1_char);
        
        {
            /* Inner block 2 */
            int inner2 = 3;
            double inner2_double = 3.14;
            use(&inner2);
            use(&inner2_double);
            
            {
                /* Inner block 3 */
                int inner3 = 4;
                int *inner3_ptr = &inner3;
                use(inner3_ptr);
            }
        }
    }
    
    /* Another block with control flow */
    {
        int x = 5;
        if (x > 0) {
            int y = x * 2;
            use(&y);
        }
    }
}

/* ========== CONSTRUCTOR coverage ========== */
/* Aggregate initializers */
struct constructor_struct {
    int a;
    double b;
    char c;
    int *d;
};

int constructor_coverage(void) {
    /* Array constructor */
    int array_constructor[5] = {10, 20, 30, 40, 50};
    
    /* Struct constructor */
    struct constructor_struct s1 = {1, 2.0, 'x', &array_constructor[0]};
    
    /* Designated initializer */
    struct constructor_struct s2 = {
        .a = 42,
        .b = 3.14159,
        .c = 'z',
        .d = &array_constructor[1]
    };
    
    /* Nested struct with constructor */
    struct nested {
        struct constructor_struct inner;
        int extra;
    } n1 = {{5, 6.0, 'y', NULL}, 100};
    
    use(array_constructor);
    use(&s1);
    use(&s2);
    use(&n1);
    
    return array_constructor[0] + s1.a + s2.a + n1.extra;
}

/* ========== OMP_CLAUSE coverage ========== */
#ifdef _OPENMP
void omp_clause_coverage(int size) {
    int i;
    int *data = malloc(size * sizeof(int));
    
    if (!data) return;
    
    /* Various OpenMP clauses */
    #pragma omp parallel for private(i) shared(data) schedule(static)
    for (i = 0; i < size; i++) {
        data[i] = i * 2;
    }
    
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (i = 0; i < size; i++) {
        sum += data[i];
    }
    
    /* More complex clause combination */
    #pragma omp parallel private(i) firstprivate(size)
    {
        #pragma omp for nowait
        for (i = 0; i < size; i++) {
            data[i] += 1;
        }
    }
    
    use(&sum);
    free(data);
}
#else
void omp_clause_coverage(int size) {
    /* Dummy implementation when OpenMP not available */
    (void)size;
}
#endif

/* ========== TREE_VEC coverage ========== */
/* Using GCC statement expressions and vector extensions */
#ifdef __GNUC__
int tree_vec_coverage(void) {
    /* Statement expression with multiple elements */
    int result = ({
        int a = 5;
        int b = 10;
        int c = a * b;
        c + 2;
    });
    
    /* Using vector extension (creates VEC nodes) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si v1 = {1, 2, 3, 4};
    v4si v2 = {5, 6, 7, 8};
    v4si v3 = v1 + v2;
    
    /* Compound literal that might create TREE_VEC */
    struct point { int x, y; };
    struct point p = (struct point){ .x = result, .y = 20 };
    
    use(&v3);
    use(&p);
    
    return result + v3[0] + p.x;
}
#else
int tree_vec_coverage(void) {
    return 42;
}
#endif

/* ========== C++ specific code for TREE_BINFO ========== */
#ifdef __cplusplus

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

/* Multiple inheritance to ensure BINFO nodes */
class DerivedClass : public BaseClass1, public BaseClass2 {
public:
    virtual ~DerivedClass() {}
    virtual void method1() override { base_data1 = 1; }
    virtual void method2() override { base_data2 = 2.0; }
    
    void derived_method() {
        method1();
        method2();
    }
    
    int derived_data;
};

/* Template class for additional complexity */
template<typename T>
class TemplateBase {
public:
    virtual T get_value() = 0;
    T template_data;
};

class ConcreteClass : public TemplateBase<int> {
public:
    virtual int get_value() override { return template_data; }
};

void cpp_binfo_coverage(void) {
    DerivedClass d;
    d.derived_method();
    
    ConcreteClass c;
    c.template_data = 100;
    int val = c.get_value();
    
    use(&d);
    use(&c);
    use(&val);
    
    /* Dynamic cast for RTTI (might involve BINFO) */
    BaseClass1* b1 = &d;
    BaseClass2* b2 = dynamic_cast<BaseClass2*>(b1);
    if (b2) {
        b2->method2();
    }
}

#else
/* C version - dummy implementation */
void cpp_binfo_coverage(void) {
    /* In C mode, we can't create BINFO nodes */
    printf("C++ mode required for BINFO coverage\n");
}
#endif

/* ========== Main driver ========== */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Ensure all constructs are used */
    function_with_identifiers();
    
    result += ssa_name_coverage(100);
    
    block_coverage();
    
    result += constructor_coverage();
    
    omp_clause_coverage(1000);
    
    result += tree_vec_coverage();
    
    cpp_binfo_coverage();
    
    /* Use argc/argv to prevent dead code elimination */
    if (argc > 1) {
        result += atoi(argv[1]);
    }
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}
