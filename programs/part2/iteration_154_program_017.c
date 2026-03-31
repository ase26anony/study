std::mutex mtx;
void test() {
    std::lock_guard<std::mutex> lock(mtx);
    int local = 5;
    if (g > local) {
        g = local;
    }
}
