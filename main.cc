#include <cstdarg>
#include <cstdio>

#include <iostream>
using namespace std;

#include <pthread.h>
#include <sched.h>

#include "log.h"

void *producer(void *arg)
{
    int *base = (int *)arg;
    for (int i = *base; i < *base + 1000000; i++){
        log_debug("%d, %s", i, "producer aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        sched_yield();
    }
    printf("producer exit\n");
    //SingletonLog::get_instance()->close_log();
    return NULL;
}

void *consumer(void *arg)
{
    SingletonLog::get_instance()->write_log_to_file();
    return NULL;
}

int main(int argc, char *argv[])
{
    SingletonLog::get_instance()->open_log("./log/log", SingletonLog::DEBUG, 1000);

    pthread_t aid, bid, cid, did;

    int a = 0, b = 1000000, c = 200000;
    pthread_create(&aid, NULL, producer, &a);
    pthread_create(&cid, NULL, producer, &b);
    pthread_create(&did, NULL, producer, &c);
    pthread_create(&bid, NULL, consumer, NULL);

    pthread_join(aid, NULL);
    pthread_join(cid, NULL);
    pthread_join(did, NULL);
    pthread_join(bid, NULL);

    return 0;
}
