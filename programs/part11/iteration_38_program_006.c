/* test_parse.h - Complex GTY annotations to test balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void* my_ptr;

/* Macros to introduce nested delimiter layers */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define COMPLEX_BRACKET_EXPR [ (10) + (20) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define CALLBACK_MACRO GTY((callback))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((length("(sizeof(int) * (2 + 3))"))) {
    int data[];
};

/* More complex parentheses nesting */
struct s2 GTY((chain_next("next"), chain_prev("prev"), user)) {
    struct s2 *next;
    struct s2 *prev;
    int value GTY((skip, length("((NESTED_PAREN_EXPR) + 5)")));
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY((length("N"))) {
    int arr[ (2 * 3) + sizeof(int) ];
};

/* Pointer to array with nested brackets */
typedef int (*array_ptr GTY((tag("ARRAY"))))[ (4) + (5) ];

/* Multi-dimensional array with brackets */
struct s4 GTY(()) {
    int matrix[ (2) ][ (3) * (4) ];
    char buffer[ sizeof(struct { int x; char y; }) ];
};

/* Case 3: Braces - structure with nested struct definition inside GTY */
struct s5 GTY((user)) {
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y[2];
    } nested;
};

/* Union with brace-enclosed initializer in macro expansion */
union u1 GTY((desc("(%1.u_type)"))) {
    int i;
    char c;
    struct {
        int type;
        void* data;
    } s;
};

/* Function pointer with complex argument list containing brackets */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int [ (sizeof(int) * 2) ]),
    struct { int x; int y; } point
);

/* GTY with deeply nested parentheses in options */
struct s6 GTY((chain_next("next"),
               chain_prev("prev"),
               length("((((1) + (2)) * (3)) / (4))"))) {
    struct s6 *next;
    struct s6 *prev;
    int data[];
};

/* Array of function pointers with GTY */
typedef int (*func_array GTY((length("5"))))[3](
    int,
    char[ (10) ]
);

/* Structure containing all delimiter types */
struct s7 GTY((user, skip("skip_func"))) {
    /* Parentheses in bitfield */
    unsigned int flags : (sizeof(int) * 8 - 1);
    
    /* Brackets in array with expression */
    int values[ (1 << 3) | (1 << 2) ];
    
    /* Nested structure (braces) */
    struct {
        int x GTY((tag("X")));
        int y;
    } coord;
    
    /* Pointer with cast expression */
    void* ptr GTY((skip));
};

/* Forward declaration with GTY containing parentheses */
struct s8 GTY((user));

/* Complete definition with complex GTY options */
struct s8 {
    int id;
    char* name GTY((length("(strlen(name) + 1)")));
    struct s8* children GTY((length("child_count")));
};

/* Union with array containing bracket expressions */
union u2 GTY((desc("(%1.type)"))) {
    int type;
    struct {
        int count;
        float data[ (int)(sizeof(float) * 10) ];
    } floats;
    struct {
        char* str;
        int length;
    } string;
};

/* Macro that expands to GTY with parentheses */
#define GTY_USER(user_data) GTY((user, tag(user_data)))

/* Use of macro with nested parentheses */
struct s9 GTY_USER("(DATA_STRUCT)") {
    int value;
    void* extra GTY((skip, length("extra_size")));
};

/* Complex callback with nested types */
typedef struct callback_data {
    int (*handler GTY((callback)))(void*, int);
    void* context;
} callback_data_t GTY((tag("CALLBACK")));

/* Final test: All delimiters in one GTY annotation (though not all may be valid) */
struct s10 GTY((chain_next("n"), chain_prev("p"), length("count"))) {
    struct s10* n;
    struct s10* p;
    int count;
    /* Attempt to include brace-like syntax in string */
    char* desc GTY((length("(sizeof(\"{test}\"))")));
};

#endif /* TEST_PARSE_H */
