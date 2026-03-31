// Creating an int pointer type
struct type *int_type = create_type(TYPE_SCALAR);
struct type *int_ptr_type = create_type(TYPE_POINTER);
int_ptr_type->u.pointer_to = int_type;

// Creating an array of strings
struct type *string_type = create_type(TYPE_STRING);
struct type *string_array_type = create_type(TYPE_ARRAY);
string_array_type->u.element_type = string_type;
