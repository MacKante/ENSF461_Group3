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
  int length;
  int tickets;
  struct job *next;

  int wait;       // amount of timer waited
  int startTime;  // start timer
  int endTime;    // end timer
  int runCount;   // times run
  
};

/*** Globals ***/ 
int seed = 100;
int isCompleted(struct job* head);

//This is the start of our linked list of jobs, i.e., the job list
struct job *head = NULL;

/*** Globals End ***/

/*Function to append a new job to the list*/
void append(int id, int arrival, int length, int tickets){
  // create a new struct and initialize it with the input data
  struct job *tmp = (struct job*) malloc(sizeof(struct job));

  //tmp->id = numofjobs++;
  tmp->id = id;
  tmp->length = length;
  tmp->arrival = arrival;
  tmp->tickets = tickets;

  tmp->wait = 0;
  tmp->startTime = -1;
  tmp->endTime = length;
  tmp->runCount = 0;

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
       
    // Make sure neither arrival nor length are null. 
    assert(arrival != NULL && length != NULL);
        
    append(id++, atoi(arrival), atoi(length), tickets);
  }

  fclose(fp);

  // Make sure we read in at least one job
  assert(id > 0);

  return;
}

/*---------------------------------------------------*/

void policy_STCF(struct job* head, int slice) {
  // TO DO FIXN THIS 
    struct job* current = head;
    struct job* next_job = NULL;
    int timer = 0;

    while (current != NULL) {
        // Find the job with the shortest remaining timer
        next_job = head;
        while (next_job != NULL) {
            if (next_job->arrival <= timer) {
                if (next_job->length < current->length) {
                    current = next_job;
                }
            }
            next_job = next_job->next;
        }

        // Execute the current job for 'slice' timer units
        if (current->length > slice) {
            current->length -= slice;
            timer += slice;
            current->wait += slice;
        } else {
            timer += current->length;
            current->length = 0;
        }

        // Move to the next timer unit
        current = current->next;
    }
}
void analyze_STCF(struct job *head) {
  // TODO: Fill this in

  return;
}

/*---------------------------------------------------*/

void policy_RR(struct job* head, int slice) {
  printf("Execution trace with RR:\n");
  struct job* current = head;
  int timer = 0;

  while(isCompleted(head) != 1) {
    if(timer < current->startTime || current->startTime == -1){
      current->startTime = timer;
    }

    if(current->length > slice) {
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, slice);
      current->length -= slice;
      timer += slice;
      current->wait += 1;

    } else if(current->length != 0){
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, current->length);
      timer += current->length;
      current->wait *= slice;
      current->length = 0;
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

void analyze_RR(struct job *head) {
  float totalResponse = 0;
  float totalTurnaround = 0;
  float totalWait = 0;
  
  int i = 0;

  struct job* current = head;

  while(current != NULL) {
    int response = current->startTime - current->arrival;
    int turnaround = current->endTime + current->wait;
    
    printf("Job %d -- Response time: %d Turnaround: %d Wait: %d\n",
            current->id, response, turnaround, current->wait);

    totalResponse += response;
    totalTurnaround += turnaround;
    totalWait += current->wait;
    current = current->next;
    i++;
  }
  
  float aveResponse = totalResponse / i;
  float aveTurnaround = totalTurnaround / i;
  float aveWait = totalWait / i;
  printf("Average -- Response: %.2f Turnaround %.2f Wait %.2f\n",
          aveResponse, aveTurnaround, aveWait);
  return;
}

/*------------------------------------------------------*/

void policy_LT(struct job *head, int slice) {
  printf("Execution trace with LT:\n");

  int timer = 0;

  struct job* current = head;
  int ticketCount = 0;
  for(int i = 1; current != NULL; i++) {
    current->tickets = i * 100;
    ticketCount += i * 100;
    current = current->next;
  }

  // [1 - 100] [101 - 300] [301 - 600] [601 - 1000]
  

  srand(seed);
  
  while(isCompleted(head) != 1){
    int winner = (rand() % (ticketCount)) + 1;
    current = head;

    int ticketIndex = current->tickets;
    while (winner > ticketIndex) {
      current = current->next;
      ticketIndex += current->tickets;
    }

    
    if(timer < current->startTime || current->startTime == -1){
      current->startTime = timer;
    }

    if(current->length > slice) {
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, slice);

      current->length -= slice;
      timer += slice;
      current->runCount += 1;

    } else if(current->length != 0){
      printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", 
              timer, current->id, current->arrival, current->length);
      timer += current->length;
      current->runCount += 1;
      current->wait = timer - (current->runCount * slice);
      current->length = 0;
    }
  }

  printf("End of execution with LT.\n");
  return;
}

void analyze_LT(struct job *head) {
  float totalResponse = 0;
  float totalTurnaround = 0;
  float totalWait = 0;
  
  int i = 0;

  struct job* current = head;

  while(current != NULL) {
    int response = current->startTime - current->arrival;
    int turnaround = current->endTime + current->wait;
    
    printf("Job %d -- Response time: %d Turnaround: %d Wait: %d\n",
            current->id, response, turnaround, current->wait);

    totalResponse += response;
    totalTurnaround += turnaround;
    totalWait += current->wait;
    current = current->next;
    i++;
  }
  
  float aveResponse = totalResponse / i;
  float aveTurnaround = totalTurnaround / i;
  float aveWait = totalWait / i;
  printf("Average -- Response: %.2f Turnaround %.2f Wait %.2f\n",
          aveResponse, aveTurnaround, aveWait);
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
      analyze_STCF(head);
      printf("End analyzing STCF.\n");
    }

    exit(EXIT_SUCCESS);
  }

  // TODO: Add other policies

  if (strcmp(policy, "RR") == 0 ) {
    policy_RR(head, slice);
    if (analysis) {
      printf("Begin analyzing RR:\n");
      analyze_RR(head);
      printf("End analyzing RR.\n");
    }

    exit(EXIT_SUCCESS);
  }

  if (strcmp(policy, "LT") == 0 ) {
    policy_LT(head, slice);
    if (analysis) {
      printf("Begin analyzing LT:\n");
      analyze_LT(head);
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
    if(temp->length != 0){
      return 0;
    } else if (temp->next != NULL) {
      temp = temp->next;
    } else {
      break;
    }
  }
  return 1;
}
