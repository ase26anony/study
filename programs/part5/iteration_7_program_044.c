/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* Trigger default case with unusual characters in type definitions */
#define ARRAY_DIM (1 << 2) /* Contains << operators */
#define ALIGN_SPEC __attribute__((aligned(16)))
#define PACKED_SPEC __attribute__((packed))

/* Macro expansions within type definitions */
#define FUNC_PTR_TYPEDEF(name, ret) \
    typedef ret (*name##_fp)(int, char**); \
    /* Embedded comment in macro */ \
    typedef ret (/**/ *name##_fp2)(void)

/* Use the macro with nested parentheses */
FUNC_PTR_TYPEDEF(complex, void);

/* Complex struct with all delimiter types mixed */
struct level1 {
    /* Nested anonymous union with attributes */
    union {
        int x;
        double y;
    } ALIGN_SPEC;
    
    /* Function pointer with complex arguments */
    void (*callback)(
        struct level1 *self,  /* Parameter with pointer */
        int arr[ARRAY_DIM],   /* Array parameter with macro */
        void (*nested_cb)(char)  /* Nested function pointer */
    );
    
    /* Bit-field with unusual size expression */
    unsigned int flags : (sizeof(int) * 8 - 1);
    
    /* Array of pointers to functions */
    int (*(*func_array[5])(float))[3];
};

/* Trigger default case with line continuation */
struct weird_struct {
    int value1; \
    /* The backslash continues the line */
    int value2;
    
    /* Attribute with parentheses inside struct */
    char data[10] ALIGN_SPEC;
    
    /* Nested struct with all delimiters */
    struct {
        /* Multi-dimensional array with computed size */
        int matrix[3][(2 + 1)];
        
        /* Union containing array of structs */
        union {
            struct {
                short a;
                long b;
            } items[5];
            
            /* Pointer to array of function pointers */
            void (*(*func_ptr_arr)[10])(int, ...);
        } nested_union;
        
        /* Complex declarator with parentheses, brackets, braces */
        enum { RED = 1, GREEN = 2, BLUE = 4 } colors : 3;
    } inner;
};

/* Typedef with deeply nested delimiters */
typedef struct level1** (*(*complex_type)(
    int param1, 
    /* Comment between parameters */
    char param2[][10],  /* 2D array parameter */
    struct { 
        int x; 
        union { 
            float f; 
            double d; 
        } u; 
    } param3  /* Anonymous struct parameter */
))[10];  /* Returns pointer to array of 10 pointers to pointer to struct level1 */

/* Union with GCC vector extension */
union vector_data {
    /* Vector type with attribute */
    int v4si __attribute__((vector_size(16)));
    
    /* Nested struct with bit-fields */
    struct {
        unsigned int a : 4;
        unsigned int b : 4;
        unsigned int c : (8 * sizeof(int) - 8);
    } bits;
    
    /* Function pointer returning pointer to array */
    float (*(*get_matrix)(void))[4][4];
};

/* Even more complex declaration mixing everything */
static volatile const struct level1* (*(*global_func)(
    /* Parameter with __attribute__ */
    int __attribute__((unused)) dummy,
    /* Array parameter with size from expression */
    char str[sizeof(struct level1) / 2],
    /* Function pointer parameter */
    void (*handler)(union vector_data*, enum {A,B,C})
))[] = {  /* Initializer braces */
    0, /* NULL pointer */
    /* More initializers could go here */
};

/* Struct with computed array size containing nested parentheses */
struct computed_array {
    /* Size expression with parentheses and operators */
    int data[(ARRAY_DIM * 2) + 1];
    
    /* Pointer to function returning pointer to array */
    int (*(*compute)(int n))[];
    
    /* Nested anonymous struct with attribute */
    struct {
        long long big_num;
        char small;
    } PACKED_SPEC inner_packed;
};

/* Macro that expands to contain unusual characters */
#define WEIRD_MACRO(x) [(x) + 1] /* Contains brackets */

struct macro_test {
    /* Use macro in array dimension */
    int array WEIRD_MACRO(5);
    
    /* Pointer with __attribute__ containing parentheses */
    void* ptr ALIGN_SPEC;
    
    /* Complex type with line continuation and comment */
    struct computed_array* \
        /* Comment between line continuation */ \
        next;
};

/* Function pointer type with ellipsis and attributes */
typedef int (__attribute__((cdecl)) *vararg_func)(
    const char *fmt,  /* String literal in comment: "test" */
    ...  /* Ellipsis parameter */
);

/* Final ultra-complex typedef using all features */
typedef union {
    struct level1* l1;
    struct weird_struct* ws;
    complex_type ct;
    vararg_func vf;
    
    /* Anonymous struct with nested everything */
    struct {
        /* Array of pointers to functions returning pointers to arrays */
        int (*(*callbacks[3])(float))[2];
        
        /* Nested union with bit-fields */
        union {
            unsigned char bytes[8];
            struct {
                unsigned int low : 16;
                unsigned int high : 16;
            } PACKED_SPEC;
        } data;
        
        /* Function pointer with __attribute__ */
        void (__attribute__((noreturn)) *fatal_error)(int);
    } handler;
} ultimate_type_t;

/* Trigger default case with numeric constants and operators */
enum {
    VALUE = (1 << 8) | 0x0F,  /* Contains | operator and hex constant */
    ANOTHER = sizeof(struct level1) * 2  /* Contains * operator */
};

/* Struct with attribute containing multiple sets of parentheses */
struct __attribute__((aligned(32), packed)) double_attr {
    char c;
    int i;
    long l;
};

/* Additional type to ensure multiple parsing passes */
typedef struct double_attr*(*factory_func)(int count, ...);

/* Empty main - file is for parsing only */
int main(void) {
    return 0;
}
