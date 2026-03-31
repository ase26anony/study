/* tls_defs.c - Definitions of TLS variables with various attributes */

/* Force emulated TLS by including this in compilation flags */
#ifdef __GNUC__
#define THREAD_LOCAL __thread
#else
#define THREAD_LOCAL _Thread_local
#endif

/* Public TLS with external linkage */
THREAD_LOCAL int public_tls_var = 42;

/* Static TLS with internal linkage */
static THREAD_LOCAL int static_tls_var = 100;

/* Weak TLS symbol */
THREAD_LOCAL int weak_tls_var __attribute__((weak)) = 200;

/* Common TLS symbol (tentative definition) */
THREAD_LOCAL int common_tls_var __attribute__((common));

/* TLS with hidden visibility */
THREAD_LOCAL int hidden_tls_var __attribute__((visibility("hidden"))) = 300;

/* TLS with protected visibility */
THREAD_LOCAL int protected_tls_var __attribute__((visibility("protected"))) = 400;

/* DLL export simulation */
#ifdef _WIN32
THREAD_LOCAL int __declspec(dllexport) exported_tls_var = 500;
#else
THREAD_LOCAL int exported_tls_var __attribute__((visibility("default"))) = 500;
#endif

/* TLS variable that will be preserved */
THREAD_LOCAL volatile int preserved_tls_var = 600;

/* TLS in different contexts */
struct S {
    THREAD_LOCAL int member_tls;
};

/* Function to force usage of TLS variables */
void use_tls_variables_defs(void) {
    /* Take addresses to ensure TREE_USED is set */
    int* ptr1 = &public_tls_var;
    int* ptr2 = &static_tls_var;
    int* ptr3 = &weak_tls_var;
    int* ptr4 = &common_tls_var;
    int* ptr5 = &hidden_tls_var;
    int* ptr6 = &protected_tls_var;
    int* ptr7 = &exported_tls_var;
    int* ptr8 = &preserved_tls_var;
    
    /* Use in asm to prevent optimization */
    asm volatile ("" : : "r"(ptr1), "r"(ptr2), "r"(ptr3), "r"(ptr4),
                     "r"(ptr5), "r"(ptr6), "r"(ptr7), "r"(ptr8));
    
    /* Modify values */
    public_tls_var++;
    static_tls_var++;
    weak_tls_var++;
    common_tls_var = 999;
    hidden_tls_var++;
    protected_tls_var++;
    exported_tls_var++;
    preserved_tls_var++;
}

/* TLS variable used in OpenMP context */
#ifdef _OPENMP
THREAD_LOCAL int omp_tls_var = 700;
#endif
