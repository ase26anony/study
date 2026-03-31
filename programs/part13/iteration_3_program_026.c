/* test_gengtype_coverage.h
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* ==================== MACRO EXPANSIONS WITH BRACKETS ==================== */

/* Macro that expands to include parentheses */
#define FUNC_PTR(T) T (*)(T)
#define ARRAY_DECL(T, N) T [N]
#define NESTED_FUNC_PTR(T) T (*(*)(T (*)(T)))(T)

/* Macro that expands to include braces (struct definition) */
#define DEFINE_PAIR(T) struct { T first; T second; }

/* ==================== COMPLEX TYPE DEFINITIONS ==================== */

/* Type 1: Struct with function pointer containing nested parentheses */
struct SignalHandler {
    /* Function returning function pointer with nested params */
    void (*signal(int sig, void (*handler)(int)))(int);
    
    /* Pointer to array of function pointers */
    int (*(*callback_array[5])(int))(void);
};

/* Type 2: Deeply nested function pointer types */
typedef int (*(*ComplexFuncPtr)(int (*(*)(char[10]))(double)))(float);

/* Type 3: Union with anonymous struct and bit-fields */
union DataContainer {
    struct {
        unsigned int flags : 4;
        unsigned int status : 2;
        unsigned int : 26;  /* Unnamed bit-field */
    } bits;
    
    struct {
        int (*processor)(int (*)(int[3][2]));
        long values[2][4];
    } ops;
    
    DEFINE_PAIR(double) pair;  /* Macro expansion */
};

/* Type 4: Multi-dimensional arrays with function pointers */
struct MatrixProcessor {
    /* 3D array of function pointers */
    int (*(*func_matrix[2][3][4])(int[2]))(void);
    
    /* Flexible array member with nested types */
    struct {
        int len;
        int data[];
    } flexible;
};

/* Type 5: Highly nested single declaration combining all bracket types */
struct UltimateType {
    /* Combination 1: Function pointer with array parameter */
    void (*(*setup)(int (*handlers[2])(char[10])))(int);
    
    /* Combination 2: Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 3;
            unsigned int c[2][2];
        } s1;
        
        struct {
            int (*func)(int (*(*)(int[5]))(void));
            long matrix[3][3];
        } s2;
    } u;
    
    /* Combination 3: Array of structs containing function pointers */
    struct {
        int id;
        char (*(*name_processor)(char (*)[10]))(int);
        double values[4][2];
    } processors[5];
    
    /* Attribute with double parentheses */
    int aligned_data[16] __attribute__((aligned(64)));
};

/* ==================== FUNCTION POINTERS WITH VARIED SIGNATURES ==================== */

/* Simple function pointer */
typedef int (*SimpleFunc)(int);

/* Pointer to function returning function pointer */
typedef int (*(*FuncReturningFunc)(int))(int);

/* Triple-nested function pointer */
typedef int (*(*(*TripleNestedFunc)(int (*(*)(int))(int)))(int))(int);

/* Function pointer with array parameter and function pointer parameter */
typedef void (*ComplexHandler)(int data[5][5], int (*callback)(int, int));

/* ==================== ATTRIBUTED DECLARATIONS ==================== */

/* Struct with multiple attributes */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b __attribute__((aligned(16)));
    double c;
};

/* Function declaration with attributes */
void process_data(struct UltimateType *data)
    __attribute__((nonnull(1), warn_unused_result));

/* Variable with attribute containing parentheses */
int important_array[100] 
    __attribute__((aligned(32), section(".critical_data")));

/* ==================== ADDITIONAL COMPLEX DECLARATIONS ==================== */

/* Mix of all bracket types in typedef */
typedef struct {
    int (*(*get_processor)(void))[5];
    union {
        struct {
            short x:4;
            short y:4;
            short z[2];
        } coord;
        long packed;
    } location;
    char (*strings[3])(int, char (*)[10]);
} MultiBracketType;

/* Declaration using macro expansions */
FUNC_PTR(int) *global_func_ptr;
ARRAY_DECL(FUNC_PTR(double), 3) func_array;

/* Nested macro expansion */
NESTED_FUNC_PTR(long) ultra_complex_func;

/* ==================== REAL-WORLD INSPIRED PATTERNS ==================== */

/* Similar to signal handler pattern found in Unix */
typedef void (*sighandler_t)(int);
struct sigaction {
    union {
        sighandler_t sa_handler;
        void (*sa_sigaction)(int, void *, void *);
    } __sigaction_handler;
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

/* Similar to X Window event handler */
typedef struct {
    int type;
    union {
        struct {
            int x, y;
            unsigned int width, height;
        } configure;
        struct {
            int (*filter)(int, int (*)(int[2][2]));
            unsigned long data[4];
        } custom;
    } event_data;
} XEvent;

/* ==================== EDGE CASES ==================== */

/* Empty braces */
struct EmptyStruct {};

/* Single element */
struct Singleton { int x; };

/* Just brackets, no content */
typedef int (*EmptyParamFunc)();

/* Multiple adjacent brackets */
int (*(*weird[2])())[3];

/* Deep nesting (10 levels) */
typedef int (*(*(*(*(*(*(*(*(*(*deep10)(int))(int))(int))(int))(int))(int))(int))(int))(int);

#endif /* TEST_GENGTYPE_COVERAGE_H */
