   struct tagged_string *create_string(int length_bits, size_t data_size) {
       struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_size);
       if (ts) {
           ts->length_bits = length_bits;
           // Initialize data...
       }
       return ts;
   }
