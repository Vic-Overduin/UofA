/*
 * =====================================================================================
 *	Operating Systems Assignment 1
 *  Filename:  OnlineService.c
 *
 *	Student1 Name: Pongsakorn Surasarang
 *  Student1 ID  : a1697114
 *
 *	Student2 Name: Victor Overduin
 *  Student2 ID  : a1653894
 *
 *  Command: gcc -std=c11 OnlineService.c -o run
 *  Command: ./run input.txt > output.txt
 *
 * =====================================================================================
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 1000
#define AGING_MECHANIC 8

// Defines node structure for LinkedList
typedef struct node {
  // Given data
  char *customer_ID;
  int arrival_time;
  int total_time;
  int priority;
  int age;

  // Output Data
  int terminationTime;
  int readyTime;

  // How long the CPU has been running for
  int runTime;

  // Queue1 Round Robin counter
  int fiveRuns;
  // Queue1 Demotion counter
  int twoFiveRuns;

  // Tracks Queue1 Round Robin, demotion / aging in Q2 Q3
  int numOfTimes;

  struct node *next;
} node_t;

// Initialisation of global booleans to be used in the main function
bool startInsert = false;
bool noChange = false;
bool preemption = false;
bool print_all = true;
bool queueTwoEmptied = false;
bool isAgePriortized = false;

bool queueThreeLengthOne = true;
bool lastInsert = false;
bool debug = false;

// Tracks current process time
int worldTime = 0;

// Declaration of LinkedLists
node_t *head = NULL;
node_t *Queue1 = NULL;
node_t *Queue2 = NULL;
node_t *Queue3 = NULL;
node_t *Final_Output = NULL;

node_t *head_tail = NULL;
node_t *Queue1_tail = NULL;
node_t *Queue2_tail = NULL;
node_t *Queue3_tail = NULL;
node_t *Final_Output_tail = NULL;

// Checks if a LinkedList is empty
bool isEmpty(node_t *the_node) { return the_node == NULL; }

// Finds a link whose Customer ID matches
node_t *findSubQueue(char *queueName, char *customer_ID) {
  node_t *current;

  if (strcmp(queueName, "Queue1") == 0) {
    current = Queue1;

    if (isEmpty(Queue1)) {
      return NULL;
    }
  } else if (strcmp(queueName, "Queue2") == 0) {
    current = Queue2;

    if (isEmpty(Queue2)) {
      return NULL;
    }
  } else if (strcmp(queueName, "Queue3") == 0) {
    current = Queue3;

    if (isEmpty(Queue3)) {
      return NULL;
    }
  } else if (strcmp(queueName, "final") == 0) {
    current = Final_Output;

    if (isEmpty(Final_Output)) {
      return NULL;
    }
  }

  while (true) {
    if (isEmpty(current)) {
      return NULL;
    } else {
      if (strcmp(current->customer_ID, customer_ID) == 0) {
        break;
      }

      current = current->next;
    }
  }

  return current;
}

// Find a link whose priority matches with the arrival time of the current time
node_t *tierFind(int x, int y, int currentTime) {
  struct node *current = head;

  if (isEmpty(head)) {
    return NULL;
  }

  while (true) {
    if (isEmpty(current)) {
      return NULL;
    } else {
      if ((current->priority == x && current->arrival_time == currentTime) ||
          (current->priority == y && current->arrival_time == currentTime)) {
        break;
      }
      current = current->next;
    }
  }

  return current;
}

// Insert link to the Head LinkedList
void insertHead(char *customer_ID, int arrival_time, int priority, int age,
                int total_time) {

  node_t *link = (struct node *)malloc(sizeof(node_t));

  link->customer_ID = customer_ID;
  link->arrival_time = arrival_time;
  link->priority = priority;
  link->age = age;
  link->total_time = total_time;
  link->runTime = 0;
  link->numOfTimes = 0;
  link->fiveRuns = 0;
  link->twoFiveRuns = 0;

  link->terminationTime = 0;
  link->readyTime = -1;

  if (isEmpty(head)) {
    head = link;
    head->next = NULL;
    head_tail = head;
    return;
  }

  head_tail->next = link;
  head_tail = link;
}

// Prints the results for debugging
void printQueue(char *queueName) {
  node_t *ptr;

  if (strcmp(queueName, "Queue1") == 0) {
    ptr = Queue1;
    queueName = "Queue 1";
  } else if (strcmp(queueName, "Queue2") == 0) {
    ptr = Queue2;
    queueName = "Queue 2";
  } else if (strcmp(queueName, "Queue3") == 0) {
    ptr = Queue3;
    queueName = "Queue 3";
  }
  printf("This is %s:\n", queueName);
  printf("Index	Arrival	Priority	age	CPU_Time\n");

  while (ptr != NULL) {
    if (print_all) {
      printf("%s	%d	%d	%d	%d\n", ptr->customer_ID,
             ptr->arrival_time, ptr->priority, ptr->age, ptr->total_time);
    } else {
      printf("%s\t\t %d\t\t %d\t\t %d\t\t %d\t\t %d\t\t %d\t\t %d\t\t %d\n",
             ptr->customer_ID, ptr->arrival_time, ptr->priority, ptr->age,
             ptr->total_time, ptr->runTime, ptr->numOfTimes, ptr->fiveRuns,
             ptr->twoFiveRuns);
    }
    ptr = ptr->next;
  }
}

// Displays the final output
void printFinal() {
  node_t *ptr = Final_Output;

  printf("Index	Priority\tArrival	End	Ready	CPU_Time\tWaiting\n");
  while (ptr != NULL) {
    printf("%s	%d	%d	%d	%d	%d	%d\n", ptr->customer_ID,
           ptr->priority, ptr->arrival_time, ptr->terminationTime,
           ptr->readyTime, ptr->total_time,
           ptr->terminationTime - ptr->readyTime - ptr->total_time);
    ptr = ptr->next;
  }
}

void printData() {
  if (debug) {
    printf("***************************\n");
    printf("The last moment of Time(%d)\n", worldTime);
    printQueue("Queue1");
    printQueue("Queue2");
    printQueue("Queue3");
  }

  worldTime++;
}

// Splits input chars into splitting by blocks of inputs that is read from file
int splitToBlocks(char *string, char *argv[]) {
  char *p = string;
  int argc = 0;

  while (*p != '\0') {
    while (isspace(*p))
      ++p;

    if (*p != '\0') {
      argv[argc++] = p;
    } else {
      break;
    }

    while (*p != '\0' && !isspace(*p))
      p++;

    if (*p != '\0') {
      *p = '\0';
      ++p;
    }
  }

  return argc;
}

// This function shares the statement of inserting to any other LinkedLists
void insertToQueue(char *name, node_t *values) {
  node_t *link = (struct node *)malloc(sizeof(struct node));

  link->customer_ID = values->customer_ID;
  link->arrival_time = values->arrival_time;
  link->priority = values->priority;
  link->age = values->age;
  link->total_time = values->total_time;
  link->runTime = values->runTime;

  // Checks for how data transfers between Queue 1 -> Queue 2 -> Queue 3 and if
  // Queue 3 -> Queue 2 -> Queue 1
  if (worldTime == 0 || noChange)
    link->numOfTimes = values->numOfTimes;
  else if (startInsert)
    link->numOfTimes = 0;
  else
    link->numOfTimes = -1;

  // Used in Real-Time where Queue 3 might contain more than one, and therefore
  // the last Queue 3 value will need to be one value higher
  if (lastInsert)
    link->terminationTime = values->terminationTime;
  else
    link->terminationTime = worldTime;

  link->readyTime = values->readyTime;
  link->fiveRuns = values->fiveRuns;
  link->twoFiveRuns = values->twoFiveRuns;

  // To make it easier for calls, this functions are split by comparing which
  // LinkedLists name it finds based on name of the function
  if (strcmp(name, "Queue1") == 0) {
    if (isEmpty(Queue1)) {
      Queue1 = link;
      Queue1->next = NULL;
      Queue1_tail = Queue1;
      return;
    }

    // Gives prioritise to when current arrival time insertion is 6 / 5
    node_t *current = Queue1;
    if (current->priority == 5 && link->priority == 6) {
      if (Queue1->runTime % 5 != 0) {
        Queue1->fiveRuns = Queue1->fiveRuns + 1;
        Queue1->runTime = (Queue1->runTime - Queue1->runTime % 5) + 5;
      }

      link->next = Queue1;
      Queue1 = link;
    } else if (current->priority == 5 && link->priority == 5 &&
               link->arrival_time == worldTime) {
      if (Queue1->runTime % 5 != 0) {
        Queue1->fiveRuns = Queue1->fiveRuns + 1;
        Queue1->runTime = (Queue1->runTime - Queue1->runTime % 5) + 5;
      }

      link->next = Queue1;
      Queue1 = link;
    } else if (link->priority == 5) {
      Queue1_tail->next = link;
      Queue1_tail = link;

    } else {
      while (current->next->priority != 5) {
        current = current->next;
      }

      link->next = current->next;
      current->next = link;
    }

    // Double check that the tail is always up to date
    while (current->next != NULL) {
      current = current->next;
    }
    Queue1_tail = current;

    // Inserting to Queue 2
  } else if (strcmp(name, "Queue2") == 0) {
    if (isEmpty(Queue2)) {
      Queue2 = link;
      Queue2->next = NULL;
      Queue2_tail = Queue2;
      return;
    }

    node_t *current = Queue2;
    Queue2_tail->next = link;
    Queue2_tail = link;

    // Double check that the tail is always up to date
    while (current->next != NULL) {
      current = current->next;
    }
    Queue2_tail = current;

    // Inserting to Queue 3
  } else if (strcmp(name, "Queue3") == 0) {
    if (isEmpty(Queue3)) {
      Queue3 = link;
      Queue3->next = NULL;
      Queue3_tail = Queue3;
      return;
    }

    node_t *current = Queue3;
    Queue3_tail->next = link;
    Queue3_tail = link;

    // Double check that the tail is always up to date
    while (current->next != NULL) {
      current = current->next;
    }
    Queue3_tail = current;

  } else if (strcmp(name, "final") == 0) {
    if (isEmpty(Final_Output)) {
      Final_Output = link;
      Final_Output->next = NULL;
      Final_Output_tail = Final_Output;
      return;
    }

    Final_Output_tail->next = link;
    Final_Output_tail = link;
  }
}

// Delete a link with given Customer ID key from the head
node_t *deleteHead(char *customer_ID) {
  node_t *current = head;
  node_t *previous = NULL;

  if (isEmpty(head)) {
    return NULL;
  }

  while (strcmp(current->customer_ID, customer_ID) != 0) {
    if (current->next == NULL) {
      return NULL;
    } else {
      previous = current;
      current = current->next;
    }
  }

  if (current == head) {
    head = head->next;
  } else {
    previous->next = current->next;
  }

  return current;
}

// A function that help delete Queue 1
node_t *deleteQueue1(char *customer_ID) {
  node_t *current = Queue1;
  node_t *previous = NULL;

  if (isEmpty(Queue1)) {
    return NULL;
  }

  while (strcmp(current->customer_ID, customer_ID) != 0) {
    if (current->next == NULL) {
      return NULL;
    } else {
      previous = current;
      current = current->next;
    }
  }

  if (current == Queue1) {
    Queue1 = Queue1->next;
  } else {
    previous->next = current->next;
  }

  return current;
}

// A function that help delete Queue 2
node_t *deleteQueue2(char *customer_ID) {
  node_t *current = Queue2;
  node_t *previous = NULL;

  if (isEmpty(Queue2)) {
    return NULL;
  }

  while (strcmp(current->customer_ID, customer_ID) != 0) {
    if (current->next == NULL) {
      return NULL;
    } else {
      previous = current;
      current = current->next;
    }
  }

  if (current == Queue2) {
    Queue2 = Queue2->next;
  } else {
    previous->next = current->next;
  }

  return current;
}

// A function that help delete Queue 3
node_t *deleteQueue3(char *customer_ID) {
  node_t *current = Queue3;
  node_t *previous = NULL;

  if (isEmpty(Queue3)) {
    return NULL;
  }

  while (strcmp(current->customer_ID, customer_ID) != 0) {
    if (current->next == NULL) {
      return NULL;
    } else {
      previous = current;
      current = current->next;
    }
  }

  if (current == Queue3) {
    Queue3 = Queue3->next;
  } else {
    previous->next = current->next;
  }

  return current;
}

// A function that help looks for insertion of priority at each time quantum
void priorityQueue(int x, int y, char *queueName) {
  while (true) {
    node_t *foundLink = tierFind(x, y, worldTime);
    if (!isEmpty(foundLink)) {
      if (strcmp(queueName, "Queue1") == 0) {
        if (isEmpty(Queue1) && foundLink)
          preemption = true;
      }

      insertToQueue(queueName, foundLink);
      deleteHead(foundLink->customer_ID);
    } else {
      break;
    }
  }
}

// At every time quantum, check for insertion of the new arrival time
void getQueueData() {
  priorityQueue(5, 6, "Queue1"); // High Priority
  priorityQueue(3, 4, "Queue2"); // Medium Priority
  priorityQueue(1, 2, "Queue3"); // Low Priority
}

// A function that helps to move from queue x to y
void reOrderingQueues(char *queueName, char *queueDestination) {
  node_t *reorderQueue;

  if (strcmp(queueName, "Queue1") == 0) {
    reorderQueue = findSubQueue(queueName, Queue1->customer_ID);
  } else if (strcmp(queueName, "Queue2") == 0) {
    reorderQueue = findSubQueue(queueName, Queue2->customer_ID);
  } else if (strcmp(queueName, "Queue3") == 0) {
    reorderQueue = findSubQueue(queueName, Queue3->customer_ID);
  } else if (strcmp(queueName, "final") == 0) {
    reorderQueue = findSubQueue(queueName, Final_Output->customer_ID);
  }

  if (!isEmpty(reorderQueue)) {
    if (strcmp(queueName, "Queue1") == 0) {
      deleteQueue1(reorderQueue->customer_ID);
    } else if (strcmp(queueName, "Queue2") == 0) {
      deleteQueue2(reorderQueue->customer_ID);
    } else if (strcmp(queueName, "Queue3") == 0) {
      deleteQueue3(reorderQueue->customer_ID);
    }

    insertToQueue(queueDestination, reorderQueue);
  }
}

// A function that help checks for whether promotion is available for Queue 1 or
// Queue 2 based on their prority
node_t *checkPriority(char *queueName, node_t *current) {
  current->priority = current->priority + 1;
  current->age = 0;

  if (strcmp(queueName, "Queue2") == 0) {
    current->numOfTimes = 0;

    if (current->priority == 5) {
      node_t *reorderQueue2 = findSubQueue("Queue2", current->customer_ID);

      if (reorderQueue2 != NULL) {
        deleteQueue2(reorderQueue2->customer_ID);
        insertToQueue("Queue1", reorderQueue2);
      }
    }
  } else if (strcmp(queueName, "Queue3") == 0) {
    if (current->priority == 3) {
      node_t *reorderQueue3 = findSubQueue("Queue3", current->customer_ID);

      if (reorderQueue3 != NULL) {
        deleteQueue3(reorderQueue3->customer_ID);
        startInsert = true;
        insertToQueue("Queue2", reorderQueue3);
        startInsert = false;
      }
    }
  }

  return current;
}

// When Queue 1 is active, this function will increment Queue 2 and Queue 3 age
// based on how long they have been inside the loop for
void increaseTime(char *queueName) {
  node_t *current;

  if (strcmp(queueName, "Queue2") == 0)
    current = Queue2;

  else if (strcmp(queueName, "Queue3") == 0)
    current = Queue3;

  while (!isEmpty(current)) {
    if (!preemption)
      current->numOfTimes = current->numOfTimes + 1;

    if (isAgePriortized) {
      if (current->arrival_time != worldTime)
        current->age = current->age + 1;
      current->numOfTimes = 0;

      if (strcmp(queueName, "Queue2") == 0)
        if (current->priority == 5) {
          reOrderingQueues("Queue2", "Queue1");
        }

        else if (strcmp(queueName, "Queue3") == 0)
          if (current->priority == 3) {
            reOrderingQueues("Queue3", "Queue2");
          }
    }

    if (current->numOfTimes == 5) {
      current->age = current->age + 1;
      current->numOfTimes = 0;

    } else if (current->age == AGING_MECHANIC) {
      if (strcmp(queueName, "Queue2") == 0)
        current = checkPriority("Queue2", current);

      if (strcmp(queueName, "Queue3") == 0)
        current = checkPriority("Queue3", current);
    }

    if (current->age != 8)
      current = current->next;
  }

  if (preemption) {
    preemption = false;
  }
}

// Updates age only for selected Queue and reset counter for how long it has
// been in the loop
void updateAgeInQueues(node_t *current) {
  while (!isEmpty(current)) {
    current->age = current->age + 1;
    current->numOfTimes = 0;
    current = current->next;
  }
}

// Updates age only for selected Queue
void updateAgeOnlyInQueues(node_t *current) {
  while (!isEmpty(current)) {
    current->age = current->age + 1;
    current = current->next;
  }
}

// Whenever Queue 1 / Queue 2 and Queue 1 finishes a process call, update the
// top age and compare if it is possible for promotion
void checkQueueThree() {
  if (Queue3->numOfTimes == 10) {
    Queue3->age = Queue3->age + 1;
    Queue3->numOfTimes = 0;
  }

  if (Queue3->age == AGING_MECHANIC) {
    Queue3->age = 0;
    Queue3->priority = Queue3->priority + 1;
    Queue3->numOfTimes = 0;

    if (Queue3->priority == 5) {
      reOrderingQueues("Queue3", "Queue2");
    }
  } else {
    noChange = true;
    reOrderingQueues("Queue3", "Queue3");
    noChange = false;
  }
}

// A loop that updates in order
// -> Checks for new data to be added first
// -> Checks for any reordering for Q1->Q2->Q3
void checkForUpdate() {

  getQueueData();
  isAgePriortized = false;
  bool queue1Emptied = false;

  // When there are data in Queue 1 to handle
  if (!isEmpty(Queue1)) {
    // When there are no sudden interruption to Queue 1
    if (!preemption) {

      // This checks for if a process has recently started based on the arrival
      // time, otherwise the current process real time is incremented by one
      if (Queue1->arrival_time == worldTime)
        Queue1->readyTime = worldTime;
      else if (Queue1->arrival_time != worldTime)
        Queue1->runTime = Queue1->runTime + 1;

      if (Queue1->readyTime == -1)
        Queue1->readyTime = worldTime - 1;

      // This function checks for demotion or to be moved back to the end of the
      // current Queue based on their priority
      if ((Queue1->runTime / 25 > Queue1->twoFiveRuns) &&
          (Queue1->runTime != Queue1->total_time)) {
        Queue1->twoFiveRuns = Queue1->twoFiveRuns + 1;

        if (Queue1->priority == 6) {
          if (Queue1->fiveRuns != 5)
            Queue1->fiveRuns = 5;
        }

        Queue1->priority = Queue1->priority - 1;
        if (Queue1->next == NULL)
          Queue1->age = -1;
        else
          Queue1->age = 0;

        if (Queue1->priority < 5) {
          Queue1->fiveRuns = Queue1->fiveRuns + 1;
          reOrderingQueues("Queue1", "Queue2");

        } else if (Queue1->priority == 5) {
          reOrderingQueues("Queue1", "Queue1");
        }

      } else if (Queue1->runTime == Queue1->total_time) {
        reOrderingQueues("Queue1", "final");

        // Whenever Queue 1 finishes, this will allow Queue 2 and Queue 3 to age
        // up by 1 value
        isAgePriortized = true;

      } else if ((Queue1->runTime / 5) > (Queue1->fiveRuns)) {
        Queue1->fiveRuns = Queue1->fiveRuns + 1;
        reOrderingQueues("Queue1", "Queue1");
      }
      // When there exists a sudden interruption of Queue 1, Queue 2 and Queue 3
      // will update their age at the time of new arrivals
    } else {
      startInsert = true;
      Queue2->runTime = Queue2->runTime + 1;
      Queue2->age = 0;

      node_t *reorderQueue2 = findSubQueue("Queue2", Queue2->customer_ID);
      if (!isEmpty(reorderQueue2)) {
        deleteQueue2(Queue2->customer_ID);
        updateAgeInQueues(Queue2);
        insertToQueue("Queue2", reorderQueue2);
      }
      startInsert = false;
      updateAgeOnlyInQueues(Queue3);
    }

    if (isEmpty(Queue1)) {
      queue1Emptied = true;
      updateAgeInQueues(Queue2);
      updateAgeInQueues(Queue3);
    }
  }

  // When there are data in Queue 2 to handle
  // This function is split into 2 parts, where it first checks if Queue 1 is
  // empty or Queue 1 is not empty.
  if (!isEmpty(Queue2)) {
    if (isEmpty(Queue1)) {
      if (Queue2->readyTime == -1 && queue1Emptied) {
        Queue2->readyTime = worldTime;
        queue1Emptied = false;
      } else {

        if (Queue2->readyTime == -1)
          Queue2->readyTime = worldTime - 1;

        Queue2->runTime = Queue2->runTime + 1;
        Queue2->numOfTimes = Queue2->numOfTimes + 1;
        Queue2->age = 0;

        // Once a process finishes her CPU time, move the process ID to the
        // final output and update the rest of the current Queue 2's age to be 1
        // higher and check if promotion is possible
        if (Queue2->runTime == Queue2->total_time) {
          reOrderingQueues("Queue2", "final");

          updateAgeOnlyInQueues(Queue2);
          node_t *current = Queue3;

          while (!isEmpty(current)) {
            current->age = current->age + 1;
            current->numOfTimes = 0;

            if (current->age == AGING_MECHANIC) {
              current->age = 0;
              current->priority = current->priority + 1;
              current->numOfTimes = 0;

              if (current->priority == 3) {
                reOrderingQueues("Queue3", "Queue2");
              }
            }
            current = current->next;
          }

          if (!isEmpty(Queue3))
            Queue3->numOfTimes = -1;

          // Checks whether the current Queue2 is going to demote or go back to
          // the end of the Round Robin based on their priority and how many
          // times it has been in the loop for
        } else if (Queue2->numOfTimes == 10 || Queue2->numOfTimes == 20) {
          if (Queue2->numOfTimes == 20) {
            Queue2->priority = Queue2->priority - 1;

            if (Queue2->priority < 3) {
              Queue2->age = -1;
              Queue2->numOfTimes = -1;
              Queue2->runTime = Queue2->runTime - 1;
              Queue2->priority = 2;
              reOrderingQueues("Queue2", "Queue3");

              // For all Queue2 available
              if (!isEmpty(Queue2)) {
                updateAgeOnlyInQueues(Queue2);
              }

              // For all Queue3 available
              if (!isEmpty(Queue3)) {
                updateAgeOnlyInQueues(Queue3);
              }

            } else {
              Queue2->numOfTimes = 0;

              noChange = true;
              reOrderingQueues("Queue2", "Queue2");
              noChange = false;

              if (!isEmpty(Queue2))
                Queue2->age = Queue2->age + 1;

              if (!isEmpty(Queue3))
                checkQueueThree();
            }
          } else {
            noChange = true;
            reOrderingQueues("Queue2", "Queue2");
            noChange = false;

            if (!isEmpty(Queue2))
              Queue2->age = Queue2->age + 1;

            if (!isEmpty(Queue3))
              checkQueueThree();
          }
        }
      }
    } else if (!isEmpty(Queue1)) {
      increaseTime("Queue2");
    }

    // Double check if Queue 2 is empty during run time
    if (isEmpty(Queue2)) {
      queueTwoEmptied = true;

      node_t *current = Queue3;
      while (!isEmpty(current)) {
        current->numOfTimes = 0;
        current = current->next;
      }
    }
  }

  // When there are data in Queue 3 to handle
  if (!isEmpty(Queue3)) {
    if (isEmpty(Queue1) && (!isEmpty(Queue2))) {
      Queue3->numOfTimes = Queue3->numOfTimes + 1;
    } else if (isEmpty(Queue1) && isEmpty(Queue2)) {

      if (Queue3->next != NULL)
        queueThreeLengthOne = false;

      if (Queue3->readyTime == -1)
        Queue3->readyTime = worldTime;

      if (Queue3->readyTime == worldTime) {
        Queue3->runTime = -1;
      }

      Queue3->runTime = Queue3->runTime + 1;
      Queue3->numOfTimes = Queue3->numOfTimes + 1;

      if (queueTwoEmptied) {
        queueTwoEmptied = false;
      } else
        Queue3->age = 0;

      if (Queue3->runTime == Queue3->total_time) {
        if (!queueThreeLengthOne && Queue3->next == NULL) {
          lastInsert = true;
          Queue3->terminationTime = worldTime + 1;
        }

        reOrderingQueues("Queue3", "final");
        node_t *current = Queue3;
        while (current != NULL) {
          current->runTime = current->runTime + 1;
          current->age = current->age + 1;
          current = current->next;
        }
      } else if ((Queue3->numOfTimes == 20)) {
        Queue3->numOfTimes == 0;

        noChange = true;
        node_t *reorderQueue3 = findSubQueue("Queue3", Queue3->customer_ID);
        if (!isEmpty(reorderQueue3)) {
          deleteQueue3(Queue3->customer_ID);
          updateAgeOnlyInQueues(Queue3);
          insertToQueue("Queue3", reorderQueue3);
        }
        noChange = false;
      }

    } else {
      increaseTime("Queue3");
    }
  }
}

// Function that reads data from file
char *readFile(char *fileName) {
  FILE *file = fopen(fileName, "r");
  char *code = malloc(MAX_SIZE);
  size_t n = 0;
  int c;

  if (file == NULL)
    return NULL;

  while ((c = fgetc(file)) != EOF) {
    code[n++] = (char)c;
  }

  code[n] = '\0';
  return code;
}

// Main driven code
int main(int argc, char *argv[]) {
  char *text = readFile(argv[1]);
  char *av[MAX_SIZE];

  // Reads input and split into tokens for storing to Linked List
  int ac = splitToBlocks(text, av);
  for (int i = 0; i < ac; i += 5) {
    char *ID = av[i + 0];
    int aTime = atoi(av[i + 1]);
    int priority = atoi(av[i + 2]);
    int age = atoi(av[i + 3]);
    int tTime = atoi(av[i + 4]);
    insertHead(ID, aTime, priority, age, tTime);
  }

  // Initialise values for time 0
  getQueueData();
  printData();
  preemption = false;

  while (true) {
    checkForUpdate();
    printData();

    if (isEmpty(Queue1) && isEmpty(Queue2) && isEmpty(Queue3) && isEmpty(head))
      break;
  }

  if (debug)
    printf("\nThey are finished processes:\n");

  printFinal();
  return 0;
}