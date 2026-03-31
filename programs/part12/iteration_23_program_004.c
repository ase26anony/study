/* test_targhooks.c - Comprehensive test to trigger artificial symbol generation */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

/* Pattern 1: AddressSanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
void asan_stack_overflow() {
    int arr[10];
    /* Deliberate overflow when ASan is enabled */
    arr[15] = 42;  /* Out-of-bounds write */
}

void asan_use_after_return() {
    static volatile int* dangling_ptr = NULL;
    {
        int local = 42;
        dangling_ptr = &local;
    }
    /* Use after scope - only dangerous with ASan */
    if (dangling_ptr) {
        volatile int x = *dangling_ptr;
        (void)x;
    }
}
#endif

/* Pattern 2: ThreadSanitizer triggers */
volatile int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_counter++;  /* Data race when unsynchronized */
    }
    return NULL;
}

void create_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter (racy): %d\n", shared_counter);
}

/* Pattern 3: OpenMP artificial symbols */
#ifdef _OPENMP
void omp_reduction_test() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

void omp_private_test() {
    int private_var = 0;
    #pragma omp parallel private(private_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            printf("Thread %d private_var: %d\n", omp_get_thread_num(), private_var);
        }
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void inline_asm_test() {
    int x = 42, y = 0;
    
    /* Complex asm with multiple clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "%ecx", "memory", "cc"
    );
    
    printf("Inline asm result: %d\n", y);
    
    /* asm goto example */
    asm goto (
        "cmpl $100, %0\n\t"
        "jg %l[label]\n\t"
        : : "r" (y) : : label
    );
    
    printf("Did not jump\n");
    return;
    
label:
    printf("Jumped to label\n");
}

/* Pattern 5: C++ features (compiled as C++ if needed) */
#ifdef __cplusplus
class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
    int base_data;
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
    int derived_data;
};

void cpp_rtti_test() {
    Base* b = new Derived();
    
    /* Trigger typeinfo generation */
    if (Derived* d = dynamic_cast<Derived*>(b)) {
        d->foo();
    }
    
    /* Static local with potential guard variable */
    static int static_local = []() {
        printf("Initializing static local\n");
        return 42;
    }();
    
    printf("Static local value: %d\n", static_local);
    
    delete b;
}

void cpp_exception_test() {
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
}
#endif

/* Pattern 6: Profile-guided optimization triggers */
void pgo_hot_function() {
    /* Hot loop for PGO instrumentation */
    volatile int counter = 0;
    for (int i = 0; i < 10000; i++) {
        counter += i * 2;
        if (i % 1000 == 0) {
            printf("PGO loop progress: %d\n", i);
        }
    }
    (void)counter;
}

/* Pattern 7: Builtin functions */
void builtin_test() {
    /* Use GCC builtins that might generate internal symbols */
    int x = 42;
    printf("Population count of %d: %d\n", x, __builtin_popcount(x));
    printf("Clz of %d: %d\n", x, __builtin_clz(x));
    
    /* Memory builtins */
    char src[100], dst[100];
    __builtin_memcpy(dst, src, sizeof(src));
    __builtin_memset(src, 0, sizeof(src));
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizer_tests = 0;
    
    /* Check if we should run sanitizer-specific tests */
    if (argc > 1 && strcmp(argv[1], "--run-all") == 0) {
        run_sanitizer_tests = 1;
    }
    
    printf("=== Starting artificial symbol generation tests ===\n");
    
    /* Pattern 1: Sanitizers (conditionally executed) */
    if (run_sanitizer_tests) {
        #ifdef __SANITIZE_ADDRESS__
        printf("\n--- ASan tests ---\n");
        asan_stack_overflow();
        asan_use_after_return();
        #endif
    }
    
    /* Pattern 2: ThreadSanitizer */
    printf("\n--- ThreadSanitizer-like test ---\n");
    create_data_race();
    
    /* Pattern 3: OpenMP */
    #ifdef _OPENMP
    printf("\n--- OpenMP tests ---\n");
    omp_reduction_test();
    omp_private_test();
    #endif
    
    /* Pattern 4: Inline assembly */
    printf("\n--- Inline assembly test ---\n");
    inline_asm_test();
    
    /* Pattern 5: C++ features */
    #ifdef __cplusplus
    printf("\n--- C++ features test ---\n");
    cpp_rtti_test();
    cpp_exception_test();
    #endif
    
    /* Pattern 6: PGO hot code */
    printf("\n--- PGO hot function ---\n");
    pgo_hot_function();
    
    /* Pattern 7: Builtins */
    printf("\n--- Builtin functions ---\n");
    builtin_test();
    
    printf("\n=== All tests completed ===\n");
    return 0;
}
