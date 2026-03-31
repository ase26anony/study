/* test-gty.h - Header file with GTY annotations for gengtype testing */

#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int ival;
    double dval;
    void* pval;
};

/* TYPE_POINTER: Will be used through pointer fields */
struct pointer_container GTY(()) {
    struct my_struct* GTY((skip)) ptr_field;  /* TYPE_POINTER */
    union my_union* GTY((skip)) union_ptr;    /* TYPE_POINTER */
};

/* TYPE_ARRAY: Fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) fixed_arr[10];    /* TYPE_ARRAY */
    struct my_struct GTY((length("5"))) struct_arr[5];
};

/* TYPE_SCALAR: Scalar types with GTY */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;                 /* TYPE_SCALAR */
    unsigned GTY((skip)) flags;
    short GTY((skip)) small;
};

/* TYPE_STRING: String fields */
struct string_container GTY(()) {
    const char* GTY((skip)) name;             /* TYPE_STRING */
    char* GTY((skip)) buffer;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));  /* TYPE_CALLBACK */

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;
};

/* Complex nested structure for type graph testing */
struct node GTY(()) {
    int value;
    struct node* GTY((skip)) next;           /* Recursive pointer */
    struct node* GTY((skip)) children[4];    /* Array of pointers */
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct my_struct*);

/* Forward declaration for user-defined type */
struct user_defined_type;

#endif /* TEST_GTY_H */
