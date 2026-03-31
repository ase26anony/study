   struct tagged_string *create_string(int data_size) {
       struct tagged_string *str = malloc(sizeof(struct tagged_string) + data_size);
       str->length_bits = data_size * 8; // Assuming 8 bits per char
       return str;
   }
