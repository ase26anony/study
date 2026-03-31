/* gty-test.h - Test file for gengtype type classification coverage */

#ifndef GTY_TEST_H
#define GTY_TEST_H

/* Forward declaration for TYPE_UNDEFINED/opaque pointer */
struct opaque;

/* TYPE_SCALAR: plain scalar type */
/* TYPE_STRING: string type */
/* TYPE_POINTER: pointer type */
/* TYPE_ARRAY: array type */
struct GTY(()) scalar_and_pointer_types {
    int scalar_field;              /* TYPE_SCALAR */
    char *string_field;            /* TYPE_STRING */
    struct opaque *opaque_ptr;     /* TYPE_POINTER (to undefined type) */
    int array_field[10];           /* TYPE_ARRAY */
};

/* TYPE_STRUCT: regular struct */
struct GTY(()) regular_struct {
    int id;
    char *name;
    struct regular_struct *next;   /* TYPE_POINTER */
};

/* TYPE_UNION: union type */
union GTY(()) test_union {
    int int_val;
    char *str_val;                 /* TYPE_STRING */
    void *ptr_val;                 /* TYPE_POINTER */
};

/* TYPE_CALLBACK: function pointer */
typedef void GTY((callback)) (*callback_func)(int, char*);

/* Structure containing callback */
struct GTY(()) struct_with_callback {
    int id;
    callback_func callback;        /* TYPE_CALLBACK */
};

/* Nested structures for complex type relationships */
struct GTY(()) inner_struct {
    int value;
};

struct GTY(()) outer_struct {
    struct inner_struct inner;     /* TYPE_STRUCT */
    struct outer_struct *self_ptr; /* TYPE_POINTER */
};

/* Union within struct */
struct GTY(()) struct_with_union {
    int type;
    union {
        int int_member;
        char *str_member;          /* TYPE_STRING */
    } GTY((desc("type"))) u;
};

/* Array of pointers */
struct GTY(()) array_of_pointers {
    struct regular_struct * GTY((length("count"))) items[50]; /* TYPE_ARRAY of TYPE_POINTER */
    int count;
};

/* Chain structure for linked list testing */
struct GTY(()) chain_link {
    int data;
    struct chain_link * GTY((skip)) next; /* TYPE_POINTER with skip */
    struct chain_link *prev;               /* TYPE_POINTER */
};

/* Language-specific structure (simulating TYPE_LANG_STRUCT) */
#ifdef LANGUAGE_HOOKS
struct GTY(()) lang_specific_struct {
    void *language_data;
};
#endif

/* User-defined structure type */
typedef struct GTY(()) regular_struct user_struct_t;

#endif /* GTY_TEST_H */
