
#include <stdio.h>
#include <stdlib.h>

#define _GNU_SOURCE
#include <pthread.h>


static inline void raise_error(const char* message)
{
    fprintf(stderr, "%s\n", message);
    exit(EXIT_FAILURE);
}


static void display_pthread_attr(pthread_attr_t *attr, char *prefix)
{
    int s, i;
    size_t v;
    void *stkaddr;
    struct sched_param sp;

    s = pthread_attr_getdetachstate(attr, &i);
    if (s != 0)
        raise_error("Bad pthread_attr_getdetachstate");
    else
        printf("%sDetach state        = %s\n", prefix,
                (i == PTHREAD_CREATE_DETACHED) ? "PTHREAD_CREATE_DETACHED" :
                (i == PTHREAD_CREATE_JOINABLE) ? "PTHREAD_CREATE_JOINABLE" :
                "???");


    s = pthread_attr_getscope(attr, &i);
    if (s != 0)
        raise_error("Bad pthread_attr_getscope");
    else      
        printf("%sScope               = %s\n", prefix,
                (i == PTHREAD_SCOPE_SYSTEM)  ? "PTHREAD_SCOPE_SYSTEM" :
                (i == PTHREAD_SCOPE_PROCESS) ? "PTHREAD_SCOPE_PROCESS" :
                "???");


    s = pthread_attr_getinheritsched(attr, &i);
    if (s != 0)
        raise_error("Bad pthread_attr_getinheritsched");
    else
        printf("%sInherit scheduler   = %s\n", prefix,
                (i == PTHREAD_INHERIT_SCHED)  ? "PTHREAD_INHERIT_SCHED" :
                (i == PTHREAD_EXPLICIT_SCHED) ? "PTHREAD_EXPLICIT_SCHED" :
                "???");


    s = pthread_attr_getschedpolicy(attr, &i);
    if (s != 0)
        raise_error("Bad pthread_attr_getschedpolicy");
    else
        printf("%sScheduling policy   = %s\n", prefix,
                (i == SCHED_OTHER) ? "SCHED_OTHER" :
                (i == SCHED_FIFO)  ? "SCHED_FIFO" :
                (i == SCHED_RR)    ? "SCHED_RR" :
                "???");

    s = pthread_attr_getschedparam(attr, &sp);
    if (s != 0)
        raise_error("Bad pthread_attr_getschedparam");
    else
        printf("%sScheduling priority = %d\n", prefix, sp.sched_priority);


    s = pthread_attr_getguardsize(attr, &v);
    if (s != 0)
        raise_error("Bad pthread_attr_getguardsize");
    else
        printf("%sGuard size          = %ld bytes\n", prefix, v);

    s = pthread_attr_getstack(attr, &stkaddr, &v);
    if (s != 0)
        raise_error("Bad pthread_attr_getstack");
    
    printf("%sStack address       = %p\n", prefix, stkaddr);
    printf("%sStack size          = 0x%zx bytes\n", prefix, v);
}


void* start_thread(void* arg)
{
    pthread_attr_t attr;

    int s = pthread_getattr_np(pthread_self(), &attr);
    if (s != 0)
        raise_error("Bad pthread getattr np");
    
    display_pthread_attr(&attr, "\t");

    return NULL;
}


int main()
{
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) == -1)
        raise_error("Failed to init pthread attr");

    pthread_t thread;
    if (pthread_create(&thread, &attr, start_thread, NULL) == -1)
        raise_error("Failed to create thread");
    
    if (pthread_join(thread, NULL) == -1)
        raise_error("Bad pthread_join");

    return EXIT_SUCCESS;
}