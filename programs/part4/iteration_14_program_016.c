Here's a comprehensive test program that targets all the uncovered tree node types in `tree_code_kind`:

```cpp
// Compile with: g++ -O1 -fopenmp -fno-strict-aliasing tree_test.cc -o tree_test
// Also test with: g++ -O0 -fopenmp tree_test.cc -o tree_test

#include <iostream>
#include <cstdlib>

// ==================== C++ CLASSES FOR TREE_BINFO ====================
struct Base1 {
    virtual ~Base1() {}
    virtual void vfunc1() { std::cout << "Base1\n"; }
    int data1;
};

struct Base2 {
    virtual ~Base2() {}
    virtual void vfunc2() { std::cout << "Base2\n"; }
    int data2;
};

// Virtual inheritance to ensure BINFO nodes are created
struct Derived : virtual Base1, virtual Base2 {
    virtual void vfunc1() override { std::cout << "Derived\n"; }
    virtual void vfunc2() override { std::cout << "Derived\n"; }
    int derived_data;
};

// Another class for more BINFO variations
template<typename T>
class TemplateClass : public Base1 {
public:
    T template_data;
    virtual void vfunc1() override {}
};

// ==================== FUNCTIONS FOR VARIOUS TREE NODES ====================

// Function with IDENTIFIER_NODE (labels), BLOCK, and TREE_VEC
__attribute__((noinline, optimize("O0")))
static void function_with_multiple_nodes(int n) {
    // Use __label__ for local labels (IDENTIFIER_NODE)
    __label__ label1, label2, exit_label;
    
    // BLOCK nodes from nested scopes
    {
        int block_var1 = 10;
        // Another nested block
        {
            int block_var2 = block_var1 * 2;
            // GCC statement expression creates BLOCK
            int stmt_expr = ({
                int temp = block_var2;
                temp * 3;
            });
            (void)stmt_expr;
        }
    }
    
    // TREE_VEC from VLA and compound literals
    if (n > 0) {
        // Variable-length array (TREE_VEC)
        int vla[n];
        for (int i = 0; i < n; ++i) {
            vla[i] = i * 2;
        }
        
        // Compound literal with designators (TREE_VEC)
        struct Point {
            int x, y, z;
        };
        struct Point p = (struct Point){.x = 1, .y = 2, .z = 3};
        
        // Array compound literal
        int arr[] = (int[]){[0] = 1, [2] = 3, [4] = 5};
    }
    
    // Use goto with labels (IDENTIFIER_NODE)
    goto label1;
    
label1:
    {
        int x = 5;
        if (x > 10) goto label2;
    }
    
label2:
    // CONSTRUCTOR nodes from aggregate initialization
    struct Complex {
        int a;
        double b;
        char c[4];
    };
    
    // Designated initializer (CONSTRUCTOR)
    struct Complex c1 = {
        .a = 42,
        .b = 3.14,
        .c = {'a', 'b', 'c', '\0'}
    };
    
    // Nested constructor
    struct Outer {
        struct Inner {
            int x, y;
        } inner;
        int data;
    };
    
    struct Outer o1 = {
        .inner = {.x = 1, .y = 2},
        .data = 100
    };
    
    // More complex constructor with arrays
    struct WithArray {
        int nums[5];
        struct Point points[2];
    };
    
    struct WithArray wa = {
        .nums = {[0] = 1, [3] = 4},
        .points = {{.x = 1, .y = 2, .z = 3},
                   {.x = 4, .y = 5, .z = 6}}
    };
    
    (void)c1; (void)o1; (void)wa;
    
    goto exit_label;
    
exit_label:
    return;
}

// Function that will generate SSA_NAME nodes
__attribute__((noinline))
static int ssa_generating_function(int iterations) {
    int sum = 0;
    float fsum = 0.0f;
    
    // Loop with conditional assignments - creates SSA_NAME nodes
    for (int i = 0; i < iterations; ++i) {
        // This creates phi nodes in SSA form
        if (i % 2 == 0) {
            sum += i * 2;
            fsum += i * 0.5f;
        } else {
            sum += i;
            fsum += i * 0.25f;
        }
        
        // More complex SSA patterns
        int temp = sum;
        for (int j = 0; j < 3; ++j) {
            temp += j;
            if (temp > 100) {
                fsum *= 1.1f;
            }
        }
        sum = temp;
    }
    
    // Conditional that creates merge points
    int result;
    if (sum > 1000) {
        result = sum / 2;
    } else {
        result = sum * 2;
    }
    
    return result + (int)fsum;
}

// Function with OpenMP clauses (OMP_CLAUSE)
__attribute__((noinline))
static int openmp_function(int size) {
    int total = 0;
    int* array = new int[size];
    
    // Initialize array
    for (int i = 0; i < size; ++i) {
        array[i] = i + 1;
    }
    
    // OpenMP region with multiple clauses
    #pragma omp parallel for reduction(+:total) private(size) \
            schedule(static, 16) if(size > 1000)
    for (int i = 0; i < size; ++i) {
        // Nested OpenMP (more clauses)
        #pragma omp atomic
        total += array[i];
        
        // SIMD loop with clause
        #pragma omp simd reduction(+:total) aligned(array:64)
        for (int j = 0; j < 4; ++j) {
            total += j;
        }
    }
    
    // OpenMP sections with different clauses
    #pragma omp parallel sections private(array) \
            num_threads(2)
    {
        #pragma omp section
        {
            total += 10;
        }
        
        #pragma omp section
        {
            total += 20;
        }
    }
    
    delete[] array;
    return total;
}

// Recursive function with mixed nodes
__attribute__((noinline))
static void recursive_function(int depth, int max_depth) {
    // Base case
    if (depth >= max_depth) {
        return;
    }
    
    // BLOCK in recursion
    {
        // CONSTRUCTOR in recursive context
        struct RecursiveData {
            int level;
            int data[3];
        };
        
        struct RecursiveData rd = {
            .level = depth,
            .data = {depth * 1, depth * 2, depth * 3}
        };
        
        // TREE_VEC from compound literal
        int* local_array = (int[3]){rd.data[0], rd.data[1], rd.data[2]};
        
        // Use GNU nested function (creates BLOCK)
        auto nested_func = [&]() {
            return rd.level + local_array[0];
        };
        
        (void)nested_func();
    }
    
    // Recursive call
    recursive_function(depth + 1, max_depth);
}

// ==================== MAIN FUNCTION ====================

int main() {
    // Initialize with CONSTRUCTOR
    struct AppState {
        int counter;
        double values[4];
        struct Config {
            int mode;
            char name[16];
        } config;
    };
    
    struct AppState state = {
        .counter = 0,
        .values = {1.1, 2.2, 3.3, 4.4},
        .config = {
            .mode = 1,
            .name = "test_config"
        }
    };
    
    // Call function with multiple node types
    function_with_multiple_nodes(10);
    
    // Generate SSA_NAME nodes
    int ssa_result = ssa_generating_function(100);
    std::cout << "SSA result: " << ssa_result << std::endl;
    
    // Exercise C++ RTTI and BINFO nodes
    Derived* derived = new Derived();
    Base1* base1 = derived;
    Base2* base2 = derived;
    
    // dynamic_cast uses BINFO
    Derived* back1 = dynamic_cast<Derived*>(base1);
    Derived* back2 = dynamic_cast<Derived*>(base2);
    
    // Cross-cast (requires BINFO traversal)
    Base2* cross_cast = dynamic_cast<Base2*>(base1);
    
    // typeid also uses BINFO
    const std::type_info& ti1 = typeid(*derived);
    const std::type_info& ti2 = typeid(*base1);
    
    std::cout << "Type names: " << ti1.name() << ", " << ti2.name() << std::endl;
    
    // Template instantiation for more BINFO
    TemplateClass<float> tc;
    Base1* base_tc = &tc;
    
    // Use OpenMP with clauses
    int omp_result = openmp_function(500);
    std::cout << "OpenMP result: " << omp_result << std::endl;
    
    // Recursive function with mixed nodes
    recursive_function(0, 3);
    
    // More complex OpenMP with collapse clause
    int matrix[10][10];
    #pragma omp parallel for collapse(2) private(state)
    for (int i = 0; i < 10; ++i) {
        for (int j = 0; j < 10; ++j) {
            matrix[i][j] = i * j;
            // SSA variables inside OpenMP
            int ssa_var = matrix[i][j];
            if (ssa_var % 2 == 0) {
                matrix[i][j] = ssa_var * 2;
            } else {
                matrix[i][j] = ssa_var / 2;
            }
        }
    }
    
    // Cleanup
    delete derived;
    
    // Final print to prevent optimization
    std::cout << "Final counter: " << state.counter << std::endl;
    
    return 0;
}
```

This test program specifically targets each uncovered tree node type:

1. **IDENTIFIER_NODE**: Created through `__label__` declarations and `goto` labels
2. **TREE_VEC**: Generated by VLAs and compound literals with designators
3. **TREE_BINFO**: Created through C++ virtual inheritance, `dynamic_cast`, and `typeid`
4. **SSA_NAME**: Generated by loops and conditional assignments with optimization enabled
5. **BLOCK**: Created through nested scopes, statement expressions, and GNU nested functions
6. **CONSTRUCTOR**: Generated by designated initializers and compound literals
7. **OMP_CLAUSE**: Created through various OpenMP pragmas with multiple clauses

The program should be compiled with different optimization levels to ensure all node types are created and classified by `tree_code_kind`.
