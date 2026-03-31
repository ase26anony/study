/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED */
struct opaque;

/* TYPE_SCALAR */
typedef int GTY(()) scalar_type;

/* TYPE_STRING */
typedef char *GTY(()) string_type;

/* TYPE_STRUCT */
struct GTY(()) my_struct {
    /* TYPE_SCALAR */
    int scalar_field;
    
    /* TYPE_STRING */
    char *string_field;
    
    /* TYPE_POINTER */
    struct my_struct *next;
    
    /* TYPE_ARRAY */
    int array_field[10];
    
    /* TYPE_UNDEFINED (opaque pointer) */
    struct opaque *opaque_ptr;
};

/* TYPE_USER_STRUCT */
typedef struct my_struct GTY(()) my_user_struct;

/* TYPE_UNION */
union GTY(()) my_union {
    int int_val;
    char *string_val;
    struct my_struct *struct_ptr;
};

/* TYPE_CALLBACK */
typedef void (*GTY(()) callback_type)(int, char *);

/* TYPE_POINTER (standalone) */
typedef struct my_struct *GTY(()) struct_ptr_type;

/* TYPE_ARRAY (standalone) */
typedef int GTY(()) int_array_type[20];

/* TYPE_LANG_STRUCT - Typically used for language-specific structures */
struct GTY(()) lang_specific {
    int lang_tag;
    void *lang_data;
};

/* Nested structures for comprehensive testing */
struct GTY(()) outer_struct {
    struct my_struct inner;
    union my_union choice;
    callback_type callback;
    struct lang_specific *lang_struct;
};

/* Function pointer in struct for TYPE_CALLBACK */
struct GTY(()) with_callback {
    int id;
    callback_type handler;
    void (*GTY(()) another_handler)(struct my_struct *);
};

/* Array of pointers */
struct GTY(()) pointer_array {
    struct my_struct *GTY((length("count"))) items;
    int count;
};

/* Self-referential structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
};

#endif /* GTY_TEST_H */
