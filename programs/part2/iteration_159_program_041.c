#ifndef TEST_GTY_H
#define TEST_GTY_H

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
    int x;
    double y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
    int int_val;
    double double_val;
    void* ptr_val;
};

/* TYPE_POINTER: Struct containing pointers */
struct pointer_container GTY(()) {
    /* TYPE_POINTER case */
    struct my_struct* GTY((skip)) struct_ptr;
    
    /* TYPE_STRING case */
    const char* GTY((skip)) name;
    
    /* TYPE_SCALAR case */
    long GTY((skip)) counter;
};

/* TYPE_ARRAY: Struct with fixed-size array */
struct array_container GTY(()) {
    /* TYPE_ARRAY case with length attribute */
    int GTY((length("10"))) arr[10];
    
    /* Multi-dimensional array */
    double GTY((length("5"), length("5"))) matrix[5][5];
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

/* Struct using callback type */
struct callback_container GTY(()) {
    callback_fn handler;
    int data;
};

/* Template-like macro to generate multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(void*);

/* Complex nested structure for type graph */
struct node GTY(()) {
    int value;
    struct node* GTY((skip)) next;
    struct node* GTY((skip)) prev;
};

/* Union containing struct and pointer */
union complex_union GTY(()) {
    struct my_struct s;
    struct node* n;
    callback_fn handler;
};

/* Forward declaration for mutual reference */
struct forward_decl GTY(());

struct mutual_ref_a GTY(()) {
    int id;
    struct mutual_ref_b* GTY((skip)) partner;
};

struct mutual_ref_b GTY(()) {
    int id;
    struct mutual_ref_a* GTY((skip)) partner;
};

/* Language-specific structure hook simulation */
struct lang_specific GTY((tag("TS_VAR_DECL"))) {
    int decl_type;
    const char* GTY((skip)) identifier;
    struct lang_specific* GTY((skip)) chain;
};

#endif /* TEST_GTY_H */
