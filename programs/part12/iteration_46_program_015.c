// Creating a pointer to int
struct type *int_type = create_type(TYPE_SCALAR);
struct type *int_ptr = create_type(TYPE_POINTER);
int_ptr->u.pointer_to = int_type;

// Creating an array of strings
struct type *string_type = create_type(TYPE_STRING);
struct type *string_array = create_type(TYPE_ARRAY);
string_array->u.element_type = string_type;
