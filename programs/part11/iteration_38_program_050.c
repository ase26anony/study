/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void *ptr_t;

/* Macros to add nesting depth */
#define NESTED_EXPR ( (10) + (20) )
#define COMPLEX_SIZE (sizeof(int) * (2 + 3))
#define ARRAY_DIM ( (5) * (6) )
#define BRACE_BLOCK { int x = 1; int y = 2; }
#define CALLBACK_FUNC(func) (void (*)(int))func

/* Case 1: Parentheses - struct with complex length expression */
struct s1 GTY((length("(2 + 3) * sizeof(int)"))) {
    int data[];
};

/* More parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Even more nested parentheses */
struct s3 GTY((user, desc("NESTED_EXPR"), length("COMPLEX_SIZE"))) {
    int arr[];
};

/* Case 2: Brackets - array types with complex dimensions */
typedef int arr_type1 GTY((length("N")))[ (2 * 3) ];

struct s4 GTY(()) {
    int matrix GTY((length("rows * cols")))[ (10 + 5) ][ (20 - 3) ];
    int rows;
    int cols;
};

/* Pointer to array with brackets */
typedef int (*ptr_to_array GTY((skip)))[ ARRAY_DIM ];

/* Multi-dimensional with nested brackets */
struct s5 GTY((tag("ARRAY_STRUCT"))) {
    int (*complex_arr GTY((length("dim1 * dim2"))))[ (1 << 3) ][ (sizeof(double) / sizeof(int)) ];
    int dim1;
    int dim2;
};

/* Case 3: Braces - structures with nested definitions */
/* First, a structure with nested struct definition */
struct outer GTY((user)) {
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y;
    } nested;
    int value;
};

/* Union with nested union definition */
union u1 GTY(()) {
    struct {
        int a;
        int b;
    } s;
    int i;
    char arr[ 10 + (5) ];
};

/* More complex nested braces */
struct s6 GTY((for_user)) {
    union {
        struct {
            int x;
            int y;
        } point;
        int coordinates[2];
    } data;
    int type;
};

/* Case 4: Function pointers with parentheses */
typedef void (*callback_func GTY((callback)))(int (*)(int [ (4) ]));

/* Complex function pointer type */
typedef int (*complex_func_ptr GTY((skip)))(
    struct s1 *,
    int (*handler)(int, char **),
    void (*)(int [ (2 + 2) ])
);

/* Structure with function pointer member */
struct s7 GTY((chain_next("next"))) {
    struct s7 *next;
    void (*operation GTY((callback)))(
        int param1,
        char *param2[ (sizeof(int)) ]
    );
    int result;
};

/* Case 5: Mixed delimiters - all three types together */
struct mixed GTY((desc("Mixed({test}, [array], (func))"))) {
    /* Parentheses in array dimension */
    int arr1[ (NESTED_EXPR) ];
    
    /* Brackets in structure */
    struct {
        int x;
        int y;
    } point;
    
    /* Braces in union */
    union {
        int i;
        char c;
    } data;
    
    /* Function pointer with parentheses */
    void (*func GTY((callback)))(int, char *);
};

/* Case 6: Template-like macro expansion with delimiters */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name##_t GTY((length(#size))) { \
        int data[size]; \
        int count; \
    }

/* Use the macro with nested expressions */
DEFINE_GTY_STRUCT(my_struct, (10 + 5));

/* Case 7: Forward declarations with GTY annotations */
struct forward_decl GTY((user));

/* Later definition with complex annotations */
struct forward_decl GTY((chain_next("next"))) {
    struct forward_decl *next;
    int values[ (sizeof(int) > 4) ? 8 : 4 ];
    struct {
        int x;
        int y;
    } point;
};

/* Case 8: Enum with GTY marker (though enums don't usually need GC) */
enum my_enum GTY((tag("ENUM_TYPE"))) {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1),
    VALUE3 = (1 << 2)
};

/* Case 9: Typedef with nested type expression */
typedef struct {
    int (*get_value GTY((callback)))(void);
    void (*set_value GTY((callback)))(int);
} interface_t GTY((tag("INTERFACE")));

/* Case 10: Array of structures with GTY */
struct element GTY((user)) {
    int id;
    char name[ (32) ];
};

typedef struct element element_array_t GTY((length("count")))[ (100) ];

/* Additional test: Very deeply nested parentheses */
struct deep_nest GTY((desc("((((((deep))))))"), length("((((1 + 2) * 3) - 4) / 5)"))) {
    int value;
    char buffer[ (((16) * (2)) + (8)) ];
};

#endif /* TEST_PARSE_H */
