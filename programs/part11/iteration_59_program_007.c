/* test_targhooks.c */
/* Compile with: gcc -O2 -fvisibility=hidden -fexceptions -c test_targhooks.c -o test_targhooks.o */
/* For C++: g++ -O2 -fvisibility=hidden -fexceptions -c test_targhooks.cc -o test_targhooks.o */

/* Force generation of hidden external symbols */
#pragma GCC visibility push(hidden)

/* External volatile variable with hidden visibility */
extern volatile int hidden_volatile_var;
volatile int hidden_volatile_var = 0;

/* External function with nothrow attribute and hidden visibility */
extern void hidden_nothrow_func(void) __attribute__((nothrow));
void hidden_nothrow_func(void) {
    /* Empty but external */
}

/* Thread-local storage with explicit model */
__thread int tls_var __attribute__((tls_model("initial-exec")));

/* Constructor with hidden visibility */
static void __attribute__((constructor, visibility("hidden"))) hidden_constructor(void) {
    /* Use volatile to prevent optimization */
    volatile int x = 0;
    /* Use __builtin_expect with volatile condition */
    if (__builtin_expect((x == 0), 0)) {
        hidden_volatile_var = 1;
    }
}

/* Destructor with hidden visibility */
static void __attribute__((destructor, visibility("hidden"))) hidden_destructor(void) {
    /* Access TLS variable */
    tls_var = 42;
}

#pragma GCC visibility pop

/* Artificial function that might be ignored */
static inline __attribute__((unused, always_inline)) 
void artificial_ignored(void) {
    /* Use __builtin_unreachable to create interesting control flow */
    if (hidden_volatile_var) {
        __builtin_unreachable();
    }
}

/* Complex function pointer type */
typedef void (*complex_func_ptr)(void) __attribute__((nothrow));

#ifdef __cplusplus
/* C++ specific code to trigger exception handling helpers */
extern "C" void external_nothrow_c_func(void) __attribute__((nothrow));

namespace hidden_visibility {
    __attribute__((visibility("hidden"))) 
    extern "C" void hidden_extern_c_func(void) {
        /* Empty but has interesting attributes */
    }
}

/* Class with static member that might trigger helper generation */
class TriggerClass {
public:
    static volatile int static_volatile_member;
    
    __attribute__((noinline, visibility("hidden")))
    static void hidden_static_method(void) noexcept {
        /* Use atomic operations */
        __atomic_fetch_add(&static_volatile_member, 1, __ATOMIC_SEQ_CST);
    }
};

volatile int TriggerClass::static_volatile_member = 0;

/* Try block that calls nothrow external function */
void test_exception_context(void) {
    try {
        external_nothrow_c_func();
        hidden_nothrow_func();
        hidden_visibility::hidden_extern_c_func();
    } catch (...) {
        /* Catch all */
    }
}
#endif

/* Main function orchestrates everything */
int main(void) {
    /* Force use of hidden volatile variable */
    int local = hidden_volatile_var + 1;
    
    /* Use inline assembly to prevent optimization */
    __asm__ volatile ("" : : "r"(local));
    
    /* Call hidden constructor-like behavior */
    hidden_nothrow_func();
    
    /* Access TLS */
    tls_var = local;
    
    /* Call artificial function */
    artificial_ignored();
    
#ifdef __cplusplus
    /* C++ specific calls */
    TriggerClass::hidden_static_method();
    test_exception_context();
    
    /* Use complex function pointer */
    complex_func_ptr fp = hidden_nothrow_func;
    if (fp) fp();
#endif
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = (int)(long)&tls_var + local + hidden_volatile_var;
    
    /* Use checksum in a way that can't be optimized away */
    __asm__ volatile ("" : : "r"(checksum));
    
    return checksum & 0xFF;
}

/* External C function declaration (no definition to keep it external) */
#ifdef __cplusplus
extern "C" {
#endif
    void external_nothrow_c_func(void) __attribute__((nothrow));
#ifdef __cplusplus
}
#endif

/* Define it weak to avoid linker error but keep it external */
void external_nothrow_c_func(void) __attribute__((weak, nothrow));
void external_nothrow_c_func(void) {
    /* Weak definition */
}
