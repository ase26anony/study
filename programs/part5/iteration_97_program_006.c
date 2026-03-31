/* complex_types.h - Header with deeply nested balanced symbols */

#ifndef COMPLEX_TYPES_H
#define COMPLEX_TYPES_H

/* ==================== MACROS THAT EXPAND TO BALANCED SYMBOLS ==================== */

/* Macro for function pointer type */
#define PTR_TO_FUNC(T) T (*)(T, T)

/* Macro for array declaration */
#define MAKE_ARRAY(T, sz) T arr[sz]

/* Macro chain that produces balanced braces */
#define OPEN_BRACE {
#define CLOSE_BRACE }

/* Macro that expands to nested parentheses */
#define NESTED_PARENS(x) ((((x))))

/* Macro with multiple balanced symbols */
#define COMPLEX_MACRO int (*func_ptr)(int, int) = NULL; \
                     int arr[2][3] = {{1,2,3},{4,5,6}}; \
                     struct { int a; char b; } s = {0, 'x'};

/* ==================== TYPEDEF CHAINS WITH NESTED GROUPING ==================== */

/* Basic function pointer typedef */
typedef int (*cmp_fn)(int, int);

/* Typedef that references another typedef */
typedef cmp_fn (*factory_fn)(void);

/* Even more complex typedef chain */
typedef factory_fn (*factory_registry[10])(cmp_fn, cmp_fn);

/* Function returning complex typedef */
factory_fn get_factory(void);

/* Typedef with deeply nested parentheses */
typedef void (*(*signal_handler)(int, void (*(*)(int))(int)))(int);

/* ==================== COMPLEX TYPE DECLARATIONS ==================== */

/* Function pointer with nested parameter lists */
void (*signal(int sig, void (*func)(int)))(int);

/* Array with multiple dimensions and nested initializers */
int matrix[3][2][2] = {
    {{1, 2}, {3, 4}},
    {{5, 6}, {7, 8}},
    {{9, 10}, {11, 12}}
};

/* Structure with nested anonymous structures and bit-fields */
struct OuterStruct {
    union {
        struct {
            int a:4;
            int b:4;
            int c:8;
        } bits;
        struct {
            long x;
            long y;
        } coords;
        char data[4];
    } inner_union;
    
    struct {
        int (*compare)(int, int);
        void (*print)(void);
    } funcs;
    
    int d[2][3];
};

/* Union with nested structures */
union ComplexUnion {
    struct {
        int (*callback)(int, char**, char**);
        void* ptr;
    } s1;
    struct {
        int matrix[2][2];
        char (*strings[5])(void);
    } s2;
};

/* ==================== FUNCTION DECLARATIONS WITH COMMENTS IN BALANCED SEQUENCES ==================== */

/* Function with comments between parameters */
void process_data(
    int input,          /* First parameter */
    char* buffer,       /* Second parameter */
    void (*callback)(   /* Callback function */
        int status,     /* Status code */
        char* result    /* Result data */
    )                   /* End of callback */
);

/* Function returning function pointer */
int (*(*get_operation(char op))(int, int))(int, int);

#endif /* COMPLEX_TYPES_H */
