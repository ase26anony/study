#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int g = 0;

void test() {
    int local = 5;
    pthread_mutex_lock(&lock);
    if (g > local) {
        g = local;
    }
    pthread_mutex_unlock(&lock);
}
