#include <bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>

using namespace std;

// number of threads
long p = 1;

// maximum double value for the program
const double DOUBLE_MAX = 1e10;
enum choices { BUSY_WAIT_MUTEX_CHOICE, SEMAPHORE_CHOICE, CONDITION_VARIABLE_CHOICE };

// initialize average, maximum and minimum for three cases
double busy_wait_mutex_average_time = 0;
double busy_wait_mutex_max_time = 0;
double busy_wait_mutex_min_time = DOUBLE_MAX;

double semaphore_average_time = 0;
double semaphore_max_time = 0;
double semaphore_min_time = DOUBLE_MAX;

double condition_variable_average_time = 0;
double condition_variable_max_time = 0;
double condition_variable_min_time = DOUBLE_MAX;

double slowest_thread_time = 0;

// barrier mutex
pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER;

// counting semaphore and barrier semaphore
sem_t count_sem;
sem_t barrier_sem;

// condition variable
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

long barrier_counter = 0;       // initialize to 0

void barrier_using_busy_wait_mutex () {

    // Barrier
    pthread_mutex_lock (&barrier_mutex);
    barrier_counter++;
    pthread_mutex_unlock (&barrier_mutex);

    // while counter is less than number of threads
    while (barrier_counter < p);

}

void barrier_using_semaphore () {

    // Barrier 
    sem_wait (&count_sem);
    // if the last thread is executing this
    if (barrier_counter == p-1) {

        barrier_counter = 0;            // reset barrier_counter to 0
        sem_post (&count_sem);
        // for all other (p-1)-threads
        for (int thread = 0; thread < p-1; thread++) {
            sem_post (&barrier_sem);
        }

    } else {

        barrier_counter++;
        sem_post (&count_sem);
        // wait till the last thread signals it to go
        sem_wait (&barrier_sem);

    }

}

// consider this barrier as default barrier for computing time for other barriers
void barrier_using_condition_variable () {

    pthread_mutex_lock (&barrier_mutex);
    barrier_counter++;

    if(barrier_counter == p) {
        barrier_counter = 0;
        pthread_cond_broadcast (&cond_var);
    } else {
        while ((pthread_cond_wait (&cond_var, &barrier_mutex)) != 0);
    }
    pthread_mutex_unlock (&barrier_mutex);

}

/* call barrier depending on the choice 
 * my_choice = 0        => call barrier using busy-wait mutex
 * my_choice = 1        => call barrier using semaphore
 * my_choice = 2        => call barrier using condition variable
**/
void *call_barrier (void *choice) {

    long my_choice = (long) choice;

    // choice is busy-wait mutex
    if(my_choice == BUSY_WAIT_MUTEX_CHOICE) {

        barrier_using_condition_variable ();

        auto start_time = chrono :: system_clock :: now();
        barrier_using_busy_wait_mutex ();
        auto end_time   = chrono :: system_clock :: now();

        chrono::duration<double> time_taken = end_time - start_time;

        slowest_thread_time = max (slowest_thread_time, (double) time_taken.count());

    } else if (my_choice == SEMAPHORE_CHOICE) {

        barrier_using_condition_variable ();

        auto start_time = chrono :: system_clock :: now();
        barrier_using_semaphore ();
        auto end_time   = chrono :: system_clock :: now();

        chrono::duration<double> time_taken = end_time - start_time;
        
        slowest_thread_time = max (slowest_thread_time, (double) time_taken.count());

    } else if (my_choice == CONDITION_VARIABLE_CHOICE) {

        barrier_using_condition_variable ();

        auto start_time = chrono :: system_clock :: now();
        barrier_using_condition_variable ();
        auto end_time   = chrono :: system_clock :: now();

        chrono::duration<double> time_taken = end_time - start_time;
        slowest_thread_time = max (slowest_thread_time, (double) time_taken.count());

    } else {

        cout << " Err : Wrong choice for calling barrier \n";

    }
    
    return NULL;

}

int main (int argc, char* argv[]) {

    long no_of_iterations = 5;
    long counter;

    if (argc > 1) {
        p = strtol (argv[1], NULL, 10);
        if (p <= 0) {
            cout << "Invalid Argument 1 : Number of threads can't be negative or zero\n";
            return -1;
        }
    }
    if (argc > 2) {
        no_of_iterations = strtol (argv[2], NULL, 10);
        if (no_of_iterations <= 0) {
            cout << "Invalid Argument 2 : Number of iterations can't be negative or zero\n";
            return -2;
        }
    }
    
    // barrier using busy-wait and mutex
    for (counter = 0; counter < no_of_iterations; counter++) {

        pthread_t threads[p];
        slowest_thread_time = 0;

        for (long thread = 0; thread < p; thread++) {
            pthread_create (&threads[thread], NULL, call_barrier, (void *) BUSY_WAIT_MUTEX_CHOICE);
        }
        for (long thread = 0; thread < p; thread++) {
            pthread_join (threads[thread], NULL);
        }

        busy_wait_mutex_average_time += slowest_thread_time;
        busy_wait_mutex_max_time      = max (busy_wait_mutex_max_time, slowest_thread_time);
        busy_wait_mutex_min_time      = min (busy_wait_mutex_min_time, slowest_thread_time);

        // reset barrier_counter to 0
        barrier_counter = 0;
        /* NOTE : In busy-wait mutex implementation of barrier, we can reset barrier
                  inside the thread function because it may cause some threads to
                  never come out of the while loop
        */
    }

    // barrier using semaphore
    for (counter = 0; counter < no_of_iterations; counter++) {

        sem_init (&count_sem, 0, 1);        // initialize to 1
        sem_init (&barrier_sem, 0, 0);      // initialize to 0
        
        pthread_t threads[p];
        slowest_thread_time = 0;

        for (long thread = 0; thread < p; thread++) {
            pthread_create (&threads[thread], NULL, call_barrier, (void *) SEMAPHORE_CHOICE);
        }
        for (long thread = 0; thread < p; thread++) {
            pthread_join (threads[thread], NULL);
        }

        semaphore_average_time += slowest_thread_time;
        semaphore_max_time      = max (semaphore_max_time, slowest_thread_time);
        semaphore_min_time      = min (semaphore_min_time, slowest_thread_time);

    }

    // barrier using condition variable
    for (counter = 0; counter < no_of_iterations; counter++) {

        pthread_t threads[p];
        slowest_thread_time = 0;

        for (long thread = 0; thread < p; thread++) {
            pthread_create (&threads[thread], NULL, call_barrier, (void *) CONDITION_VARIABLE_CHOICE);
        }
        for (long thread = 0; thread < p; thread++) {
            pthread_join (threads[thread], NULL);
        }

        condition_variable_average_time += slowest_thread_time;
        condition_variable_max_time      = max (condition_variable_max_time, slowest_thread_time);
        condition_variable_min_time      = min (condition_variable_min_time, slowest_thread_time);

    }

    busy_wait_mutex_average_time    /= no_of_iterations;
    semaphore_average_time          /= no_of_iterations;
    condition_variable_average_time /= no_of_iterations;

    cout << "\n \t\033[1;31mFINAL OUTPUT :\033[0m " << endl;

    cout << "\n Number of threads used : " << p << endl;

    cout << "\n \033[31mFor busy-wait :\033[0m " << endl;
    cout << "\n \033[34mMax. time taken :\033[0m " << busy_wait_mutex_max_time << endl;
    cout << " \033[34mMin. time taken :\033[0m "   << busy_wait_mutex_min_time << endl;
    cout << " \033[34mAvg. time taken :\033[0m "   << busy_wait_mutex_average_time << endl;

    cout << "\n \033[31mFor semaphore :\033[0m " << endl;
    cout << "\n \033[34mMax. time taken :\033[0m " << semaphore_max_time << endl;
    cout << " \033[34mMin. time taken :\033[0m "   << semaphore_min_time << endl;
    cout << " \033[34mAvg. time taken :\033[0m "   << semaphore_average_time << endl;

    cout << "\n \033[31mFor condition variable :\033[0m " << endl;
    cout << "\n \033[34mMax. time taken :\033[0m " << condition_variable_max_time << endl;
    cout << " \033[34mMin. time taken :\033[0m "   << condition_variable_min_time << endl;
    cout << " \033[34mAvg. time taken :\033[0m "   << condition_variable_average_time << endl;

    return 0;

}