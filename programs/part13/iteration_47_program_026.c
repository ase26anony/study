/* tree_coverage_test.c - Comprehensive test for GCC tree node coverage */

/* External function to prevent optimization */
extern void use(void*);
extern int get_value(void);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_helper = 42;
volatile int volatile_tracker = 0;

/* Struct for CONSTRUCTOR nodes */
struct Point {
    double x;
    double y;
    int id;
};

struct Data {
    struct Point p;
    int values[4];
    char tag;
};

/* Array with complex initializer (TREE_VEC) */
int matrix[3][3] = {
    {1, 2, 3},
    {[1] = 5, [0] = 4, [2] = 6},
    {7, 8, 9}
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct Data process_data(int depth, int seed) {
    /* Local block with variable (BLOCK) */
    {
        int local_temp = seed * 2;
        volatile_tracker += local_temp;
    }
    
    struct Data result;
    
    /* Constructor initialization */
    result.p = (struct Point){.x = depth * 1.5, .y = depth * 2.5, .id = seed};
    
    /* Array constructor with designators */
    result.values[0] = seed;
    result.values[1] = seed + depth;
    result.values[2] = seed * depth;
    result.values[3] = seed - depth;
    result.tag = 'A' + (depth % 26);
    
    if (depth > 0) {
        struct Data nested = process_data(depth - 1, seed + 1);
        /* Combine results */
        result.p.x += nested.p.x * 0.1;
        result.values[0] += nested.values[2];
    }
    
    return result;
}

/* Function with complex control flow for SSA_NAME */
int compute_with_phi(int n, int flag) {
    int x, y, z;
    
    /* This creates phi nodes during SSA */
    if (flag > 0) {
        x = n * 2;
        y = n + 10;
    } else {
        x = n / 2;
        y = n - 5;
    }
    
    /* Another phi node */
    for (int i = 0; i < n; i++) {
        if (i % 2 == 0) {
            z = x + i;
        } else {
            z = y - i;
        }
        x = z * 2;  /* Creates additional phi in loop */
    }
    
    return x + y;
}

/* Function with goto and blocks (BLOCK nodes) */
int block_test(int val) {
    int result = 0;
    
    /* Block 1 */
    {
        int a = val * 2;
        if (a > 100) {
            goto skip_block;
        }
        result += a;
    }
    
    /* Block 2 - skipped by goto */
    {
        int hidden = 50;
        result += hidden;
    skip_block:
        result += 10;
    }
    
    /* Block 3 with nested block */
    {
        int outer = 30;
        {
            int inner = outer + 5;
            result += inner;
        }
    }
    
    return result;
}

#ifdef __cplusplus
/* C++ specific code for TREE_BINFO */
class Base {
public:
    virtual int method() { return 1; }
    virtual ~Base() {}
    int base_data;
};

class Derived : public Base {
public:
    virtual int method() override { return 2; }
    int derived_data;
};

class SecondDerived : public Derived {
public:
    virtual int method() override { return 3; }
    int second_data;
};

void test_inheritance() {
    Base* b1 = new Derived();
    Base* b2 = new SecondDerived();
    Derived* d = dynamic_cast<Derived*>(b1);
    
    volatile_tracker += b1->method();
    volatile_tracker += b2->method();
    if (d) {
        volatile_tracker += d->method();
    }
    
    delete b1;
    delete b2;
}
#endif

/* OpenMP test with multiple clauses (OMP_CLAUSE) */
void openmp_test(int size) {
    int i, j;
    int sum = 0;
    int private_var = 100;
    int shared_array[100];
    
    /* Initialize array */
    for (i = 0; i < 100; i++) {
        shared_array[i] = i;
    }
    
    /* Complex OpenMP region with multiple clauses */
    #pragma omp parallel for private(i) firstprivate(private_var) \
            shared(shared_array) reduction(+:sum) collapse(2) \
            schedule(dynamic, 4)
    for (i = 0; i < 10; i++) {
        for (j = 0; j < 10; j++) {
            int local = private_var + i * j;
            sum += shared_array[(i * 10 + j) % 100] + local;
        }
    }
    
    /* Nested OpenMP */
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task private(i)
            {
                for (i = 0; i < 5; i++) {
                    volatile_tracker += i;
                }
            }
        }
    }
    
    use(&sum);
}

/* Main function tying everything together */
int main(int argc, char** argv) {
    int i, result = 0;
    
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? get_value() : 5;
    int flag = (argc > 2) ? 1 : -1;
    
    /* Test CONSTRUCTOR and recursive function */
    struct Data d = process_data(3, iterations);
    result += d.values[0] + d.p.id;
    
    /* Test SSA_NAME generation */
    result += compute_with_phi(iterations, flag);
    
    /* Test BLOCK nodes with goto */
    result += block_test(iterations);
    
    /* Test OpenMP clauses */
    openmp_test(iterations);
    
    #ifdef __cplusplus
    /* Test C++ inheritance (TREE_BINFO) */
    test_inheritance();
    
    /* Template instantiation (TREE_VEC) */
    {
        /* Simulate template-like behavior in C */
        int template_vec[3][4] = {
            {1, 2, 3, 4},
            {5, 6, 7, 8},
            {9, 10, 11, 12}
        };
        result += template_vec[1][2];
    }
    #endif
    
    /* Use various identifiers (IDENTIFIER_NODE) */
    global_counter++;
    static_helper--;
    result += global_counter + static_helper + volatile_tracker;
    
    /* Use matrix (TREE_VEC) */
    for (i = 0; i < 3; i++) {
        result += matrix[i][i];
    }
    
    /* Final result to ensure all code is live */
    return result % 256;
}
