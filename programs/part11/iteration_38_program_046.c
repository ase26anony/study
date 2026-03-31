/* test_parse.h - Complex GTY annotations to exercise balanced delimiter parsing */

#ifndef TEST_PARSE_H
#define TEST_PARSE_H

/* Basic typedefs for dependencies */
typedef int my_int;
typedef char my_char;
typedef void (*generic_callback)(void);

/* Macros to introduce nested delimiters */
#define NESTED_PAREN_EXPR ( (sizeof(int) * (2 + 3)) )
#define BRACKET_EXPR [ (2 * 3) + (4 / 2) ]
#define BRACE_INITIALIZER { .x = 1, .y = {2, 3} }
#define COMPLEX_MACRO(x) ((x) + ( (x) * (x) ))

/* Case 1: Parentheses - struct with GTY length containing nested parentheses */
struct s1 GTY((user)) {
    int count;
    /* Flexible array member with complex length expression */
    int data GTY((length("(sizeof(int) * (2 + 3))")))[];
};

/* Another parentheses example with multiple parenthesized groups */
struct s2 GTY((chain_next("next"), chain_prev("prev"))) {
    struct s2 *next;
    struct s2 *prev;
    int value;
};

/* Case 2: Brackets - array types with complex dimension expressions */
struct s3 GTY(()) {
    /* Array with dimension containing parentheses */
    int arr1 GTY((length("N")))[ (2 * 3) ];
    
    /* Pointer to array with nested brackets */
    int (*arr2 GTY((length("M"))))[ (4 + 2) ];
    
    /* Multi-dimensional array */
    int matrix GTY((length("ROWS * COLS")))[ (3) ][ (4) ];
};

/* typedef with function pointer containing array parameters */
typedef void (*complex_func_ptr GTY((callback)))(
    int (*)(int arr[ (sizeof(int) > 4) ? 8 : 4 ]),
    char buffer[ (16 + (8/2)) ]
);

/* Case 3: Braces - structures with nested struct definitions */
struct outer GTY((user)) {
    int id;
    
    /* Nested struct definition (contains braces) */
    struct inner GTY((tag("NESTED"))) {
        int x;
        int y;
        int z;
    } nested_struct;
    
    /* Union with array */
    union {
        int i;
        char str[ (10) + (5) ];
    } data GTY((skip));
};

/* Union with GTY and complex array */
union u1 GTY(()) {
    int i;
    long l;
    /* Array with macro-expanded size */
    char arr[ 10 + (5) ];
    
    /* Nested anonymous struct */
    struct {
        float f;
        double d;
    } GTY((desc("1"))) fs;
};

/* Forward declared struct with GTY annotation containing expression */
struct forward_decl GTY((user));

/* Complete definition with nested parentheses in tag */
struct forward_decl GTY((tag("TAG_" "VALUE"))) {
    struct forward_decl *next;
    int value GTY((length("(1 << 5) + (3 * 2)")));
};

/* Enum with GTY marker (though enums don't typically need GC) */
enum my_enum GTY((tag("ENUM_TYPE"))) {
    VALUE_A = (1 << 0),
    VALUE_B = (1 << 1),
    VALUE_C = (1 << 2) | (1 << 3)
};

/* Complex typedef with nested parentheses in array dimensions */
typedef struct {
    int count;
    /* Using macro with parentheses in array dimension */
    int items GTY((length("count")))[ NESTED_PAREN_EXPR ];
} array_container GTY((user));

/* Function pointer type with complex signature */
typedef int (*comparator_func GTY((callback)))(
    const void *a,
    const void *b,
    void *user_data GTY((skip))
);

/* Structure with callback marker and nested type */
struct tree_node GTY((chain_next("next"), chain_prev("prev"))) {
    struct tree_node *parent;
    struct tree_node *children GTY((length("child_count")))[];
    struct tree_node *next;
    struct tree_node *prev;
    int child_count;
    
    /* Nested union with array */
    union {
        int int_value;
        char *string_value GTY((length("strlen(string_value) + 1")));
        double double_value;
    } data;
};

/* Macro-based type definition with GTY */
#define DEFINE_BUFFER_TYPE(name, type) \
    struct name ## _buffer GTY((user)) { \
        type *data GTY((length("capacity"))); \
        size_t size; \
        size_t capacity; \
    }

/* Instantiate the macro-generated type */
DEFINE_BUFFER_TYPE(int, int);
DEFINE_BUFFER_TYPE(char, char);

/* Template-like structure with GTY options containing all delimiters */
struct template_like GTY((user)) {
    /* Parentheses in length expression */
    void *buffer GTY((length("(alloc_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)")));
    
    /* Array with brackets containing expression */
    int offsets[ (256 / sizeof(int)) ];
    
    /* Nested struct (braces) */
    struct {
        int x;
        int y;
        int z[ (16) ];
    } GTY((skip)) coordinates;
    
    size_t alloc_size;
};

/* Final test: structure with all delimiter types in one GTY annotation */
struct all_delimiters GTY((
    user,
    desc("%1.tag"),
    length("(sizeof(struct all_delimiters) + (extra * 2))")
)) {
    int tag;
    int extra;
    
    /* Array with complex dimension */
    char padding[ (sizeof(void*) * 2) ];
    
    /* Nested anonymous union */
    union {
        int as_int;
        struct {
            short a;
            short b;
        } as_shorts;
    } value;
};

#endif /* TEST_PARSE_H */
