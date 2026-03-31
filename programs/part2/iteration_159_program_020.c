#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int i;
    double d;
    void* p;
};

/* TYPE_POINTER: Will be used within other structs */
struct pointer_container GTY(()) {
    struct my_struct * GTY((skip)) ptr_field;  /* TYPE_POINTER */
};

/* TYPE_ARRAY: Fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) arr[10];  /* TYPE_ARRAY */
};

/* TYPE_SCALAR: Scalar type with GTY */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;  /* TYPE_SCALAR */
};

/* TYPE_STRING: String field */
struct string_container GTY(()) {
    const char * GTY((skip)) name;  /* TYPE_STRING */
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));  /* TYPE_CALLBACK */

struct callback_container GTY(()) {
    callback_fn handler;
};

/* Complex nested type for recursive processing */
struct nested_struct GTY(()) {
    struct my_struct * GTY((skip)) child;
    struct nested_struct * GTY((skip)) next;  /* Self-referential pointer */
};

/* Union with mixed members */
union mixed_union GTY(()) {
    struct my_struct s;
    struct pointer_container *p;
    int arr[5];
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for user-defined type */
struct user_defined_type;

#endif /* TEST_GTY_H */
