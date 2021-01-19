# Flight Schedule Manager

Task: Create a program that takes in various commands concerning flights from the user and processes them accordingly. The user should be able to add/remove destination cities, list all possible cities, add/remove flights, list flight times and their corresponding flight capacities, and schedule/unschedule seats on said flights.

Tech Stack Summary: The program is written entirely in C. The two main structures used for this program are flight and flight_schedule. Flight holds all the needed information for a single flight: the departure time, the capacity, and the number of seats still available. Flight schedule contains the destination city, an array of flights for that city, and a prev and next pointer. The flight schedules are organized into a doubly linked list consisting of a free list and an active list. The free list contains all the empty schedules that can be created, while the active list contains all the created flight schedules.
