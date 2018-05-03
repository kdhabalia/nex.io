#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include "../Queue/queue.h"

#define RESULT_FILE 0
#define HARDWARE_STATS 1

typedef struct Workload* WorkloadPacket;
struct Workload {

  int jobID;
  int exeID;
  char* executablePath;
  char* dataPath;
  int load;
  int workloadType;

};

typedef struct Device* HardwareDevice;
struct Device {

  float capUtilization;
  float capMemoryUsage;
  float utilization;
  float memoryUsage;
  Queue Q;
  int numLaunched;
  WorkloadPacket* launchedPackets;

  pthread_rwlock_t lock;

};

struct senderArgs {

  HardwareDevice H;
  int sessfd;

};

struct receiverArgs {

  HardwareDevice H;
  int sockfd;

};

extern Queue inQ;

extern int registeredDevices;

extern HardwareDevice* devices;

HardwareDevice registerDevice (float capUtilization, float capMemoryUsage, float utilization, float memoryUsage);

void updateDeviceStats (HardwareDevice H, float capUtilization, float capMemoryUsage, float utilization, float memoryUsage);

void balanceLoads (void* threadArgs);

int numberOfDevices ();

HardwareDevice grabDevice (int index);

float* deviceStats (int index);

void sendToHardwareDevice (void* threadArgs);

void receiveFromHardwareDevice (void* threadArgs);

