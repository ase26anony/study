/* test_targhooks.c - Comprehensive test to trigger artificial symbol generation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef __cplusplus
#include <typeinfo>
#include <exception>
#endif

/* Pattern 1: AddressSanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
void asan_stack_overflow() {
    int arr[10];
    /* Potential stack buffer overflow - ASan will instrument */
    arr[15] = 0;  /* Out-of-bounds write */
}

void asan_use_after_return() {
    int *ptr;
    {
        int local = 42;
        ptr = &local;  /* Pointer to local variable */
    }
    /* Use after scope - ASan may detect */
    *ptr = 43;  /* Use after return */
}
#endif

/* Pattern 2: ThreadSanitizer triggers */
volatile int shared_counter = 0;

void* thread_func(void* arg) {
    for (int i = 0; i < 1000; i++) {
        shared_counter++;  /* Data race potential */
    }
    return NULL;
}

void tsan_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, thread_func, NULL);
    pthread_create(&t2, NULL, thread_func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter: %d\n", shared_counter);
}

/* Pattern 3: OpenMP artificial symbols */
#ifdef _OPENMP
void omp_reduction() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
}

void omp_private_vars() {
    int private_var = 0;
    #pragma omp parallel private(private_var)
    {
        private_var = omp_get_thread_num();
        #pragma omp critical
        {
            printf("Thread %d: private_var = %d\n", 
                   omp_get_thread_num(), private_var);
        }
    }
}
#endif

/* Pattern 4: Complex inline assembly */
void inline_asm_clobbers() {
    int x = 42, y = 0;
    
    /* Extended asm with multiple clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $10, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (y)
        : "r" (x)
        : "%eax", "%ebx", "memory", "cc"
    );
    
    printf("Inline asm result: %d -> %d\n", x, y);
}

/* Pattern 5: Static local variables (C++ guard variables) */
#ifdef __cplusplus
class Base {
public:
    virtual ~Base() {}
    virtual void foo() { printf("Base::foo\n"); }
};

class Derived : public Base {
public:
    virtual void foo() override { printf("Derived::foo\n"); }
};

void static_local_guard() {
    /* Static local with potential guard variable */
    static int counter = 0;
    counter++;
    printf("Static local counter: %d\n", counter);
}

void rtti_usage() {
    Base* b = new Derived();
    
    /* Use RTTI - generates typeinfo symbols */
    const std::type_info& ti = typeid(*b);
    printf("Type name: %s\n", ti.name());
    
    /* Use dynamic_cast */
    Derived* d = dynamic_cast<Derived*>(b);
    if (d) {
        d->foo();
    }
    
    delete b;
}

void exception_handling() {
    try {
        throw std::runtime_error("Test exception");
    } catch (const std::exception& e) {
        printf("Caught exception: %s\n", e.what());
    }
}
#endif

/* Pattern 6: Profile-guided optimization counters */
void pgo_hot_function() {
    /* Loop-heavy function for PGO */
    long long total = 0;
    for (int i = 0; i < 100000; i++) {
        for (int j = 0; j < 100; j++) {
            total += i * j;
        }
    }
    printf("PGO hot function total: %lld\n", total);
}

/* Pattern 7: Builtin usage */
void builtin_functions() {
    /* Use compiler builtins */
    int x = 42;
    int y = __builtin_popcount(x);
    printf("Builtin popcount(%d) = %d\n", x, y);
    
    /* Memory builtins */
    char src[100], dst[100];
    __builtin_memcpy(dst, src, sizeof(src));
    __builtin_memset(src, 0, sizeof(src));
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int run_sanitizers = 0;
    
    /* Check if we should run sanitizer-triggering code */
    if (argc > 1 && strcmp(argv[1], "--run-all") == 0) {
        run_sanitizers = 1;
    }
    
    printf("=== Starting artificial symbol generation test ===\n");
    
    /* Always run safe patterns */
    pgo_hot_function();
    builtin_functions();
    
    #ifdef _OPENMP
    omp_reduction();
    omp_private_vars();
    #endif
    
    inline_asm_clobbers();
    
    #ifdef __cplusplus
    static_local_guard();
    rtti_usage();
    exception_handling();
    #endif
    
    /* Conditionally run sanitizer patterns */
    if (run_sanitizers) {
        #ifdef __SANITIZE_ADDRESS__
        printf("\nRunning ASan patterns...\n");
        asan_stack_overflow();
        asan_use_after_return();
        #endif
        
        printf("\nRunning TSan patterns...\n");
        tsan_data_race();
    } else {
        printf("\nSanitizer patterns skipped (use --run-all to execute)\n");
    }
    
    printf("\n=== Test completed ===\n");
    return 0;
}
