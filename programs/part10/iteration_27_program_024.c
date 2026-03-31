   for (int i = 0; i < 5000; i++) {
       for (int j = 0; j < 4; j++) {
           acc[j] = acc[j] * mul[j] + input[i];
       }
   }
   for (int j = 0; j < 4; j++) result[j] = acc[j];
