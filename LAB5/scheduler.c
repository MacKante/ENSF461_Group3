#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>

// TODO: Add more fields to this struct
struct job {
  int id;
  int arrival;
  int remainingTime; // time remaining
  int tickets;
  struct job *next;

  int length;     // store length
  int startTime;  // start timer
  int endTime;    // end timer
  
};

/*** Globals ***/ 
int seed = 100;

void analyze(struct job* head);
int isCompleted(struct job* head);
struct job* findShortest(struct job* head, int timer);

//This is the start of our linked list of jobs, i.e., the job list
struct job *head = NULL;

/*** Globals End ***/

/*Function to append a new job to the list*/
void append(int id, int arrival, int length, int tickets){
  // create a new struct and initialize it with the input data
  struct job *tmp = (struct job*) malloc(sizeof(struct job));

  //tmp->id = numofjobs++;
  tmp->id = id;
  tmp->remainingTime = length;
  tmp->arrival = arrival;
  tmp->tickets = tickets;

  tmp->length = length;
  tmp->startTime = -1;
  tmp->endTime = 0;

  // the new job is the last job
  tmp->next = NULL;

  // Case: job is first to be added, linked list is empty 
  if (head == NULL){
    head = tmp;
    return;
  }

  struct job *prev = head;

  //Find end of list 
  while (prev->next != NULL){
    prev = prev->next;
  }

  //Add job to end of list 
  prev->next = tmp;
  return;
}

/*Function to read in the workload file and create job list*/
void read_workload_file(char* filename) {
  int id = 0;
  FILE *fp;
  size_t len = 0;
  ssize_t read;
  char *line = NULL,
       *arrival = NULL, 
       *length = NULL;
  int tickets = 0;

  struct job **head_ptr = malloc(sizeof(struct job*));

  if( (fp = fopen(filename, "r")) == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) > 1) {
    arrival = strtok(line, ",\n");
    length = strtok(NULL, ",\n");
    tickets += 100;
       
    // Make sure neither arrival nor remainingTime are null. 
    assert(arrival != NULL && length != NULL);
        
    append(id++, atoi(arrival), atoi(length), tickets);
  }

  fclose(fp);

  // Make sure we read in at least one job
  assert(id > 0);

  return;
}

/*------------------------------------------------------*/

void policy_STCF(struct job* head, int slice) {
  printf("Execution trace with STCF:\n");
  int timer = 0;

  while(!isCompleted(head)){
    struct job* current = findShortest(head, timer); // find shortest job

    // set start time
    if(current->startTime == -1){
      current->startTime = timer;
    }

    // If remaining time is more than slice, run for slice
    if(current->remainingTime > slice) {
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n",
              timer, current->id, current->arrival, slice);
      current->remainingTime -= slice;
      timer += slice;

    // If remaining time is not 0, but not more than slice
    // (job is not completed), then do final run
    } else if(current->remainingTime != 0){
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, current->remainingTime);
      timer += current->remainingTime;
      current->remainingTime = 0;

      current->endTime = timer; // log end time
    }
  }
  printf("End of execution with STCF.\n");
  return;  
}

/*------------------------------------------------------*/

void policy_RR(struct job* head, int slice) {
  printf("Execution trace with RR:\n");
  struct job* current = head;
  int timer = 0;

  while(!isCompleted(head)) {
    // Set Start Time
    if(current->startTime == -1){
      current->startTime = timer;
    }

    // If remaining time is more than slice, run for slice
    if(current->remainingTime > slice) {
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, slice);
      current->remainingTime -= slice;
      timer += slice;

    // If remaining time is not 0, but not more than slice
    // (job is not completed), then do final run
    } else if(current->remainingTime != 0){
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, current->remainingTime);
      timer += current->remainingTime;
      current->remainingTime = 0;

      current->endTime = timer;
    }

    // Move to the next job cyclically
    if(current->next != NULL) {
      current = current->next;
    } else {
      current = head;
    }
  }
  printf("End of execution with RR.\n");

  return;
}

/*------------------------------------------------------*/

void policy_LT(struct job *head, int slice) {
  printf("Execution trace with LT:\n");
  srand(seed);

  int timer = 0;
  int ticketCount = 0;

  // Give Tickets
  struct job* current = head;
  for(int i = 1; current != NULL; i++) {
    current->tickets = i * 100;
    ticketCount += i * 100;
    current = current->next;
  }

  while(!isCompleted(head)){
    int winner = (rand() % (ticketCount)) + 1; // random number generator in tickets
    
    // Find Job that the ticket belongs to
    current = head;
    int ticketIndex = current->tickets;
    while (winner > ticketIndex) {
      current = current->next;
      ticketIndex += current->tickets;
    }

    // Set Start Timer
    if(current->startTime == -1){
      current->startTime = timer;
    }

    // If remaining time is more than slice, run for slice
    if(current->remainingTime > slice) {
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, slice);
      timer += slice;
      current->remainingTime -= slice;

    // If remaining time is not 0, but not more than slice
    // (job is not completed), then do final run
    } else if(current->remainingTime != 0){
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, current->remainingTime);
      timer += current->remainingTime;
      current->remainingTime = 0;
      current->endTime = timer;

      ticketCount -= current->tickets;
      current->tickets = 0;
    }
  }

  printf("End of execution with LT.\n");
  return;
}

/*------------------------------------------------------*/

int main(int argc, char **argv) {

 if (argc < 5) {
    fprintf(stderr, "missing variables\n");
    fprintf(stderr, "usage: %s analysis-flag policy workload-file slice-length\n", argv[0]);
		exit(EXIT_FAILURE);
  }

  int analysis = atoi(argv[1]);
  char *policy = argv[2],
       *workload = argv[3];
  int slice = atoi(argv[4]);

  // Note: we use a global variable to point to 
  // the start of a linked-list of jobs, i.e., the job list 
  read_workload_file(workload);

  if (strcmp(policy, "STCF") == 0 ) {
    policy_STCF(head, slice);
    if (analysis) {
      printf("Begin analyzing STCF:\n");
      analyze(head);
      printf("End analyzing STCF.\n");
    }

    exit(EXIT_SUCCESS);
  }

  if (strcmp(policy, "RR") == 0 ) {
    policy_RR(head, slice);
    if (analysis) {
      printf("Begin analyzing RR:\n");
      analyze(head);
      printf("End analyzing RR.\n");
    }

    exit(EXIT_SUCCESS);
  }

  if (strcmp(policy, "LT") == 0 ) {
    policy_LT(head, slice);
    if (analysis) {
      printf("Begin analyzing LT:\n");
      analyze(head);
      printf("End analyzing LT.\n");
    }

    exit(EXIT_SUCCESS);
  } 

	exit(EXIT_SUCCESS);
}

// Check if the jobs are completed
  // Return 0 if incomplete
  // Return 1 if complete
int isCompleted(struct job* head){
  struct job* temp = head;
  while(1){
    if(temp->remainingTime != 0){
      return 0;
    } else if (temp->next != NULL) {
      temp = temp->next;
    } else {
      break;
    }
  }
  return 1;
}

// Finds Shortest Job in list of arrived jobs 
  // pick from jobs that have arrived (arrival < timer)
  // that is not completed (remaining time is not 0)
  // Returns pointer to shortest job
struct job* findShortest(struct job* head, int timer){
  struct job* current = head;
  int minRemainingTime = INT_MAX;
  struct job *shortestJob = NULL;
  while (current != NULL) {
    int TTC = current->arrival + current->remainingTime;
    if (TTC < minRemainingTime && current->remainingTime != 0 
        && timer >= current->arrival) {
        minRemainingTime = current->remainingTime;
        shortestJob = current;
    }
    current = current->next;
  }

  return shortestJob;
}

// Analyze Function (All analyzations are the same)
void analyze(struct job *head) {
  float totalResponse = 0;
  float totalTurnaround = 0;
  float totalWait = 0;
  
  int i = 0;

  struct job* current = head;

  while(current != NULL) {
    int response = current->startTime - current->arrival;
    int turnaround = current->endTime - current->arrival;
    int wait =  turnaround - current->length;
    
    printf("Job %d -- Response time: %d  Turnaround: %d  Wait: %d\n",
            current->id, response, turnaround, wait);

    totalResponse += response;
    totalTurnaround += turnaround;
    totalWait += wait;
    current = current->next;
    i++;
  }
  
  float aveResponse = totalResponse / i;
  float aveTurnaround = totalTurnaround / i;
  float aveWait = totalWait / i;
  printf("Average -- Response: %.2f  Turnaround %.2f  Wait %.2f\n",
          aveResponse, aveTurnaround, aveWait);
  return;
}