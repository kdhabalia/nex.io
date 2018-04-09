#include "capstoneServer.h"
sbuf_t sbuf;

int main(int argc, char **argv)
{
    int i, listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    pthread_t tid;

    if (argc != 2) {
	    fprintf(stderr, "usage: %s <port>\n", argv[0]);
	    exit(1);
    }
    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);

    // Launches threads for separate components
    srand(time(NULL));

    inQ = queueInit();

    pthread_t D0worker;
    pthread_create(&D0worker, NULL, Device, (void*)"0");

    pthread_t D1worker;
    pthread_create(&D1worker, NULL, Device, (void*)"1");

    pthread_t EPworker;
    pthread_create(&EPworker, NULL, enqueueNewPacket, (void*)NULL);

    pthread_t LBworker;
    pthread_create(&LBworker, NULL, balanceLoads, (void*)NULL);

    sbuf_init(&sbuf, SBUFSIZE);
    for(i = 0; i < NTHREADS; i++) {
      Pthread_create(&tid, NULL, thread, NULL);
    }

    while (1) {
		  clientlen = sizeof(clientaddr);
		  connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
			sbuf_insert(&sbuf, connfd);
    }
}

float randomFloat()
{
  float r = (float)rand()/(float)RAND_MAX;
  return r;
}

void Device (void* threadArgs) {

  char* IP = (char*)threadArgs;

  float DcapU = randomFloat();
  DcapU = (DcapU < 0.5) ? 0.5 : DcapU;
  float DcapMU = randomFloat();
  DcapMU = (DcapMU < 0.6) ? 0.6 : DcapMU;
  float DU = randomFloat();
  DU = (DU > 0.4) ? 0.43 : DU;
  float DMU = randomFloat();
  DMU = (DMU > 0.4) ? 0.43 : DMU;

  HardwareDevice H = registerDevice(IP, DcapU, DcapMU, DU, DMU);
  printf("Device %s registered\n", IP);

  int currentPacketCount = 0;
  time_t seconds = time(NULL);
  while (1) {

    int l = queueLength(H->Q);
    if (l > currentPacketCount) {
      DU += 0.15;
      DU = (DU > 1.0) ? 1.0 : DU;
      DMU += 0.12;
      DMU = (DMU > 1.0) ? 1.0 : DMU;
      updateDeviceStats(H, DcapU, DcapMU, DU, DMU);

      currentPacketCount++;
    }

    if (l > 0 && time(NULL) - seconds > 10) {
      seconds = time(NULL);
      void* e = queueDequeue(H->Q);
      DU -= 0.15;
      DMU -= 0.12;
      updateDeviceStats(H, DcapU, DcapMU, DU, DMU);
      currentPacketCount--;
    }

    if (l == 0) {
      seconds = time(NULL);
    }

  }

}

void* thread(void* vargp)
{
  Pthread_detach(pthread_self());
  while(1) {
    int connfd = sbuf_remove(&sbuf);
    doit(connfd);
    Close(connfd);
  }
}


void doit(int fd)
{
  char funcNumber[4];
  int whichFunction;
  recv(fd, funcNumber, 4, MSG_WAITALL);
  memcpy(&whichFunction, funcNumber, 4);
  if(whichFunction == SYSTEM_STATE) {
    int registeredDevices = numberOfDevices();
    send(fd, &registeredDevices, INT_SIZE, 0);
    float* infoArray;
    //MARIOS INFO ARRAY FUNCTION CALL IS HERE
    for(int i = 0; i < registeredDevices; i++) {
      infoArray = deviceStats(i);
      send(fd, infoArray, 4*INT_SIZE, 0);
      free(infoArray);
    }
  } else if(whichFunction == SEND_FILE) {
    int pathSize;
    recv(fd, &pathSize, INT_SIZE, MSG_WAITALL);
    char* fileName = malloc(pathSize+1);
    char* parentName = "../Storage/";
    char* result = malloc(pathSize + strlen(parentName) +1);
    recv(fd, fileName, pathSize, MSG_WAITALL);
    fileName[pathSize] = '\0';
    strcpy(result, parentName);
    strcat(result, fileName);
    int copyFd = open(result, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
    int fileSize;
    recv(fd, &fileSize, INT_SIZE, MSG_WAITALL);
    char* bufferToRecv = malloc(fileSize);
    recv(fd, bufferToRecv, fileSize, MSG_WAITALL);
    write(copyFd, bufferToRecv, fileSize);

    free(fileName);
    free(result);
    free(bufferToRecv);
    close(copyFd);
  }

}

