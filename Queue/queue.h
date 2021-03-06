#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct Bin* Bin;
struct Bin {

  void* element;
  Bin next;

};

typedef struct Queue* Queue;
struct Queue {

  Bin front;
  Bin back;
  int length;
  pthread_rwlock_t lock;

};

int queueLoad (Queue Q, int (*eval)(void*));

void queueEnqueue (Queue Q, void* e);

void* queueDequeue (Queue Q);

int queueLength (Queue Q);

Queue queueInit ();

