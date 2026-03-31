/* Test program to trigger tree_kind dispatch for uncovered TREE_CODE cases */
#ifdef __cplusplus
#include <vector>
#include <iostream>
extern "C" {
#endif

/* External function to prevent optimization */
extern void use(void*);

/* Global identifiers (IDENTIFIER_NODE) */
int global_counter = 0;
static int static_accumulator = 0;
extern int external_reference;

/* Struct for CONSTRUCTOR nodes */
struct Data {
    int values[4];
    struct Data* next;
};

/* Recursive function returning struct (CONSTRUCTOR) */
struct Data build_data(int depth, int base) {
    struct Data d;
    
    /* Array initializer with designator (TREE_VEC in C mode) */
    int temp[4] = {[0] = base, [2] = base * 2, [3] = base * 3};
    for (int i = 0; i < 4; i++) {
        d.values[i] = temp[i] + i;
    }
    
    if (depth > 0) {
        d.next = &d; /* Self-reference for complexity */
        /* Recursive call */
        struct Data child = build_data(depth - 1, base + 1);
        d.values[1] += child.values[0];
    } else {
        d.next = 0;
    }
    
    return d; /* Returns CONSTRUCTOR node */
}

#ifdef __cplusplus
/* C++ classes for TREE_BINFO */
class Base {
public:
    virtual int method() { return 42; }
    virtual ~Base() {}
    int base_data;
};

class Derived : public Base {
public:
    virtual int method() override { return base_data * 2; }
    int derived_data;
};

template<typename T>
class TemplateClass {
    T data;
public:
    TemplateClass(T val) : data(val) {}
    T get() { return data; }
};
#endif

int main(int argc, char** argv) {
    /* Use argc to prevent optimization */
    int iterations = (argc > 1) ? 10 : 5;
    volatile int vol = argc; /* volatile prevents constant folding */
    
    /* BLOCK nodes with goto */
    {
        int block_local = 100;
        goto skip_init;
        int unused = 50; /* This won't be executed */
        skip_init:
        block_local += vol;
        use(&block_local);
    }
    
    /* Another complex block with label */
    {
        int x = 0;
        if (vol > 2) {
            goto middle;
        }
        x = 10;
        middle:
        x += 20;
        
        /* Nested block */
        {
            int inner = x * 2;
            use(&inner);
        }
    }
    
    /* CONSTRUCTOR nodes - struct initialization */
    struct Data dataset = {.values = {1, 2, 3, 4}, .next = 0};
    
    /* Array constructor with designators */
    int arr[10] = {[0] = 1, [5] = vol, [9] = iterations};
    
    /* Call recursive function */
    struct Data recursive_data = build_data(3, vol);
    use(&recursive_data);
    
    /* SSA_NAME generation - complex conditional assignments */
    int ssa_var = 0;
    for (int i = 0; i < iterations; i++) {
        if (i % 2 == 0) {
            ssa_var += i * 2;
        } else {
            ssa_var += i * 3;
        }
        
        /* Another SSA opportunity */
        int temp;
        if (ssa_var > 100) {
            temp = ssa_var / 2;
        } else {
            temp = ssa_var * 2;
        }
        ssa_var = temp + 1;
    }
    
    /* OpenMP region with multiple clauses (OMP_CLAUSE) */
    int sum = 0;
    #pragma omp parallel for private(ssa_var) firstprivate(iterations) \
            shared(arr) reduction(+:sum) collapse(2) if(iterations > 3)
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            ssa_var = i * j;
            sum += arr[(i * 4 + j) % 10] + ssa_var;
        }
    }
    
    /* Nested OpenMP */
    #pragma omp parallel
    {
        #pragma omp sections private(ssa_var)
        {
            #pragma omp section
            { ssa_var = 1; }
            #pragma omp section
            { ssa_var = 2; }
        }
    }
    
#ifdef __cplusplus
    /* C++ specific: TREE_VEC via templates and TREE_BINFO via inheritance */
    std::vector<int> vec = {1, 2, 3, 4, 5}; /* TREE_VEC in GCC's representation */
    TemplateClass<double> tc(3.14159);
    
    Base* b = new Derived();
    b->base_data = vol;
    int result = b->method(); /* Virtual call needs BINFO */
    delete b;
    
    /* More template usage */
    std::vector<TemplateClass<int>> tvec;
    tvec.push_back(TemplateClass<int>(42));
#endif
    
    /* Use all computed values to prevent dead code elimination */
    int final_result = sum + ssa_var + recursive_data.values[0];
    
#ifdef __cplusplus
    final_result += vec.size();
    std::cout << "Result: " << final_result << std::endl;
#else
    /* Use external function to output result */
    use(&final_result);
#endif
    
    return (final_result > 0) ? 0 : 1;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
