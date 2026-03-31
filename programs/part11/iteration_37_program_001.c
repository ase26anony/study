/* test-gtype.h - Comprehensive type definitions for gengtype coverage testing
 * This file contains examples of all TYPE_* categories to ensure full coverage
 * of the switch statement in gengtype.cc lines 182-213.
 */

#ifndef TEST_GTYPE_H
#define TEST_GTYPE_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* TYPE_SCALAR: Basic scalar types */
typedef int my_scalar_type GTY(());
typedef unsigned long scalar_ulong GTY(());
typedef double scalar_double GTY(());
typedef _Complex float scalar_complex GTY(());

/* TYPE_STRING: String pointer types with (string) marker */
typedef const char *my_string_type GTY((string));
typedef char *mutable_string GTY((string));
typedef const char *const constant_string GTY((string));

/* TYPE_STRUCT: Plain C structures marked for GC */
struct my_base_struct GTY(()) {
    int field1;
    my_scalar_type field2;
    my_string_type field3;
};

/* Nested structure to test deeper traversal */
struct nested_struct GTY(()) {
    struct my_base_struct base GTY(());
    int extra_field;
};

/* TYPE_USER_STRUCT: User-defined structure type 
 * Often triggered by structures defined in separate modules
 * or with special GTY options
 */
struct GTY((user)) user_defined_struct {
    int user_data;
    void *user_pointer GTY((skip));
};

/* Another user struct pattern - defined with type tag */
struct GTY((tag("USER"))) tagged_user_struct {
    long tag_id;
    struct tagged_user_struct *next GTY((chain_next));
};

/* TYPE_UNION: Union types marked with GTY */
union my_union_type GTY(()) {
    int as_int;
    double as_double;
    void *as_pointer GTY((ptr));
    my_string_type as_string;
};

/* Tagged union variant */
union GTY((desc("%0.type"))) tagged_union {
    int type;
    struct {
        int type;
        int data;
    } int_data;
    struct {
        int type;
        double data;
    } double_data;
};

/* TYPE_POINTER: Pointer types with various GTY options */
typedef struct opaque_struct *opaque_pointer GTY((ptr));
typedef void *generic_pointer GTY((ptr));
typedef const struct my_base_struct *const_struct_ptr GTY((ptr));

/* Forward declaration for pointer to incomplete type */
struct incomplete_type;
typedef struct incomplete_type *incomplete_ptr GTY((ptr));

/* TYPE_ARRAY: Array types with length specifications */
typedef int fixed_array[10] GTY(());
typedef int variable_array[] GTY((length("my_length")));
typedef struct my_base_struct struct_array[] GTY(());

/* Flexible array member in a structure */
struct with_flex_array GTY(()) {
    int count;
    int data[] GTY((length("count")));
};

/* TYPE_CALLBACK: Function pointer types with callback marker */
typedef void (*simple_callback)(void) GTY((callback));
typedef int (*filter_callback)(const char *, void *) GTY((callback));
typedef void (*typed_callback)(struct my_base_struct *) GTY((callback));

/* Callback in a structure */
struct with_callback GTY(()) {
    int id;
    simple_callback cb GTY((skip));
};

/* TYPE_LANG_STRUCT: Language-specific structure types
 * Typically identified by tag or location in lang-specific dir
 */
struct GTY((tag("LANG"), desc("%0.kind"))) lang_specific_struct {
    enum { LANG_A, LANG_B, LANG_C } kind;
    union {
        int int_val;
        double double_val;
    } u GTY((desc("%0.kind")));
};

/* Another language struct pattern */
struct GTY((tag("CPLUSPLUS"))) cxx_lang_struct {
    void *vtable GTY((skip));
    int cxx_specific;
};

/* TYPE_UNDEFINED: Forward declarations and incomplete types 
 * that should be categorized as undefined
 */
struct undefined_struct;
typedef struct undefined_struct *undefined_ptr;

/* Malformed GTY annotation that might cause undefined classification */
struct GTY((invalid_option)) potentially_undefined {
    int x;
};

/* Self-referential structure that might cause issues */
struct self_ref GTY(()) {
    int data;
    struct self_ref *next GTY((ptr));
};

/* Template-like pattern (not actual C++ template) */
#define DECLARE_GTY_STRUCT(name, field_type) \
    struct name##_struct GTY(()) { \
        field_type value; \
        struct name##_struct *next GTY((ptr)); \
    }

DECLARE_GTY_STRUCT(int_node, int);
DECLARE_GTY_STRUCT(string_node, my_string_type);

/* Enumeration type (should be scalar-like) */
typedef enum {
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
} color_enum GTY(());

/* Bitfield structure */
struct with_bitfields GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    int regular_field;
};

/* Structure with nested anonymous union */
struct with_anon_union GTY(()) {
    int type;
    union {
        int i;
        float f;
        void *p GTY((ptr));
    } data;
};

/* Structure with conditional fields */
struct conditional_struct GTY(()) {
    int has_data;
    union {
        int int_data;
        double double_data;
    } data GTY((if("has_data")));
};

/* Array of pointers */
typedef struct my_base_struct *ptr_array[5] GTY(());

/* Multi-dimensional array */
typedef int matrix[3][4] GTY(());

/* End of type definitions */

#endif /* TEST_GTYPE_H */
