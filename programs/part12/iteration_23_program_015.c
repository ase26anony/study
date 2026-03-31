/* test_targhooks.c - Combined triggers for artificial symbol generation */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#ifdef __cplusplus
#include <typeinfo>
#include <exception>
#endif

/* Pattern 1: Sanitizer triggers */
#ifdef __SANITIZE_ADDRESS__
#define USE_SANITIZERS 1
#else
#define USE_SANITIZERS 0
#endif

void trigger_asan_stack_overflow() {
    int arr[10];
    /* Deliberate overflow - only executed if sanitizers are enabled */
    if (USE_SANITIZERS) {
        arr[15] = 42;  /* Stack buffer overflow for ASan */
    }
}

int* trigger_asan_use_after_return() {
    int local = 42;
    return &local;  /* Use after return warning/error */
}

/* Pattern 2: ThreadSanitizer data race */
static int shared_counter = 0;

void* increment_counter(void* arg) {
    for (int i = 0; i < 1000; ++i) {
        shared_counter++;  /* Data race - no synchronization */
    }
    return NULL;
}

void trigger_tsan_data_race() {
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment_counter, NULL);
    pthread_create(&t2, NULL, increment_counter, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("Shared counter (racy): %d\n", shared_counter);
}

/* Pattern 3: OpenMP artificial symbols */
void trigger_openmp_symbols() {
    int sum = 0;
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < 1000; ++i) {
        sum += i;
    }
    printf("OpenMP reduction sum: %d\n", sum);
    
    /* Thread-private variables */
    #pragma omp parallel
    {
        static int thread_specific = 0;
        #pragma omp threadprivate(thread_specific)
        thread_specific++;
    }
}

/* Pattern 4: Complex inline assembly */
void trigger_assembly_symbols(int x) {
    int result;
    /* Complex asm with multiple clobbers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl $100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (result)
        : "r" (x)
        : "%eax", "%ebx", "%ecx", "memory", "cc"
    );
    printf("Assembly result: %d\n", result);
    
    /* asm goto example */
    asm goto (
        "cmpl $0, %0\n\t"
        "je %l[label]\n\t"
        : : "r" (x) : : label
    );
    printf("Did not jump\n");
    return;
    
label:
    printf("Jumped via asm goto\n");
}

#ifdef __cplusplus
/* Pattern 5a: Static local in inline function */
inline int static_local_counter() {
    static int counter = 0;  /* Guard variable may be artificial */
    return ++counter;
}

/* Pattern 5b: Exception handling personality */
void trigger_exception_symbols() {
    try {
        throw 42;
    } catch (int e) {
        printf("Caught exception: %d\n", e);
    }
}

/* Pattern 5c: RTTI and typeinfo */
class Base {
public:
    virtual ~Base() {}
    virtual void foo() {}
};

class Derived : public Base {
public:
    void foo() override {}
};

void trigger_rtti_symbols() {
    Derived d;
    Base* b = &d;
    
    /* Use typeid and dynamic_cast */
    printf("Type name: %s\n", typeid(*b).name());
    
    Derived* dd = dynamic_cast<Derived*>(b);
    if (dd) {
        printf("Dynamic cast successful\n");
    }
}
#endif

/* Pattern 6: Profile-guided optimization counters */
void trigger_pgo_counters() {
    /* Hot loop for PGO */
    volatile int counter = 0;
    for (int i = 0; i < 10000; ++i) {
        counter += i * 2;
    }
    printf("PGO loop completed: %d\n", counter);
}

/* Pattern 7: Builtin functions */
void trigger_builtin_symbols() {
    /* Use GCC builtins that may generate internal symbols */
    int x = 42;
    int y = __builtin_popcount(x);
    int z = __builtin_ctz(x);
    printf("Builtins: popcount(%d)=%d, ctz(%d)=%d\n", x, y, x, z);
    
    /* Stack protector canary */
    char buffer[100];
    strcpy(buffer, "test");
    printf("Buffer: %s\n", buffer);
}

/* Main orchestrator */
int main(int argc, char** argv) {
    int use_sanitizers = 0;
    
    /* Check if we should actually trigger sanitizer errors */
    if (argc > 1 && strcmp(argv[1], "--trigger-errors") == 0) {
        use_sanitizers = 1;
    }
    
    printf("=== Testing artificial symbol generation ===\n");
    
    /* Always execute these patterns */
    trigger_openmp_symbols();
    trigger_assembly_symbols(42);
    trigger_pgo_counters();
    trigger_builtin_symbols();
    
    /* Thread-related patterns */
    trigger_tsan_data_race();
    
    #ifdef __cplusplus
    static_local_counter();
    trigger_exception_symbols();
    trigger_rtti_symbols();
    #endif
    
    /* Sanitizer patterns - conditionally executed */
    if (use_sanitizers) {
        trigger_asan_stack_overflow();
        int* ptr = trigger_asan_use_after_return();
        printf("Use after return pointer: %p\n", (void*)ptr);
    }
    
    printf("=== Test completed ===\n");
    return 0;
}
