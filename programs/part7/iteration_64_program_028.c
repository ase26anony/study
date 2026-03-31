#ifndef TEST_GTY_TYPES_H
#define TEST_GTY_TYPES_H

/* TYPE_UNDEFINED: Forward declaration of incomplete type */
struct opaque;
typedef struct opaque *opaque_ptr_t;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int_t;
typedef char scalar_char_t;
typedef long scalar_long_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum_t;

/* TYPE_STRING: String types */
typedef const char *const_string_t;
typedef char *mutable_string_t;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*simple_callback_t)(void);
typedef int (*complex_callback_t)(const char *, int);
typedef void (*destructor_callback_t)(void *);

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr_t;
typedef struct basic_struct *struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int int_array_10_t[10];
typedef char *string_array_t[5];

/* TYPE_STRUCT: Basic structure */
struct basic_struct GTY(())
{
  int id;
  char name[32];
  float value;
  struct basic_struct *next;
};

/* TYPE_UNION: Union type */
union data_union GTY(())
{
  int int_val;
  float float_val;
  double double_val;
  char *string_val;
};

/* TYPE_USER_STRUCT: User-defined structure with custom traversal */
struct user_defined_struct GTY((user))
{
  void *custom_data;
  int user_tag;
  /* User will provide custom marking functions */
};

/* For TYPE_LANG_STRUCT in C++ */
#ifdef __cplusplus
class BaseClass GTY(())
{
public:
  virtual ~BaseClass() {}
  int base_value;
  
  virtual void method() = 0;
};

class DerivedClass : public BaseClass GTY(())
{
public:
  char *derived_name;
  void method() override {}
};
#endif

#endif /* TEST_GTY_TYPES_H */
