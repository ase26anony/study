/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* ==================== MACRO EXPANSIONS WITH BRACKETS ==================== */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T[n]
#define NESTED_PTR(T) T (*(*)(T (*)(T)))(T)
#define COMPLEX_MACRO(T) struct { T (*func)(T (*)[2]); T arr[3]; }

/* ==================== FUNCTION POINTERS WITH NESTED PARENTHESES ==================== */

/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*func_returning_func_ptr)(float))(double);

/* Deeply nested function pointers */
typedef void (*(*(*deep_nested_func)(int (*(*)(char*))(long)))(short))(int);

/* Function pointer with array parameter */
typedef void (*func_with_array_param)(int matrix[3][4]);

/* Function pointer taking function pointer as parameter */
typedef int (*comparator)(int, int);
typedef void (*sorter)(int*, int, comparator);

/* ==================== COMPLEX STRUCTS WITH ALL BRACKET TYPES ==================== */

/* Struct combining all bracket types in one declaration */
struct MasterStruct {
    /* Parentheses: function pointer with complex signature */
    void (*(*signal_handler)(int sig, void (*callback)(int)))(int);
    
    /* Brackets: multi-dimensional arrays */
    int multi_dim_array[2][3][4];
    
    /* Braces: nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int :4;  /* Unnamed bit-field */
            unsigned int value:8;
        } bits;
        unsigned int raw;
    } bitfield_container;
    
    /* Mixed: array of function pointers */
    int (*func_array[5])(int (*)(char[10]), float);
    
    /* Flexible array member */
    int flexible_array[];
};

/* ==================== UNIONS WITH COMPLEX NESTING ==================== */

/* Union containing struct with all bracket types */
union ComplexUnion {
    struct {
        /* Function pointer with attributes */
        void (__attribute__((const)) *const_func)(int) __attribute__((aligned(16)));
        
        /* Array of pointers to functions returning pointers */
        char* (*(*string_funcs[3])(void))[2];
        
        /* Nested struct with bit-fields */
        struct {
            unsigned int a:2;
            unsigned int b:2;
            unsigned int c:4;
        } nested_bits;
    } s;
    
    /* Anonymous struct inside union */
    struct {
        double matrix[2][2];
        void (*ops[2])(double[2][2]);
    };
    
    /* Simple member */
    long long raw_data;
};

/* ==================== TYPEDEFS WITH MACRO EXPANSIONS ==================== */

/* Using macro that expands to include parentheses */
typedef PTR_FUNC(int) int_func_ptr_t;

/* Using macro that creates array declaration */
typedef ARRAY_DECL(int_func_ptr_t*, 4) func_ptr_array_t;

/* Complex macro expansion */
typedef COMPLEX_MACRO(double) complex_macro_struct_t;

/* ==================== ATTRIBUTES WITH PARENTHESES ==================== */

/* Struct with multiple attributes */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b __attribute__((aligned(16)));
    double c;
} __attribute__((deprecated));

/* Variable with attribute containing parentheses */
int global_array[10] __attribute__((aligned(32), section(".data")));

/* Function declaration with attributes */
void __attribute__((noreturn, format(printf, 1, 2))) 
log_error(const char* fmt, ...) __attribute__((weak));

/* ==================== EXTREME NESTING COMBINATION ==================== */

/* The ultimate test: all bracket types deeply nested */
typedef struct {
    /* Level 1: Array */
    struct {
        /* Level 2: Function pointer */
        int (*(*level2_func)(struct {
            /* Level 3: Union with bit-fields */
            union {
                struct {
                    unsigned int x:1;
                    unsigned int y:1;
                } __attribute__((packed));
                char raw;
            } level3_union;
            
            /* Level 3: Array in struct */
            int level3_array[2][2];
        }* param))[3];
        
        /* Level 2: Another struct */
        struct {
            /* Level 3: Pointer to array */
            void (*(*level3_ptr_to_array)[5])(int);
            
            /* Level 3: Anonymous union */
            union {
                long a;
                double b[2];
            };
        } level2_nested;
    } level1_array[2];
    
    /* Direct member with all brackets */
    void (*(*direct_member)(int (*)(char[10])))[2] __attribute__((aligned(64)));
} UltimateNestingStruct;

/* ==================== ADDITIONAL EDGE CASES ==================== */

/* Empty struct (tests braces with minimal content) */
struct EmptyStruct {};

/* Struct with only bit-fields */
struct BitFieldOnly {
    unsigned int a:1;
    unsigned int b:2;
    unsigned int c:3;
    unsigned int :0;  /* Force alignment */
    unsigned int d:4;
};

/* Array of structs containing function pointers */
struct Element {
    int (*operation)(int, int);
    char name[20];
};

struct Element element_array[] = {
    {NULL, "add"},
    {NULL, "subtract"},
    {NULL, "multiply"}
};

/* Forward declaration to test incomplete types */
struct ForwardDeclared;
typedef struct ForwardDeclared* Handle;
struct ForwardDeclared {
    Handle next;
    int data;
};

/* ==================== EXTERNAL DECLARATIONS ==================== */

/* Declaration using all bracket types in one go */
extern struct MasterStruct* (*(*external_table[10])(int))[5];

/* Complex external function declaration */
extern int (*(*register_callback(
    void (*(*callback_provider)(int))(void)
))(int, char*))[3];

#endif /* TEST_GENGTYPE_COVERAGE_H */
