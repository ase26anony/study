/* Test for emulated TLS attribute copying - Main file */
#include <stdio.h>
#include <stdint.h>

/* Pattern A: Force emulated TLS with compiler flag */
/* Compile with: gcc -ftls-model=emulated -fPIC tls_main.c tls_aux.c -o tls_test */

/* 1. DECL_PRESERVE_P - Variables that should not be eliminated */
__thread int tls_preserve __attribute__((used)) = 42;
__thread int tls_not_preserve = 100;

/* 2. TREE_USED - Variables that are referenced in code */
__thread int tls_used_var = 0;
static __thread int tls_unused_var = 0;  /* Will be used later */

/* 3. TREE_PUBLIC / DECL_EXTERNAL - Mix of public and static */
__thread int tls_public = 1;           /* Public (non-static) */
static __thread int tls_static = 2;    /* Static linkage */
extern __thread int tls_extern;        /* External declaration */

/* 4. DECL_COMMON - Tentative definitions */
__thread int tls_common;               /* Common symbol */

/* 5. DECL_WEAK - Weak symbols */
__thread int tls_weak __attribute__((weak)) = 5;
extern __thread int tls_weak_undefined __attribute__((weak));

/* 6. DECL_VISIBILITY - Different visibility attributes */
__thread int tls_default __attribute__((visibility("default"))) = 10;
__thread int tls_hidden __attribute__((visibility("hidden"))) = 20;
__thread int tls_protected __attribute__((visibility("protected"))) = 30;

/* 7. DECL_DLLIMPORT_P - DLL import attribute (Windows targets) */
#ifdef _WIN32
extern __thread int tls_dllimport __declspec(dllimport);
#elif defined(__MINGW32__) || defined(__CYGWIN__)
extern __thread int tls_dllimport __attribute__((dllimport));
#endif

/* 8. DECL_CONTEXT - Variables in different scopes */
static void function_scope(void) {
    /* Local TLS variable */
    static __thread int tls_local_static = 50;
    tls_local_static++;
}

/* Complex usage patterns to ensure processing */
__thread int* tls_pointer;
__thread struct {
    int a;
    int b;
} tls_struct = {1, 2};

/* Pattern C: Use in complex expressions */
volatile void* force_use(void* ptr) {
    /* Prevent optimization */
    __asm__ volatile ("" : : "r"(ptr) : "memory");
    return ptr;
}

/* Pattern D: Different TLS models */
__thread int tls_emulated __attribute__((tls_model("emulated"))) = 99;
__thread int tls_global_dynamic __attribute__((tls_model("global-dynamic"))) = 100;

/* Function that uses TLS variables in non-trivial ways */
int use_tls_complex(void) {
    int sum = 0;
    
    /* Take addresses */
    int* ptr1 = &tls_public;
    int* ptr2 = &tls_static;
    
    /* Use in asm statement */
    __asm__ volatile (
        "addl %1, %0\n\t"
        : "+r"(sum)
        : "m"(tls_used_var)
    );
    
    /* Complex expression */
    sum += *ptr1 + *ptr2 + tls_struct.a + tls_struct.b;
    
    /* Force use of pointer */
    force_use(ptr1);
    force_use(ptr2);
    
    return sum;
}

int main(void) {
    int result = 0;
    
    /* Ensure all TLS variables are TREE_USED */
    tls_used_var = 1;
    tls_unused_var = 2;  /* Now it's used */
    
    /* Use public TLS variables */
    tls_public++;
    tls_static++;
    
    /* Use extern TLS variable (defined in another file) */
    tls_extern = 100;
    
    /* Use common TLS variable */
    tls_common = 200;
    
    /* Use weak TLS variable */
    if (&tls_weak) {
        tls_weak++;
    }
    
    /* Use visibility-controlled TLS variables */
    tls_default++;
    tls_hidden++;
    tls_protected++;
    
    /* Use DLL import if available */
#ifdef tls_dllimport
    tls_dllimport = 300;
#endif
    
    /* Call function with local TLS */
    function_scope();
    
    /* Use complex TLS types */
    tls_pointer = &tls_public;
    tls_struct.a++;
    tls_struct.b++;
    
    /* Use different TLS models */
    tls_emulated++;
    tls_global_dynamic++;
    
    /* Complex usage */
    result += use_tls_complex();
    
    /* Calculate final result using all TLS variables */
    result += tls_preserve + tls_not_preserve + tls_used_var + tls_unused_var +
              tls_public + tls_static + tls_extern + tls_common + tls_weak +
              tls_default + tls_hidden + tls_protected + tls_emulated +
              tls_global_dynamic;
    
    printf("Result: %d\n", result);
    return 0;
}
