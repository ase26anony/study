/* Define TLS variables with diverse attributes to trigger emutls_decl logic */

/* Public TLS with external linkage */
__thread int public_tls_var = 42;
_Thread_local int c11_public_tls = 100;

/* Static TLS with internal linkage */
static __thread int static_tls_var = 7;
static _Thread_local int c11_static_tls = 77;

/* Weak TLS symbol */
__thread int weak_tls_var __attribute__((weak)) = 100;
__thread int weak_undefined_tls __attribute__((weak));

/* Common linkage TLS */
__thread int common_tls_var __attribute__((common));

/* TLS with visibility attributes */
__thread int hidden_tls_var __attribute__((visibility("hidden"))) = 200;
__thread int protected_tls_var __attribute__((visibility("protected"))) = 300;
__thread int internal_tls_var __attribute__((visibility("internal"))) = 400;

/* DLL import/export simulation */
#ifdef _WIN32
__declspec(dllexport) __thread int exported_tls_var = 500;
__declspec(dllimport) __thread int imported_tls_var;
#else
__thread int __attribute__((visibility("default"))) exported_tls_var = 500;
/* Simulate dllimport with extern and weak */
extern __thread int imported_tls_var __attribute__((weak));
#endif

/* TLS in different contexts */
struct ThreadLocalStruct {
    __thread int member_tls;
    _Thread_local long member_c11_tls;
};

/* Force TREE_USED by taking addresses */
void* get_tls_addresses(void) {
    static void* addresses[] = {
        &public_tls_var,
        &static_tls_var,
        &weak_tls_var,
        &common_tls_var,
        &hidden_tls_var,
        &protected_tls_var,
        &internal_tls_var,
        &exported_tls_var,
        &c11_public_tls,
        &c11_static_tls
    };
    return addresses[0];
}

/* Complex usage patterns */
__thread int* tls_pointer_var = &public_tls_var;

/* Volatile access to prevent optimization */
void volatile_access(void) {
    __thread int volatile_tls = 99;
    asm volatile("" : : "r"(&volatile_tls));
    asm volatile("" : : "r"(&public_tls_var));
}

/* Function with local static TLS */
void func_with_local_tls(void) {
    static __thread int local_static_tls = 123;
    local_static_tls++;
}

/* Array TLS */
__thread int tls_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/* Union with TLS */
union TLSUnion {
    __thread int tls_int;
    __thread float tls_float;
};

/* External TLS definition (for tls_uses.c) */
__thread int external_tls_var = 999;
__thread int external_common_var __attribute__((common));
