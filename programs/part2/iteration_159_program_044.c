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

/* TYPE_POINTER - Will be used within other structs */
struct pointer_target GTY(()) {
    int data;
};

/* TYPE_ARRAY - Array type within a struct */
struct array_container GTY(()) {
    int GTY((length("10"))) arr[10];
    char GTY((length("20"))) str[20];
};

/* TYPE_SCALAR - Scalar types with GTY */
struct scalar_types GTY(()) {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    float GTY((skip)) value;
};

/* TYPE_STRING - String types */
struct string_types GTY(()) {
    const char * GTY((skip)) name;
    char * GTY((skip)) description;
};

/* TYPE_CALLBACK - Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

struct callback_container GTY(()) {
    callback_fn handler;
};

/* Complex nested structure for TYPE_GRAPH testing */
struct complex_node GTY(()) {
    int id;
    struct complex_node * GTY((skip)) next;  /* TYPE_POINTER */
    struct complex_node * GTY((skip)) prev;  /* TYPE_POINTER */
    union my_union GTY((tag("0"))) data;     /* TYPE_UNION inside struct */
};

/* Template-like macro to generate multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct pointer_target*);

/* Forward declaration for mutual recursion */
struct tree_node GTY(());

/* Language-specific structure simulation */
struct lang_specific GTY((tag("TS_VAR_DECL"))) {
    int decl_uid;
    struct tree_node * GTY((skip)) type;
    const char * GTY((skip)) name;
};

/* Mutual recursion example */
struct tree_node GTY(()) {
    int node_type;
    struct lang_specific * GTY((skip)) decl;  /* TYPE_LANG_STRUCT pointer */
    struct tree_node * GTY((skip)) left;
    struct tree_node * GTY((skip)) right;
};

#endif /* TEST_GTY_H */
