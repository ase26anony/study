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

/* TYPE_POINTER - Will be triggered through struct members */
/* TYPE_ARRAY - Array type within struct */
struct container GTY(()) {
  /* TYPE_POINTER case */
  struct my_struct* GTY((skip)) ptr_field;
  
  /* TYPE_ARRAY case - fixed size array */
  int GTY((length("10"))) arr_field[10];
  
  /* TYPE_SCALAR case */
  long GTY((skip)) counter;
  
  /* TYPE_STRING case */
  const char* GTY((skip)) name;
};

/* TYPE_CALLBACK - Callback function type */
typedef void (*callback_fn)(int) GTY((callback));

/* Template-like macro for complex type generation */
#define DEF_PAIR(T) struct pair_##T { T first; T second; } GTY(())

/* Instantiate template-like types */
DEF_PAIR(int);
DEF_PAIR(double);

/* Forward declaration for type graph */
struct forward_decl;
struct forward_decl GTY(());

/* Complex nested structure for type graph */
struct graph_node GTY(()) {
  struct graph_node* GTY((skip)) next;
  struct graph_node* GTY((skip)) prev;
  struct container* GTY((skip)) data;
};

/* Union containing multiple types */
union complex_union GTY(()) {
  struct my_struct s;
  struct graph_node* GTY((skip)) node_ptr;
  int arr[5];
};

/* Language-specific structure simulation */
struct lang_specific GTY((tag("TS_VAR_DECL"))) {
  int decl_uid;
  const char* GTY((skip)) decl_name;
};

#endif /* TEST_GTY_H */
