#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Skip this field */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((tag("0"))) ptr_val;  /* Tagged pointer */
    struct base_struct *GTY((skip)) struct_ptr;
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
    union data_union data;
    int GTY((skip)) metadata;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct linked_node *GTY(()) node_ptr_array[5];

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((length("strlen($)"))) dynamic_str;
    char fixed_str[50];
};

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) event_callback)(int event_id, void *GTY((skip)) user_data);

/* Structure containing callback */
struct GTY(()) event_handler {
    event_callback GTY((skip)) callback;
    void *GTY((skip)) user_data;
    int handler_id;
};

/* Recursive type structure */
struct GTY(()) tree_node {
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
    int value;
};

/* Union containing structure, structure containing union */
struct GTY(())) complex_container {
    union data_union variant;
    struct {
        int counter;
        char tag;
    } GTY((skip)) inner;
    struct tree_node *GTY((skip)) node_tree;
};

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    unsigned int GTY((skip)) flags;
    double GTY((skip)) precision;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    char *GTY((skip)) desc;
};
#else
struct conditional_struct {
    int x;
    char *desc;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) option_chain {
    void *GTY((skip, tag("1"))) tagged_ptr;
    int GTY((skip)) skip_field;
    struct linked_node *GTY((length("$->id"))) node_array[8];
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_struct types often have special handling */
struct GTY(()) lang_simulated {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl;
struct GTY(())) container_a {
    struct forward_decl *GTY((skip)) fwd_ptr;
    int a_data;
};

struct GTY(()) forward_decl {
    struct container_a *GTY((skip)) back_ptr;
    int fwd_data;
};

/* Array of different GTY-marked types */
struct GTY(()) heterogeneous_array {
    struct base_struct *GTY((skip)) structs[3];
    union data_union GTY((skip)) unions[2];
    event_callback callbacks[4];
};

/* Nested anonymous struct/unions with GTY */
struct GTY(()) nested_anonymous {
    struct {
        int x;
        int y;
    } GTY((skip)) point;
    union {
        int as_int;
        float as_float;
    } GTY((skip)) value;
    struct tree_node hierarchy;
};

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */
