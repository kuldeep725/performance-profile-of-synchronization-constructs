#include <bits/stdc++.h>
#include <pthread.h>

using namespace std;

// <n> is the number of elements of the array
long n = 1e8;     
// <p> is the number of threads
long p = 1;
// <RANGE> of elements of each thread
long RANGE = n/p;
vector<long> arr;

long flag = 0;
long sum  = 0;
double slowest_thread_time = 0;
const double DOUBLE_MAX = 1e10;

pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_var      = PTHREAD_COND_INITIALIZER;
long barrier_counter          = 0;

void Barrier () {

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

void *partial_sum (void *rank) {

    long my_rank = (long) rank;
    long start = my_rank * RANGE;
    long end   = (my_rank + 1)*RANGE - 1;

    long my_sum = 0;

    Barrier();
    auto start_time = chrono :: system_clock :: now();
    for (int i= start; i <= end; i++) {
        my_sum += arr[i];
    }
    while (flag != my_rank);
    sum += my_sum;
    flag = (flag + 1) % p;
    auto end_time = chrono :: system_clock :: now();

    chrono::duration<double> time_taken = end_time - start_time;
    slowest_thread_time = max (slowest_thread_time, (double) time_taken.count());

    return NULL;

}

int main (int argc, char* argv[]) {

    
    long no_of_iterations = 5;
    double max_v = 0, min_v = DOUBLE_MAX, avg = 0;

    if(argc > 1) {
        p = strtol (argv[1], NULL, 10);
        if(p <= 0) {
            cout << "Invalid Argument 1 : <number of threads> can't be negative or zero\n";
            return -3;
        }
    }

    if(argc > 2) {
        no_of_iterations = strtol (argv[2], NULL, 10);
        if(no_of_iterations <= 0) {
            cout << "Invalid Argument 2 : <no_of_iterations> can't be negative or zero\n";
            return -1;
        }
    }

    if(argc > 3) {
        n = (long)strtof (argv[3], NULL);
        if(n < 0) {
            cout << "Invalid Argument 3 : <size of the array> can't be negative\n";
            return -2;
        }
    }

    // assume n is divisible by p
    RANGE = n/p;
    if(n % p != 0) {
        cout << "Err : Size of the array is not divisible by number of threads\n";
        return -1;
    }

    // putting elements into the array
    for (long i = 1; i <= n; i++) {
        arr.push_back (i);
    }

    cout << " Size of the array    : " << n << endl;
    cout << " Number of iterations : " << no_of_iterations << endl;
    for (long iter = 0; iter < no_of_iterations; iter++) {

        pthread_t threads[p];
        sum = 0;
        slowest_thread_time = 0;
        // pointers are 64-bit in a 64-bit system... so we can use long
        // instead of actual pointer
        for (long i = 0; i < p; i++) {
            pthread_create (&threads[i], NULL, partial_sum, (void *) i);
        }

        for (int i = 0; i < p; i++) {
            pthread_join (threads[i], NULL);
        }

        max_v = max (max_v, slowest_thread_time);
        min_v = min (min_v, slowest_thread_time);
        avg   = avg + slowest_thread_time;

        cout << "\n---------------------------------------------------------------\n";
        cout << "\n Results for iteration " << iter+1 << " : \n\n";
        cout << "   Number of elements                      : " << n << endl;
        cout << "   Number of threads                       : " << p << endl;
        cout << "   The sum of the array using busy-loop is : " << sum << endl;
        cout << "   Time taken to compute is                : " << slowest_thread_time << "\n";
        cout << "\n----------------------------------------------------------------\n";

    }
    
    avg = avg / no_of_iterations;
    cout << "\n \033[1;31mFINAL OUTPUT :\033[0m " << endl;
    cout << "\n \033[34mMax. time taken :\033[0m " << max_v << endl;
    cout << " \033[34mMin. time taken :\033[0m " << min_v << endl;
    cout << " \033[34mAvg. time taken :\033[0m " << avg   << endl;
    return 0;

}