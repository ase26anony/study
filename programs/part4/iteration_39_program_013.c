/* test_tree_coverage.c - Comprehensive test to trigger specific tree node classifications */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization from removing our constructs */
#define NOOPT __attribute__((noinline))
#define USED __attribute__((used))
#define VOLATILE_VAR volatile int

/* ========== IDENTIFIER_NODE generation ========== */
/* Generate many unique identifiers using macros */
#define CONCAT(a, b) a##b
#define MAKE_ID(n) CONCAT(var_, n)
#define MAKE_FUNC(n) CONCAT(func_, n)

/* Generate multiple identifiers */
#define GEN_IDENTS(n) \
    int MAKE_ID(n) = n; \
    void MAKE_FUNC(n)(void) { MAKE_ID(n) += 1; }

/* Instantiate many identifiers */
GEN_IDENTS(0) GEN_IDENTS(1) GEN_IDENTS(2) GEN_IDENTS(3) GEN_IDENTS(4)
GEN_IDENTS(5) GEN_IDENTS(6) GEN_IDENTS(7) GEN_IDENTS(8) GEN_IDENTS(9)

/* More complex identifier usage */
NOOPT void test_identifiers(void) {
    /* Use all generated identifiers */
    int sum = var_0 + var_1 + var_2 + var_3 + var_4 + 
              var_5 + var_6 + var_7 + var_8 + var_9;
    
    /* Call generated functions */
    func_0(); func_1(); func_2(); func_3(); func_4();
    func_5(); func_6(); func_7(); func_8(); func_9();
    
    /* Use volatile to prevent optimization */
    VOLATILE_VAR v = sum;
    asm volatile("" : : "r"(v) : "memory");
}

/* ========== TREE_VEC generation ========== */
/* Vector types using GCC extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

NOOPT v4si test_vectors(void) {
    v4si a = {1, 2, 3, 4};
    v4si b = {5, 6, 7, 8};
    v4si c = {9, 10, 11, 12};
    
    /* Complex vector operations */
    v4si result = a + b * c - (a & b) | (c << 2);
    
    /* Mix with float vectors */
    v4sf f1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf f2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf fresult = f1 * f2 + f1 / f2;
    
    /* Prevent optimization */
    VOLATILE_VAR vr[4];
    vr[0] = result[0]; vr[1] = result[1];
    vr[2] = (int)fresult[0]; vr[3] = (int)fresult[1];
    asm volatile("" : : "r"(vr) : "memory");
    
    return result;
}

/* ========== SSA_NAME generation ========== */
NOOPT int test_ssa(int n) {
    int x = 0, y = 0, z = 0;
    
    /* Complex loop with multiple assignments to create SSA */
    for (int i = 0; i < n; i++) {
        /* Multiple conditional assignments */
        if (i % 3 == 0) {
            x = i * 2;
            y = x + 1;
        } else if (i % 3 == 1) {
            x = i * 3;
            y = x - 1;
        } else {
            x = i * 4;
            y = x / 2;
        }
        
        /* Phi node creation */
        z += (x > y) ? x : y;
        
        /* Nested conditionals */
        if (z % 5 == 0) {
            int temp = z;
            for (int j = 0; j < 3; j++) {
                temp += j * (i % 2 ? x : y);
            }
            z = temp;
        }
    }
    
    /* Prevent optimization */
    VOLATILE_VAR v = x + y + z;
    asm volatile("" : : "r"(v) : "memory");
    
    return z;
}

/* ========== BLOCK generation ========== */
NOOPT int test_blocks(int n) {
    int outer = 0;
    
    /* Level 1 block */
    {
        int level1 = n * 2;
        
        /* Level 2 block */
        {
            int level2 = level1 + 1;
            
            /* Level 3 block inside loop */
            for (int i = 0; i < 5; i++) {
                /* Level 4 block */
                {
                    int level3 = level2 * i;
                    
                    /* Level 5 block with conditional */
                    if (level3 % 2 == 0) {
                        /* Level 6 block */
                        {
                            int level4 = level3 / 2;
                            outer += level4;
                        }
                    } else {
                        /* Another level 6 block */
                        {
                            int level4 = level3 * 3;
                            outer -= level4;
                        }
                    }
                }
            }
        }
        
        /* Another level 2 block */
        {
            int level2b = outer * 7;
            outer = level2b % 13;
        }
    }
    
    /* Switch with blocks */
    switch (outer % 4) {
        case 0: {
            int case0 = outer + 1;
            outer = case0 * 2;
            break;
        }
        case 1: {
            int case1 = outer - 1;
            outer = case1 / 2;
            break;
        }
        default: {
            int casedef = outer * 3;
            outer = casedef % 7;
            break;
        }
    }
    
    return outer;
}

/* ========== CONSTRUCTOR generation ========== */
struct ComplexStruct {
    int a, b, c;
    float f;
    double d;
};

struct NestedStruct {
    struct ComplexStruct inner;
    int extra;
    int arr[3];
};

NOOPT struct NestedStruct test_constructors(int x, float y, double z) {
    /* Non-constant initializers */
    int (*func_ptr)(void) = test_identifiers;
    
    /* Complex aggregate initialization */
    struct NestedStruct ns = {
        .inner = {
            .a = x + 1,
            .b = x * 2,
            .c = x % 7,
            .f = y * 2.0f,
            .d = z / 3.0
        },
        .extra = (int)(y * 100.0f),
        .arr = { x, x + 1, x + 2 }
    };
    
    /* Array with non-constant initializers */
    int dynamic_array[4] = {
        x,
        x * 2,
        ns.inner.a,
        ns.extra
    };
    
    /* Struct array */
    struct ComplexStruct struct_array[2] = {
        { .a = x, .b = x + 1, .c = x + 2, .f = y, .d = z },
        { .a = dynamic_array[0], .b = dynamic_array[1], 
          .c = dynamic_array[2], .f = y * 2.0f, .d = z * 2.0 }
    };
    
    /* Prevent optimization */
    VOLATILE_VAR v = ns.inner.a + struct_array[0].b + dynamic_array[3];
    asm volatile("" : : "r"(v) : "memory");
    
    return ns;
}

/* ========== OMP_CLAUSE generation ========== */
#ifdef _OPENMP
#include <omp.h>

NOOPT int test_omp_clauses(int n) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 2;
    }
    
    /* OpenMP with multiple clauses */
    #pragma omp parallel for private(n) shared(arr) reduction(+:sum) \
            schedule(dynamic, 4) num_threads(2) if(n > 1000)
    for (int i = 0; i < 100; i++) {
        int local = arr[i];
        /* Nested block inside parallel region */
        {
            int squared = local * local;
            sum += squared % 17;
        }
    }
    
    /* Another OpenMP construct with different clauses */
    int max_val = 0;
    #pragma omp parallel sections private(arr) firstprivate(n) \
            reduction(max:max_val) copyin(arr[0:10])
    {
        #pragma omp section
        {
            int section_sum = 0;
            for (int i = 0; i < 50; i++) {
                section_sum += arr[i];
            }
            max_val = section_sum;
        }
        
        #pragma omp section
        {
            int section_max = arr[0];
            for (int i = 50; i < 100; i++) {
                if (arr[i] > section_max) {
                    section_max = arr[i];
                }
            }
            if (section_max > max_val) {
                max_val = section_max;
            }
        }
    }
    
    /* Task with clauses */
    #pragma omp task depend(inout: sum) final(n > 500) priority(1)
    {
        sum = (sum * 2) % 1000;
    }
    
    #pragma omp taskwait
    
    return sum + max_val;
}
#else
NOOPT int test_omp_clauses(int n) {
    /* Dummy implementation when OpenMP not available */
    return n * 2;
}
#endif

/* ========== TREE_BINFO generation (C++ version) ========== */
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

class MultiDerived : public DerivedClass {
public:
    virtual int method() override { return 3; }
    int multi_data;
};

NOOPT int test_binfo(void) {
    BaseClass* base = new DerivedClass();
    DerivedClass* derived = new MultiDerived();
    
    int result = base->method() + derived->method();
    
    /* Use dynamic_cast to trigger BINFO lookups */
    if (DerivedClass* d = dynamic_cast<DerivedClass*>(base)) {
        result += d->derived_data;
    }
    
    delete base;
    delete derived;
    
    return result;
}
#else
/* C version using LTO structures that might generate BINFO nodes */
struct BaseStruct {
    int type_id;
    void* vtable[2];
};

struct DerivedStruct {
    struct BaseStruct base;
    int extra_data;
};

NOOPT int test_binfo(void) {
    struct DerivedStruct ds = { .base = { .type_id = 1 }, .extra_data = 42 };
    return ds.base.type_id + ds.extra_data;
}
#endif

/* ========== Main function ========== */
int main(int argc, char** argv) {
    int result = 0;
    
    /* Test all tree node types */
    test_identifiers();
    
    v4si vec_result = test_vectors();
    result += vec_result[0];
    
    result += test_ssa(argc > 1 ? atoi(argv[1]) : 100);
    
    result += test_blocks(result);
    
    struct NestedStruct ns = test_constructors(result, 3.14f, 2.71828);
    result += ns.inner.a + ns.extra;
    
    result += test_omp_clauses(result);
    
    result += test_binfo();
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return result % 256;
}
