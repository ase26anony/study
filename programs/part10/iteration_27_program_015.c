int acc0 = init[0];
int acc1 = init[1];
int acc2 = init[2];
int acc3 = init[3];

int mul0 = mul[0];
int mul1 = mul[1];
int mul2 = mul[2];
int mul3 = mul[3];

for (int i = 0; i < 5000; i++) {
    int val = input[i];
    acc0 = acc0 * mul0 + val;
    acc1 = acc1 * mul1 + val;
    acc2 = acc2 * mul2 + val;
    acc3 = acc3 * mul3 + val;
}

result[0] = acc0;
result[1] = acc1;
result[2] = acc2;
result[3] = acc3;
