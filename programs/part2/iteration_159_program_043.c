/* test-gty.h - Header file with various GTY-annotated types */

#ifndef TEST_GTY_H
#define TEST_GTY_H

#ifdef __cplusplus
extern "C" {
#endif

/* TYPE_STRUCT: Basic struct with GTY annotation */
struct my_struct GTY(()) {
  int x;
  float y;
};

/* TYPE_UNION: Basic union with GTY annotation */
union my_union GTY(()) {
  int i;
  float f;
  void* p;
};

/* TYPE_POINTER: Will be used within another struct */
struct pointer_container GTY(()) {
  /* TYPE_POINTER case */
  struct my_struct* GTY((skip)) ptr_field;
  
  /* TYPE_ARRAY case - fixed size array */
  int GTY((length("10"))) arr_field[10];
  
  /* TYPE_SCALAR case */
  long GTY((skip)) counter;
  
  /* TYPE_STRING case */
  const char* GTY((skip)) name;
};

/* TYPE_CALLBACK: Callback function type */
typedef void (*callback_func)(int) GTY((callback));

/* Struct containing a callback */
struct with_callback GTY(()) {
  callback_func handler;
  int data;
};

/* Nested type graph - struct containing pointer to another GTY struct */
struct nested_graph GTY(()) {
  struct my_struct* GTY((skip)) child;
  struct nested_graph* GTY((skip)) next;  /* Self-referential pointer */
};

/* Union with mixed members */
union mixed_union GTY(()) {
  struct my_struct s;
  struct pointer_container* p;
  int i;
};

/* Template-like macro to generate multiple type instances */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

DEF_PAIR(int);
DEF_PAIR(float);
DEF_PAIR(struct my_struct*);

/* Forward declaration for user-defined type */
struct user_defined_type;

#ifdef __cplusplus
}
#endif

#endif /* TEST_GTY_H */
