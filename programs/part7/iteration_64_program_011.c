#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct opaque_struct;
typedef struct opaque_struct *opaque_ptr_t;

/* TYPE_SCALAR: Basic scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

/* TYPE_STRING: String type */
typedef const char *string_t;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_func)(void *data);
typedef int (*compare_func)(const void *, const void *);

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr;
typedef struct basic_struct *basic_struct_ptr;

/* TYPE_ARRAY: Array type */
typedef int int_array[10];

/* TYPE_STRUCT: Basic structure */
struct GTY(()) basic_struct {
    int GTY((skip)) *ignored_ptr;  /* GTY((skip)) option */
    char *GTY((length("strlen($) + 1"))) name;  /* Variable-length string */
    int value;
    color_t color;
    callback_func callback;
};

/* TYPE_UNION: Union type */
union GTY((desc("$->type"))) tagged_union {
    int GTY((tag("0"))) int_val;
    float GTY((tag("1"))) float_val;
    char *GTY((tag("2"))) string_val;
    struct basic_struct *GTY((tag("3"))) struct_ptr;
    int type;  /* Discriminator field */
};

/* TYPE_USER_STRUCT: User-defined structure with custom traversal */
struct GTY((user)) user_defined_struct {
    void *custom_data;
    int (*custom_traverse)(void *);
    void (*custom_mark)(void *);
};

/* Linked list structure for recursive traversal */
struct GTY(()) linked_list {
    int data;
    struct linked_list *GTY((skip)) next_skip;  /* Skip this pointer */
    struct linked_list *next;
};

/* Tree structure for complex traversal */
struct GTY(()) tree_node {
    int value;
    struct tree_node *left;
    struct tree_node *right;
    union tagged_union data;
};

/* TYPE_ARRAY within structure */
struct GTY(()) array_container {
    int GTY((length("len"))) *dynamic_array;
    int len;
    int fixed_array[5];
    struct basic_struct *ptr_array[3];
};

#endif /* TEST_GTY_H */
