/**
 * Assignment #3: Strings, structs, pointers, command-line arguments.
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>


// Limit constants
#define MAX_CITY_NAME_LEN 20
#define MAX_FLIGHTS_PER_CITY 5
#define MAX_DEFAULT_SCHEDULES 50

// Time definitions
#define TIME_MIN 0
#define TIME_MAX ((60 * 24)-1)
#define TIME_NULL -1


/******************************************************************************
 * Structure and Type definitions                                             *
 ******************************************************************************/
typedef int time_t;                        // integers used for time values
typedef char city_t[MAX_CITY_NAME_LEN+1];; // null terminate fixed length city
 
// Structure to hold all the information for a single flight
//   A city's schedule has an array of these
struct flight {
  time_t time;       // departure time of the flight
  int available;  // number of seats currently available on the flight
  int capacity;   // maximum seat capacity of the flight
};

// Structure for an individual flight schedule
// The main data structure of the program is an Array of these structures
// Each structure will be placed on one of two linked lists:
//                free or active
// Initially the active list will be empty and all the schedules
// will be on the free list.  Adding a schedule is finding the first
// free schedule on the free list, removing it from the free list,
// setting its destination city and putting it on the active list
struct flight_schedule {
  city_t destination;                          // destination city name
  struct flight flights[MAX_FLIGHTS_PER_CITY]; // array of flights to the city
  struct flight_schedule *next;                // link list next pointer
  struct flight_schedule *prev;                // link list prev pointer
};

/******************************************************************************
 * Global / External variables                                                *
 ******************************************************************************/
// This program uses two global linked lists of Schedules.  See comments
// of struct flight_schedule above for details
struct flight_schedule *flight_schedules_free = NULL;
struct flight_schedule *flight_schedules_active = NULL;


/******************************************************************************
 * Function Prototypes                                                        *
 ******************************************************************************/
// Misc utility io functions
int city_read(city_t city);           
bool time_get(time_t *time_ptr);      
bool flight_capacity_get(int *capacity_ptr);
void print_command_help(void);

// Core functions of the program
void flight_schedule_initialize(struct flight_schedule array[], int n);
struct flight_schedule * flight_schedule_find(city_t city);
struct flight_schedule * flight_schedule_allocate(void);
void flight_schedule_free(struct flight_schedule *fs);
void flight_schedule_add(city_t city);
void flight_schedule_listAll(void);
void flight_schedule_list(city_t city);
void flight_schedule_add_flight(city_t city);
void flight_schedule_remove_flight(city_t city);
void flight_schedule_schedule_seat(city_t city);
void flight_schedule_unschedule_seat(city_t city);
void flight_schedule_remove(city_t city);

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs);
int  flight_compare_time(const void *a, const void *b);


int main(int argc, char *argv[]) 
{
  long n = MAX_DEFAULT_SCHEDULES;
  char command;
  city_t city;

  if (argc > 1) {
    // If the program was passed an argument then try and convert the first
    // argument in the a number that will override the default max number
    // of schedule we will support
    char *end;
    n = strtol(argv[1], &end, 10); // CPAMA p 787
    if (n==0) {
      printf("ERROR: Bad number of default max scedules specified.\n");
      exit(EXIT_FAILURE);
    }
  }

  struct flight_schedule flight_schedules[n];
 
  // Initialize our global lists of free and active schedules using
  // the elements of the flight_schedules array
  flight_schedule_initialize(flight_schedules, n);

  assert(flight_schedules_free != NULL && flight_schedules_active == NULL);

  // Print the instruction in the beginning
  print_command_help();

  // Command processing loop
  while (scanf(" %c", &command) == 1) {
    switch (command) {
    case 'A': 
      //  Add an active flight schedule for a new city eg "A Toronto\n"
      city_read(city);
      flight_schedule_add(city);

      break;
    case 'L':
      // List all active flight schedules eg. "L\n"
      flight_schedule_listAll();
      break;
    case 'l': 
      // List the flights for a particular city eg. "l\n"
      city_read(city);
      flight_schedule_list(city);
      break;
    case 'a':
      // Adds a flight for a particular city "a Toronto\n
      //                                      360 100\n"
      city_read(city);
      flight_schedule_add_flight(city);
      break;
    case 'r':
      // Remove a flight for a particular city "r Toronto\n
      //                                        360\n"
      city_read(city);
      flight_schedule_remove_flight(city);
	break;
    case 's':
      // schedule a seat on a flight for a particular city "s Toronto\n
      //                                                    300\n"
      city_read(city);
      flight_schedule_schedule_seat(city);
      break;
    case 'u':
      // unschedule a seat on a flight for a particular city "u Toronto\n
      //                                                      360\n"
        city_read(city);
        flight_schedule_unschedule_seat(city);
        break;
    case 'R':
      // remove the schedule for a particular city "R Toronto\n"
      city_read(city);
      flight_schedule_remove(city);  
      break;
    case 'h':
        print_command_help();
        break;
    case 'q':
      goto done;
    default:
      printf("Bad command. Use h to see help.\n");
    }
  }
 done:
  return EXIT_SUCCESS;
}

/**********************************************************************
 * city_read: Takes in and processes a given city following a command *
 *********************************************************************/
int city_read(city_t city) {
  int ch, i=0;

  // skip leading non letter characters
  while (true) {
    ch = getchar();
    if ((ch >= 'A' && ch <= 'Z') || (ch >='a' && ch <='z')) {
      city[i++] = ch;
      break;
    }
  }
  while ((ch = getchar()) != '\n') {
    if (i < MAX_CITY_NAME_LEN) {
      city[i++] = ch;
    }
  }
  city[i] = '\0';
  return i;
}


/****************************************************************
 * Message functions so that your messages match what we expect *
 ****************************************************************/
void msg_city_bad(char *city) {
  printf("No schedule for %s\n", city);
}

void msg_city_exists(char *city) {
  printf("There is a schedule of %s already.\n", city);
}

void msg_schedule_no_free(void) {
  printf("Sorry no more free schedules.\n");
}

void msg_city_flights(char *city) {
  printf("The flights for %s are:", city);
}

void msg_flight_info(int time, int avail, int capacity) {
  printf(" (%d, %d, %d)", time, avail, capacity);
}

void msg_city_max_flights_reached(char *city) {
  printf("Sorry we cannot add more flights on this city.\n");
}

void msg_flight_bad_time(void) {
  printf("Sorry there's no flight scheduled on this time.\n");
}

void msg_flight_no_seats(void) {
    printf("Sorry there's no more seats available!\n");
}

void msg_flight_all_seats_empty(void) {
  printf("All the seats on this flights are empty!\n");
}

void msg_time_bad() {
  printf("Invalid time value\n");
}

void msg_capacity_bad() {
  printf("Invalid capacity value\n");
}

void print_command_help()
{
  printf("Here are the possible commands:\n"
	 "A <city name>     - Add an active empty flight schedule for\n"
	 "                    <city name>\n"
	 "L                 - List cities which have an active schedule\n"
	 "l <city name>     - List the flights for <city name>\n"
	 "a <city name>\n"
         "<time> <capacity> - Add a flight for <city name> @ <time> time\n"
	 "                    with <capacity> seats\n"  
	 "r <city name>\n"
         "<time>            - Remove a flight form <city name> whose time is\n"
	 "                    <time>\n"
	 "s <city name>\n"
	 "<time>            - Attempt to schedule seat on flight to \n"
	 "                    <city name> at <time> or next closest time on\n"
	 "                    which their is an available seat\n"
	 "u <city name>\n"
	 "<time>            - unschedule a seat from flight to <city name>\n"
	 "                    at <time>\n"
	 "R <city name>     - Remove schedule for <city name>\n"
	 "h                 - print this help message\n"
	 "q                 - quit\n"
);
}


/****************************************************************
 * Resets a flight schedule                                     *
 ****************************************************************/
void flight_schedule_reset(struct flight_schedule *fs) {
    fs->destination[0] = 0;
    for (int i=0; i<MAX_FLIGHTS_PER_CITY; i++) {
      fs->flights[i].time = TIME_NULL;
      fs->flights[i].available = 0;
      fs->flights[i].capacity = 0;
    }
    fs->next = NULL;
    fs->prev = NULL;
}

/******************************************************************
* Initializes the flight_schedule array that will hold any flight *
* schedules created by the user. This is called in main for you.  *
 *****************************************************************/

void flight_schedule_initialize(struct flight_schedule array[], int n)
{
  flight_schedules_active = NULL;
  flight_schedules_free = NULL;

  // takes care of empty array case
  if (n==0) return;

  // Loop through the Array connecting them
  // as a linear doubly linked list
  if (n == 1) {
    flight_schedule_reset(&array[0]);
    array[0].next = NULL;
    array[0].prev = NULL;
    flight_schedules_free = &array[0];
    return;
  }

  flight_schedule_reset(&array[0]);
  array[0].next = &array[1];
  array[0].prev = NULL;

  for (int i=1; i<n-1; i++) {
    flight_schedule_reset(&array[i]);
    array[i].next = &array[i+1];
    array[i].prev = &array[i-1];
  }

  // Takes care of last node.  
  flight_schedule_reset(&array[n-1]); // reset clears all fields
  array[n-1].next = NULL;
  array[n-1].prev = &array[n-2];
  flight_schedules_free = &array[0];
}

/***********************************************************
 * time_get: read a time from the user
   Time in this program is a minute number 0-((24*60)-1)=1439
   -1 is used to indicate the NULL empty time 
   This function should read in a time value and check its 
   validity.  If it is not valid eg. not -1 or not 0-1439
   It should print "Invalid Time" and return false.
   othewise it should return the value in the integer pointed
   to by time_ptr.
 ***********************************************************/
bool time_get(int *time_ptr) {
  if (scanf("%d", time_ptr)==1) {
    return (TIME_NULL == *time_ptr || 
	    (*time_ptr >= TIME_MIN && *time_ptr <= TIME_MAX));
  } 
  msg_time_bad();
  return false;
}

/***********************************************************
 * flight_capacity_get: read the capacity of a flight from the user
   This function should read in a capacity value and check its 
   validity.  If it is not greater than 0, it should print 
   "Invalid capacity value" and return false. Othewise it should 
   return the value in the integer pointed to by cap_ptr.
 ***********************************************************/
bool flight_capacity_get(int *cap_ptr) {
  if (scanf("%d", cap_ptr)==1) {
    return *cap_ptr > 0;
  }
  msg_capacity_bad();
  return false;
}

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs) 
{
  qsort(fs->flights, MAX_FLIGHTS_PER_CITY, sizeof(struct flight),
	flight_compare_time);
}

int flight_compare_time(const void *a, const void *b) 
{
  const struct flight *af = a;
  const struct flight *bf = b;
  
  return (af->time - bf->time);
}

// This helper function takes a blank flight_schedule off the free list and onto the active list. Used in flight_schedule_add.
struct flight_schedule *flight_schedule_allocate(void) {
  struct flight_schedule *move = flight_schedules_free;
  if (move == NULL) {
    msg_schedule_no_free();
    return NULL;
  }

  if (move->next == NULL) {
    flight_schedules_free = NULL;
  } else {
    flight_schedules_free = flight_schedules_free->next;
    move->next = NULL;
    flight_schedules_free->prev = NULL;
  }

  if (flight_schedules_active == NULL) {
    flight_schedules_active = move;
    return move;
  }

  move->next = flight_schedules_active;
  flight_schedules_active->prev = move;
  flight_schedules_active = move;

  return move;
  } 

// This helper function takes the flight schedule of the city passed into remove and removes it from the active list, resets it, and puts it back onto the free list. Used in flight_schedule_remove.
void flight_schedule_free(struct flight_schedule *fs) {
  if (fs->prev == NULL) {
    if (fs->next == NULL) {
      flight_schedules_active = NULL;
    } else {
      flight_schedules_active = flight_schedules_active->next;
      flight_schedules_active->prev = NULL;
      fs->next = NULL;
    }
      flight_schedule_reset(fs);
      if (flight_schedules_free == NULL) {
        flight_schedules_free = fs;
      } else {
        flight_schedules_free->prev = fs;
        fs->next = flight_schedules_free;
        flight_schedules_free = fs;
      }
  } else if (fs->next == NULL && fs->prev != NULL) {
    fs->prev->next = NULL;
    fs->prev = NULL;
    flight_schedule_reset(fs);
    if (flight_schedules_free == NULL) {
      flight_schedules_free = fs;
    } else {
      fs->next = flight_schedules_free;
      flight_schedules_free->prev = fs;
      flight_schedules_free = fs;
    }
  } else {
      fs->prev->next = fs->next;
      fs->next->prev = fs->prev;
      fs->next = NULL;
      fs->prev = NULL;
      flight_schedule_reset(fs);
      if (flight_schedules_free == NULL) {
        flight_schedules_free = fs;
      } else {
        fs->next = flight_schedules_free;
        flight_schedules_free->prev = fs;
        flight_schedules_free = fs;
    }
  }
}

// This function is used extensivelky throughout the program as it finds and returns a pointer to the flight schedule of a specific city. Returns NULL if a schedule for that city doesn't exist
struct flight_schedule *flight_schedule_find(city_t city) {
  struct flight_schedule *temp = flight_schedules_active;
  if (temp == NULL) {
      return NULL;
    }
  while (strncmp(city, temp->destination, sizeof(city)/sizeof(city[0])) != 0) {
    temp = temp->next;
    if (temp == NULL) {
      return NULL;
    }
  }
  return temp;
}

// This is the main fucntion that adds a flight schedule for a specifc city to the active list.
void flight_schedule_add(city_t city) {
  if (flight_schedules_free == NULL) {
    msg_schedule_no_free();
    return;
  } 
  if (flight_schedule_find(city) != NULL) {
    msg_city_exists(city);
    return;
  }
  struct flight_schedule *temp = flight_schedule_allocate();
  strncpy(temp->destination, city, sizeof(temp->destination));
}

// This is the main fucntion that removes a flight schedule for a specifc city from the active list, essentially deleting it.
void flight_schedule_remove(city_t city) {
  if (flight_schedules_active == NULL) { 
    msg_city_bad(city);
    return;
  }
  
  struct flight_schedule *sched = flight_schedule_find(city);

  if (sched == NULL) {
    msg_city_bad(city);
    return;
  }
  flight_schedule_free(sched);
}

// This function passes through the entire active list and prints the city names of each flight schedule
void flight_schedule_listAll(void) {
  if (flight_schedules_active == NULL) {
    return;
  } else {
    struct flight_schedule *temp = flight_schedules_active;
    while (temp != NULL) {
      printf("%s\n", &(temp->destination));
      temp = temp->next;
    }
  } 
}

// This function finds the flight schedule of a specific city and then prints each flight in its flight list with the format (time, available seats, total capacity)
void flight_schedule_list(city_t city) { 
  struct flight_schedule *temp = flight_schedule_find(city);
  if (temp == NULL) {
    msg_city_bad(city);
    return;
  }
  msg_city_flights(temp->destination);
  int x = 0;
  while (temp->flights[x].time == TIME_NULL) {
    x++;
    if (x == MAX_FLIGHTS_PER_CITY) {
      printf("\n");
      return;
    }
  }
  for (int i = x; i < MAX_FLIGHTS_PER_CITY; i++) {
    if (temp->flights[i].time == TIME_NULL) {
      printf("\n");
      return;
    }
    msg_flight_info(temp->flights[i].time,temp->flights[i].available,temp->flights[i].capacity);
  }
  printf("\n");
}

// This function finds the flight schedule of city, if it exists, and then adds a flight with its own time and capacity to the flights array in the flight schedule struct, if there is space for it
void flight_schedule_add_flight(city_t city) {
  struct flight_schedule *dest = flight_schedule_find(city);
  if (dest == NULL) {
    msg_city_bad(city);
    return;
  }
  
  time_t time; 
  int capacity;
  if (time_get(&time) == false || flight_capacity_get(&capacity) == false) {
    return;
  }
  
  if (dest->flights[0].time != TIME_NULL) {
    msg_city_max_flights_reached(city);
    return;
  }
  for (int i = 0; i < MAX_FLIGHTS_PER_CITY; i++) {
    if (dest->flights[i].time == TIME_NULL) {
      dest->flights[i].time = time;
      dest->flights[i].available = capacity;
      dest->flights[i].capacity = capacity;
      flight_schedule_sort_flights_by_time(dest) ;
      return;
    }
  }
  return;
}

// This function finds the flight schedule of city, if it exists, and then removes a flight with a specifc time from the flights array in the flight schedule struct
void flight_schedule_remove_flight(city_t city) {
  struct flight_schedule *dest = flight_schedule_find(city);
  if (dest == NULL) {
    msg_city_bad(city);
    return;
  } 
  
  time_t time;
  int i = 0;
  if (time_get(&time) == false) {
    return;
  }
  
  while (dest->flights[i].time != time) {
    i++;
    if (i == MAX_FLIGHTS_PER_CITY) {
      msg_flight_bad_time();
      return;
    }
  }
  dest->flights[i].time = TIME_NULL;
  dest->flights[i].available = 0;
  dest->flights[i].capacity = 0;
  flight_schedule_sort_flights_by_time(dest);
}

// This function finds the flight schedule of city, if it exists, and then finds the flight with a specifc time or the next flight after it, and schedules a seat on that flight if there are any available
void flight_schedule_schedule_seat(city_t city) {
  time_t time; 
  int i = 0;
  if (time_get(&time) == false) {
    return;
  }
  struct flight_schedule *dest = flight_schedule_find(city);
  if (dest == NULL) {
    msg_city_bad(city);
    return;
  }
  for (i = 0; i < MAX_FLIGHTS_PER_CITY; i++) {
    if (dest->flights[i].time == time) {
      break;
    } else if (dest->flights[i].time > time) {
      break;
    } else if (i == MAX_FLIGHTS_PER_CITY-1) {
      msg_flight_no_seats();
      return;
    }
  }
  if (dest->flights[i].available == 0) {
      msg_flight_no_seats();
      return;
  }
  dest->flights[i].available--;
  return;
}

// This function finds the flight schedule of city, if it exists, and then finds the flight with a specifc time, and unschedules a seat on that flight by increasing the available count if it is not empty
void flight_schedule_unschedule_seat(city_t city) {
  time_t time;
  int i = 0;
  if (time_get(&time) == false) {
    return;
  }
  struct flight_schedule *dest = flight_schedule_find(city);
  if (dest == NULL) {
    msg_city_bad(city);
    return;
  }
  int x = 0;
  while (dest->flights[x].time == TIME_NULL) {
    x++;
    if (x == MAX_FLIGHTS_PER_CITY) {
      msg_city_bad(city);
      return;
    }
  }
  i = x;
  while (i < MAX_FLIGHTS_PER_CITY) {
    if (dest->flights[i].time == time) {
      break;
    }
    i++;
  }
  if (i == MAX_FLIGHTS_PER_CITY) {
    msg_flight_bad_time();
    return;
  } else {
    if (dest->flights[i].capacity == dest->flights[i].available) {
      msg_flight_all_seats_empty();
      return;
    }
    dest->flights[i].available++;
    return;
  }
}