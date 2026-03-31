/* test_parse.h - Complex GTY annotations to exercise delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to add nesting depth for delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = 2 }
#define NESTED_MACRO(x) ( (x) * ( (x) + 1 ) )

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Array with length containing complex parenthesized expression */
    int data GTY((length("( (sizeof(int) * (NESTED_PAREN_EXPR) ) / sizeof(int) )")))[];
};

/* Case 2: Parentheses - callback with complex function signature */
typedef void (*complex_fn_ptr GTY((callback)))(
    int (*)(int, char), 
    void (*)(int (*)(int [ (4) ]))
);

/* Case 3: Brackets - array type with nested bracket expressions */
struct s2 GTY(()) {
    /* Array dimension with nested brackets and parentheses */
    int matrix GTY((length("( (5) * (3) )")))[ (2) * (3) ][ (4) + (1) ];
    
    /* Pointer to array with complex dimension */
    int (*ptr_to_array GTY((skip)))[ (sizeof(int) == 4) ? 10 : 20 ];
};

/* Case 4: Brackets - multi-dimensional array with macro expansion */
union u1 GTY((desc("1"))) {
    int i;
    /* Using macro that expands to bracketed expression */
    char multi_array[10][ NESTED_PAREN_EXPR ][20];
    
    /* Array with conditional dimension */
    double conditional_array[ (sizeof(void*) == 8) ? 16 : 8 ];
};

/* Case 5: Braces - nested structure definition within GTY */
struct outer GTY((user)) {
    int id;
    
    /* Nested struct definition (contains braces) */
    struct inner GTY((tag("SPECIAL"))) {
        int x;
        int y;
        /* Array with initializer-like syntax in GTY */
        int values GTY((length("( (3) * (2) )")))[6];
    } nested;
    
    /* Union with nested struct */
    union {
        struct inner GTY((chain_next("next"))) inner_data;
        char buffer[100];
    } variant;
};

/* Case 6: Braces - GTY with nested initializer-like syntax */
struct s3 GTY(( 
    chain_next("next"),
    chain_prev("prev"),
    user
)) {
    /* Flexible array member with complex length */
    unsigned char bytes GTY((length("( (256) * (sizeof(int)) )")))[];
    
    /* Inline struct with initializer */
    struct {
        int a;
        int b GTY((skip));
    } inline_struct;
};

/* Case 7: Mixed delimiters - complex type with all three */
typedef struct mixed_types GTY((user)) {
    /* Function pointer array with nested signatures */
    void (*callbacks[ (2) + (3) ] GTY((length("5"))))(
        int, 
        char (*)[ (10) ],
        struct mixed_types *
    );
    
    /* Nested array of structs */
    struct {
        int index;
        char name[ (32) ];
    } entries GTY((length("( (10) * (2) )")))[20];
    
    /* Union with array and nested struct */
    union {
        int numbers[ ( (5) * (4) ) ];
        struct {
            double x;
            double y;
        } point;
    } data;
} mixed_types_t;

/* Case 8: Forward declaration with GTY containing parentheses */
struct forward_decl GTY((user, chain_next("next")));
struct forward_decl {
    int value;
    struct forward_decl *next;
};

/* Case 9: Template-like macro with all delimiters */
#define DEFINE_GTY_STRUCT(name, size) \
    struct name GTY((user)) { \
        int id; \
        char buffer[size]; \
        struct { \
            int x; \
            int y; \
        } coord; \
    }

/* Instantiate the macro */
DEFINE_GTY_STRUCT(macro_struct, ( (32) + (16) ));

/* Case 10: Extremely nested expressions */
struct extreme_nesting GTY((user)) {
    /* Array with deeply nested dimension calculation */
    int extreme_array GTY((length(
        "( ( (sizeof(int) * ( (1 << 5) - (16) ) ) + "
        "( ( (10) * (20) ) / (2) ) ) / sizeof(int) )"
    )))[ 
        ( ( (10) * ( (5) + (3) ) ) - ( (20) / (2) ) ) 
    ];
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func_ptr GTY((skip)))(
        int, 
        int (*)(int[ ( (5) * (2) ) ])
    ))[ ( (3) * (4) ) ];
};

/* Case 11: GTY options with string literals containing delimiters */
struct with_strings GTY((
    user,
    tag("struct_with_(parentheses)_[brackets]_{braces}")
)) {
    int value;
    
    /* Length expression with string containing delimiters */
    char description GTY((length(
        "sizeof(\"(string with (nested) [brackets] {braces})\")"
    )))[];
};

/* Case 12: Conditional compilation with GTY */
#ifdef SPECIAL_FEATURE
struct conditional GTY((user, desc("SPECIAL"))) {
    int special_data[ ( (100) + (50) ) ];
};
#else
struct conditional GTY((user)) {
    int normal_data[ ( (50) + (25) ) ];
};
#endif

/* Case 13: Typedef with GTY and complex declarators */
typedef struct node GTY((chain_next("next"), chain_prev("prev"))) {
    int data;
    struct node *next;
    struct node *prev;
    
    /* Array of function pointers */
    void (*handlers[ (4) ] GTY((length("4"))))(void);
} node_t, *node_ptr_t;

/* Case 14: Anonymous union within struct with GTY */
struct with_anon_union GTY((user)) {
    int type;
    
    union {
        int int_value;
        double double_value;
        struct {
            char str[ (32) ];
            int len;
        } string_data;
    } value;
};

/* Final case: Multiple GTY annotations on same type */
struct multi_gty GTY((user)) GTY((desc("MULTI"))) {
    /* This should trigger multiple delimiter parsing passes */
    int (*func_array GTY((length("( (2) * (3) )")))[6])(
        int[ ( (5) + (1) ) ],
        char (*)[ (10) ]
    );
};

#endif /* TEST_PARSE_H */
