import os
from threading import Thread

databaseDictionary = {}
packetCounter = 0

def populateDictionary():
  global databaseDictionary
  directory = "../clientFileDatabase/"
  while(1):
    for folders in os.listdir(directory):
      if folders not in databaseDictionary:
        databaseDictionary[folders] = {}
        for execs in os.listdir(directory +"/" + folders):
          for txtFiles in os.listdir(directory + "/" + folders + "/" + execs):
            if(txtFiles.lower().endswith(".txt")):
              (databaseDictionary[folders])[txtFiles] = [execs, 0]


def putTasksInQueue(granularity):
  global databaseDictionary
  while(1):
    lenOfDictionary = len(databaseDictionary)
    threadArray = []
    flag = 0
    if(lenOfDictionary > granularity):
      chunk = lenOfDictionary / granularity
      for i in xrange(granularity):
        T = Thread(target=setToRun, args=(i*chunk,(i+1)*chunk, databaseDictionary,))
        threadArray.append(T)
    else:
      flag = 1
      setToRun(0, lenOfDictionary, databaseDictionary)
    if(not flag):
      for i in range(granularity):
        threadArray[i].join()


def setToRun(startVal, tillVal, databaseDictionary):
  global packetCounter
  j = startVal
  while(j < tillVal):
    jobDict = databaseDictionary[str(j)]
    for i in jobDict.keys():
      if((jobDict[i][1]) == 0):
        jobDict[i][1] = 1
        tempPath = "../clientFileDatabase/"+str(i)+"/"
        with open("../enqueCache/packet"+str(packetCounter)+".txt", "w+") as myfile:
          packetCounter += 1
          execVal = str(jobDict[i][0])
          myfile.write(str(j)+"\n"+tempPath+execVal+"\n"+tempPath+execVal+"/"+i+"\n"+str(1)+"\n"+str(0))
    j+=1



def main():
  p1 = Thread(target=populateDictionary, args=())
  p2 = Thread(target=putTasksInQueue, args=(4,))
  p1.start()
  p2.start()
  p1.join()
  p2.join()

main()

