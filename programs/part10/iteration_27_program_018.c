int acc[4] = {init[0], init[1], init[2], init[3]};
int mul_arr[4] = {mul[0], mul[1], mul[2], mul[3]};

for (int i = 0; i < 5000; i++) {
    int val = input[i];
    for (int j = 0; j < 4; j++) {
        acc[j] = (acc[j] * mul_arr[j]) + val;
    }
}

for (int j = 0; j < 4; j++) {
    result[j] = acc[j];
}
