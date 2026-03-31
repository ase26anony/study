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
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Pointer type within structure - recursive */
struct GTY(()) linked_node {
    int data;
    struct linked_node *GTY((skip)) next;  /* TYPE_POINTER */
    struct linked_node *GTY((skip)) prev;
};

/* Array type definition */
typedef int GTY(()) int_array[10];  /* TYPE_ARRAY */

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);

/* Scalar type in container */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;  /* TYPE_SCALAR */
    double GTY((skip)) double_value;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) message;  /* TYPE_STRING */
    char *GTY((skip)) buffer;
};

/* Nested structure with union */
struct GTY(()) complex_nested {
    union data_union GTY((skip)) u;
    struct linked_node *GTY((skip)) node_list;
    int_array arr;  /* Array type */
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
struct GTY(()) options_test {
    int id GTY((tag("0")));
    void *GTY((skip, tag("1"))) data;
    struct linked_node *GTY((skip, length("id"))) nodes;
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct base_struct *GTY((skip)) ptrs[5];  /* Array of pointers */
    callback_func GTY((skip)) callbacks[3];   /* Array of callbacks */
};

/* Simulating lang_struct pattern - TYPE_LANG_STRUCT */
/* In GCC, lang_struct types often have special handling */
struct GTY(()) lang_struct_sim {
    int lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declaration for mutual recursion */
struct GTY(()) forward_decl_struct;

/* Mutual recursion between structures */
struct GTY(()) mutual_a {
    int val;
    struct mutual_b *GTY((skip)) b_ptr;
};

struct GTY(()) mutual_b {
    int val;
    struct mutual_a *GTY((skip)) a_ptr;
};

/* Extern declarations for use across files */
extern struct base_struct GTY((skip)) global_base;
extern union data_union GTY((skip)) global_union;

#ifdef __cplusplus
}
#endif

#endif /* GTY_TEST_TYPES_H */
