/* Test TLS variables with various attributes to trigger EMUTLS attribute copying */

/* Force EMUTLS by using a target without native TLS support */
/* Compile with: -O2 -march=armv5te -ftls-model=emulated -fPIC */

/* Public TLS variable with explicit visibility */
__attribute__((visibility("default")))
__thread int public_tls_default = 42;

/* Hidden visibility TLS variable */
__attribute__((visibility("hidden")))
__thread int hidden_tls = 100;

/* Weak TLS variable */
__attribute__((weak))
__thread int weak_tls_var;

/* Used attribute (should set DECL_PRESERVE_P) */
__attribute__((used))
__thread int preserved_tls;

/* Common linkage TLS (no initializer) */
__thread int common_tls;

/* Static (non-public) TLS variable */
static __thread int static_tls = 7;

/* Function-scoped TLS variable */
void use_function_tls(void) {
    __thread int func_tls = 99;
    func_tls += 1;  /* Ensure TREE_USED is set */
}

/* External TLS declaration (will be defined in another file) */
extern __thread int external_tls;

/* Weak external TLS */
extern __thread int weak_external_tls __attribute__((weak));

/* DLL import simulation (for Windows targets) */
#ifdef _WIN32
__declspec(dllimport) __thread int imported_tls;
#else
/* Simulate with attribute on non-Windows */
__attribute__((dllimport)) __thread int imported_tls;
#endif

/* Reference all TLS variables to ensure TREE_USED is set */
void reference_all_tls(void) {
    public_tls_default += 1;
    hidden_tls *= 2;
    if (&weak_tls_var) weak_tls_var = 5;
    preserved_tls = 10;
    common_tls = 20;
    static_tls = 30;
    
    /* Use external TLS */
    external_tls = 40;
    if (&weak_external_tls) weak_external_tls = 50;
    
    /* Use imported TLS */
    if (&imported_tls) imported_tls = 60;
    
    use_function_tls();
}

/* Main function that uses all TLS variables */
int main(void) {
    reference_all_tls();
    return 0;
}
