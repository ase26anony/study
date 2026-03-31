/* tls_emulation_test.c
 * Tests GCC's emulated TLS initialization by creating thread-local variables
 * with various attributes that trigger the uncovered lines in tree-emutls.cc
 */

/* Force emulated TLS mode */
#pragma GCC tls_model emulated

/* Global volatile array to prevent optimization */
volatile int g_results[10];
volatile void* g_addresses[10];

/* Helper function to use TLS variables without inlining */
__attribute__((noinline, used, retain))
static void use_tls_variables(int idx) {
    /* Take addresses and perform operations to ensure TLS variables are used */
    g_addresses[idx++] = (void*)&tls_weak;
    g_addresses[idx++] = (void*)&tls_hidden;
    g_addresses[idx++] = (void*)&tls_common;
    g_addresses[idx++] = (void*)&tls_external;
    g_addresses[idx++] = (void*)&tls_imported;
    g_addresses[idx++] = (void*)&tls_preserved;
    
    /* Perform some computations */
    g_results[0] = tls_weak + 1;
    g_results[1] = tls_hidden * 2;
    g_results[2] = tls_common - 3;
    g_results[3] = tls_external / 4;
    g_results[4] = tls_imported % 5;
    g_results[5] = tls_preserved | 0xFF;
}

/* TLS variable with weak attribute - triggers DECL_WEAK copying */
__thread int tls_weak __attribute__((weak)) = 100;

/* TLS with explicit visibility - triggers DECL_VISIBILITY and DECL_VISIBILITY_SPECIFIED */
__thread int tls_hidden __attribute__((visibility("hidden"))) = 200;

/* Common TLS variable (tentative definition) - triggers DECL_COMMON */
__thread int tls_common;

/* External TLS declaration - triggers DECL_EXTERNAL and TREE_PUBLIC */
extern __thread int tls_external;

/* DLL Import TLS (Windows-specific) - triggers DECL_DLLIMPORT_P */
#ifdef _WIN32
__declspec(dllimport) __thread int tls_imported;
#else
/* Simulate similar behavior on non-Windows with weak external */
extern __thread int tls_imported __attribute__((weak));
#endif

/* Preserved TLS variable - may influence DECL_PRESERVE_P */
__thread int tls_preserved __attribute__((used)) = 500;

/* Another TLS variable inside a function to test DECL_CONTEXT */
static void function_with_tls(void) {
    static __thread int local_tls __attribute__((used)) = 600;
    g_addresses[6] = (void*)&local_tls;
    g_results[6] = local_tls;
}

/* C++ style test (compile with g++) */
#ifdef __cplusplus
namespace {
    /* TLS in anonymous namespace */
    __thread int tls_in_namespace __attribute__((used)) = 700;
}

class TestClass {
public:
    /* Static thread-local member */
    static __thread int tls_member;
    __thread int instance_tls;  /* Non-static thread-local */
};
__thread int TestClass::tls_member = 800;
#endif

/* Main function that uses all TLS variables */
int main(void) {
    int i, checksum = 0;
    
    /* Initialize common TLS variable */
    tls_common = 300;
    
    /* Use the external TLS variable (defined in another file if split) */
    /* For single-file test, we'll define it here but mark as extern */
    __thread int tls_external = 400;
    
    /* Initialize imported TLS variable */
    #ifdef _WIN32
    /* Would normally come from DLL */
    #else
    __thread int tls_imported = 250;
    #endif
    
    /* Call helper function multiple times to ensure TLS is accessed */
    for (i = 0; i < 3; i++) {
        use_tls_variables(i * 2);
        function_with_tls();
    }
    
    #ifdef __cplusplus
    /* Use C++ TLS variables */
    TestClass::tls_member = 850;
    TestClass obj;
    obj.instance_tls = 900;
    g_addresses[7] = (void*)&TestClass::tls_member;
    g_addresses[8] = (void*)&obj.instance_tls;
    g_addresses[9] = (void*)&tls_in_namespace;
    g_results[7] = TestClass::tls_member;
    g_results[8] = obj.instance_tls;
    g_results[9] = tls_in_namespace;
    #endif
    
    /* Force comparison of TLS addresses to ensure they're referenced */
    if (&tls_weak != &tls_hidden) {
        checksum++;
    }
    if (&tls_common != &tls_external) {
        checksum++;
    }
    
    /* Compute checksum from results to prevent optimization */
    for (i = 0; i < 10; i++) {
        checksum += g_results[i];
        checksum += (int)((long)g_addresses[i] & 0xFF);
    }
    
    /* Print something to ensure execution */
    printf("TLS test checksum: %d\n", checksum);
    printf("TLS addresses differ: %s\n", 
           (&tls_weak != &tls_hidden) ? "yes" : "no");
    
    return checksum != 0 ? 0 : 1;
}

/* Additional file for multi-translation unit test (compile separately) */
#ifdef MULTI_FILE_TEST
/* tls_external_def.c - defines external TLS variable */
__thread int tls_external = 400;

/* For Windows DLL export */
#ifdef _WIN32
__declspec(dllexport) __thread int tls_imported = 250;
#endif
#endif
