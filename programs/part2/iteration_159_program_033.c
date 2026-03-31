/* test-gty.h - Header file with various GTY annotations */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

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

/* TYPE_POINTER: Will be used within another struct */
struct pointer_container GTY(()) {
  /* TYPE_POINTER case */
  struct my_struct* GTY((skip)) ptr_field;
  
  /* TYPE_ARRAY: Fixed-size array */
  int GTY((length("10"))) arr_field[10];
  
  /* TYPE_SCALAR: Scalar type with GTY */
  long GTY((skip)) counter;
  
  /* TYPE_STRING: String field */
  const char* GTY((skip)) name;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

/* Struct using callback type */
struct callback_container GTY(()) {
  callback_fn handler;
  int data;
};

/* Nested type graph for complex processing */
struct node GTY(()) {
  int value;
  struct node* GTY((skip)) next;
  struct node* GTY((skip)) prev;
};

/* Union containing struct and pointer */
union complex_union GTY(()) {
  struct my_struct s;
  struct node* n;
  long l;
};

/* Template-like macro to generate multiple types */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(double);
DEF_PAIR(struct node*);

/* Language-specific structure (mimicking Tree nodes) */
struct lang_struct GTY((tag("TS_VAR_DECL"))) {
  int code;
  union {
    int ival;
    double dval;
    const char* sval;
  } GTY((desc("%1.code"))) u;
};

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
