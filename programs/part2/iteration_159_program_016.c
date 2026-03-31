#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT - Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION - Basic union with GTY annotation */
union my_union GTY(()) {
    int i;
    double d;
    void* p;
};

/* TYPE_POINTER - Will be used within another struct */
struct pointer_container GTY(()) {
    struct my_struct* GTY((skip)) ptr_field;  /* TYPE_POINTER */
    union my_union* GTY((skip)) union_ptr;    /* Another TYPE_POINTER */
};

/* TYPE_ARRAY - Fixed-size array */
struct array_container GTY(()) {
    int GTY((length("10"))) arr[10];          /* TYPE_ARRAY */
    char GTY((length("256"))) buffer[256];    /* Another TYPE_ARRAY */
};

/* TYPE_SCALAR - Scalar types with GTY */
struct scalar_container GTY(()) {
    long GTY((skip)) counter;                 /* TYPE_SCALAR */
    unsigned GTY((skip)) flags;               /* Another TYPE_SCALAR */
};

/* TYPE_STRING - String fields */
struct string_container GTY(()) {
    const char* GTY((skip)) name;             /* TYPE_STRING */
    char* GTY((skip)) dynamic_str;            /* Another TYPE_STRING */
};

/* TYPE_CALLBACK - Callback function type */
typedef void (*callback_fn)(int) GTY((callback));  /* TYPE_CALLBACK */

struct callback_container GTY(()) {
    callback_fn GTY((skip)) handler;          /* Uses TYPE_CALLBACK */
};

/* Complex nested structure for type graph */
struct node GTY(()) {
    int value;
    struct node* GTY((skip)) next;           /* Recursive pointer */
    struct node* GTY((skip)) prev;           /* Another pointer */
};

/* Union containing struct and pointer */
union complex_union GTY(()) {
    struct node n;
    void* GTY((skip)) data;
    int* GTY((skip)) int_ptr;
};

/* Template-like macro for multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct node*);

/* Forward declaration for mutual recursion */
struct forward_decl GTY(());

struct recursive_container GTY(()) {
    struct forward_decl* GTY((skip)) fwd_ptr;
    struct recursive_container* GTY((skip)) self_ptr;
};

struct forward_decl GTY(()) {
    int id;
    struct recursive_container* GTY((skip)) container;
};

#endif /* TEST_GTY_H */
