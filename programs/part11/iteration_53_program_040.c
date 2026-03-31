/* Test types for gengtype coverage */

/* TYPE_UNDEFINED - opaque/incomplete type */
struct opaque_struct;

/* TYPE_STRUCT */
struct my_struct {
    int a;
    double b;
};

/* TYPE_USER_STRUCT - using GTY marker */
struct GTY((user)) user_struct {
    int user_data;
};

/* TYPE_UNION */
union my_union {
    int i;
    float f;
    void *p;
};

/* TYPE_POINTER */
struct with_pointers {
    int *int_ptr;
    struct my_struct *struct_ptr;
    void **void_ptr_ptr;
};

/* TYPE_ARRAY */
struct with_arrays {
    int fixed_array[10];
    char string_array[5][20];
};

/* TYPE_SCALAR */
typedef int my_scalar_type;
typedef unsigned long size_type;

/* TYPE_STRING */
struct with_strings {
    char *name;
    const char *const_string;
};

/* TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);
typedef void (*simple_callback)(void);

struct with_callbacks {
    callback_func handler;
    simple_callback cleanup;
};

/* Nested structures for complex testing */
struct container {
    struct my_struct nested_struct;
    union my_union nested_union;
    struct with_pointers *ptr_member;
};
