   struct tagged_string *create_string(int data_size) {
       struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_size);
       ts->length_bits = data_size * 8; // Assuming 8 bits per char
       return ts;
   }
