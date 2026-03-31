std::mutex mtx;
int g = 0;

void test() {
    int local = 5;
    std::lock_guard<std::mutex> lock(mtx);
    if (g > local) {
        g = local;
    }
}
