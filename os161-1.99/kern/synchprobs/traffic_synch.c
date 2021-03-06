#include <types.h>
#include <lib.h>
#include <synchprobs.h>
#include <synch.h>
#include <opt-A1.h>
#include <array.h>

/*
 * This simple default synchronization mechanism allows only vehicle at a time
 * into the intersection.   The intersectionSem is used as a a lock.
 * We use a semaphore rather than a lock so that this code will work even
 * before locks are implemented.
 */

/*
 * Replace this default synchronization mechanism with your own (better) mechanism
 * needed for your solution.   Your mechanism may use any of the available synchronzation
 * primitives, e.g., semaphores, locks, condition variables.   You are also free to
 * declare other global variables if your solution requires them.
 */

/*
 * replace this with declarations of any synchronization and other variables you need here
 */
// static struct semaphore *intersectionSem;
static struct lock *intersectionLock;
static struct cv *intersectionCV;

typedef struct car{
  Direction origin;
  Direction destination;
} car;

bool right_turn(car *v);
bool can_enter_one(car *a, car *b);
bool can_enter_all(car *car);

struct array *cars;

bool
right_turn(car *v) {
  KASSERT(v != NULL);
  if (((v->origin == west) && (v->destination == south)) ||
      ((v->origin == south) && (v->destination == east)) ||
      ((v->origin == east) && (v->destination == north)) ||
      ((v->origin == north) && (v->destination == west))) {
    return true;
  } else {
    return false;
  }
}


/*
 * Checks if incoming car can enter the intersection,
 * given there is already a car in the intersection
 *
 */
bool
can_enter_one(car *a, car *b){
  if (a->origin == b->origin){
    return true;
  }
  else if (a->origin == b->destination && a->destination == b->origin){
    return true;
  }
  else if (a->destination != b->destination && (right_turn(a) || right_turn(b))){
    return true;
  }
  else {
    return false;
  }
}


/*
 * Checks if incoming car can enter the intersection,
 * given there is one or more cars in the intersection
 *
 */
bool
can_enter_all(car *a){
  for (unsigned int i=0; i<array_num(cars); i++){
    if (!can_enter_one(a,  array_get(cars, i))){
      cv_wait(intersectionCV, intersectionLock);
      return false;
    }
  }

  KASSERT(lock_do_i_hold(intersectionLock));
  array_add(cars, a, NULL);
  return true;
}

/*
 * The simulation driver will call this function once before starting
 * the simulation
 *
 * You can use it to initialize synchronization and other variables.
 *
 */
void
intersection_sync_init(void)
{
  // /* replace this default implementation with your own implementation */
  //
  // intersectionSem = sem_create("intersectionSem",1);
  // if (intersectionSem == NULL) {
  //   panic("could not create intersection semaphore");
  // }
  // return;
  intersectionLock = lock_create("intersectionLock");
  intersectionCV = cv_create("intersectionCV");
  cars = array_create();
  array_init(cars);

  if (intersectionLock == NULL){
    panic("Could not create intersectionLock");
  }
  if (intersectionCV == NULL){
    panic("Could not create intersectionCV");
  }
  if (cars == NULL){
    panic("Could not create array cars");
  }

  return;
}

/*
 * The simulation driver will call this function once after
 * the simulation has finished
 *
 * You can use it to clean up any synchronization and other variables.
 *
 */
void
intersection_sync_cleanup(void)
{
  /* replace this default implementation with your own implementation */
  KASSERT(intersectionLock != NULL);
  KASSERT(intersectionCV != NULL);
  KASSERT(cars != NULL);

  lock_destroy(intersectionLock);
  cv_destroy(intersectionCV);
  array_destroy(cars);
}


/*
 * The simulation driver will call this function each time a vehicle
 * tries to enter the intersection, before it enters.
 * This function should cause the calling simulation thread
 * to block until it is OK for the vehicle to enter the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle is arriving
 *    * destination: the Direction in which the vehicle is trying to go
 *
 * return value: none
 */

void
intersection_before_entry(Direction origin, Direction destination)
{
  // /* replace this default implementation with your own implementation */
  // (void)origin;  /* avoid compiler complaint about unused parameter */
  // (void)destination; /* avoid compiler complaint about unused parameter */
  // KASSERT(intersectionSem != NULL);
  // P(intersectionSem);
  KASSERT(intersectionLock != NULL);
  KASSERT(intersectionCV != NULL);
  KASSERT(cars != NULL);

  lock_acquire(intersectionLock);

  car *car = kmalloc(sizeof(struct car));
  car->origin = origin;
  car->destination = destination;

  while (!can_enter_all(car)){}

  lock_release(intersectionLock);
}


/*
 * The simulation driver will call this function each time a vehicle
 * leaves the intersection.
 *
 * parameters:
 *    * origin: the Direction from which the vehicle arrived
 *    * destination: the Direction in which the vehicle is going
 *
 * return value: none
 */

void
intersection_after_exit(Direction origin, Direction destination)
{
  // /* replace this default implementation with your own implementation */
  // (void)origin;  /* avoid compiler complaint about unused parameter */
  // (void)destination; /* avoid compiler complaint about unused parameter */
  // KASSERT(intersectionSem != NULL);
  // V(intersectionSem);
  KASSERT(intersectionLock != NULL);
  KASSERT(intersectionCV != NULL);
  KASSERT(cars != NULL);

  lock_acquire(intersectionLock);

  for (unsigned int i=0; i<array_num(cars); i++){
    car *car = array_get(cars, i);
    if (car->origin == origin && car->destination == destination){
      array_remove(cars, i);
      cv_broadcast(intersectionCV, intersectionLock);
      break;
    }
  }


  lock_release(intersectionLock);
}
