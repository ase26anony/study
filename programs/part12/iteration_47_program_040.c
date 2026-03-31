   struct tagged_string *create_string(int length_bits, const char *content) {
       size_t data_size = (length_bits + 7) / 8; // Convert bits to bytes
       struct tagged_string *ts = malloc(sizeof(struct tagged_string) + data_size);
       ts->length_bits = length_bits;
       memcpy(ts->data, content, data_size);
       return ts;
   }
