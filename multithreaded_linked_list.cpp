#include <bits/stdc++.h>
#include <pthread.h>
#include <time.h>

using namespace std;

long p = 1;
long no_of_operations = 1e3;
const int MAX_NODE_VALUE = 1e5;
double member_fraction = 0.8;
double insert_fraction = 0.1;
const int INITIAL_NUMBER_OF_KEYS = 1e3;

pthread_mutex_t list_mutex      = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t head_p_mutex    = PTHREAD_MUTEX_INITIALIZER;

pthread_rwlock_t read_write_lock = PTHREAD_RWLOCK_INITIALIZER;

typedef struct list_node {
    int data;
    struct list_node *next;
} list_node;

typedef struct list_node_with_mutex {
    int data;
    struct list_node_with_mutex *next;
    pthread_mutex_t mutex;
} list_node_with_mutex;

list_node *head = NULL;

double slowest_thread_time = 0;
const double DOUBLE_MAX = 1e10;

double entire_list_mutex_average = 0;
double entire_list_mutex_max     = 0;
double entire_list_mutex_min     = DOUBLE_MAX;

double read_write_average        = 0;
double read_write_max            = 0;
double read_write_min            = DOUBLE_MAX;

pthread_mutex_t barrier_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t sum_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond_var      = PTHREAD_COND_INITIALIZER;
long barrier_counter          = 0;

//utility function
const char* isMember (int c) {
    return c ? "Yes" : "No";
}

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

// to check if <value> is a member of the linked list
int Member (int value, list_node* head_p) {

    list_node* curr_p = head_p;

    while (curr_p != NULL && curr_p-> data < value) 
        curr_p = curr_p-> next;
    
    if (curr_p == NULL || curr_p->data > value) 
        return 0;

    return 1;

}

// insert value into linked list
int Insert (int value, list_node** head_p) {

    list_node* curr_p = *head_p;
    list_node* prev_p = NULL;
    list_node* temp_p;

    while (curr_p != NULL && curr_p->data < value) {
        prev_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p == NULL || curr_p->data > value) {

        temp_p = (list_node *) malloc  (sizeof(list_node));
        temp_p->data = value;
        temp_p->next = curr_p;
        if (prev_p == NULL) 
            *head_p = temp_p;
        else 
            prev_p->next = temp_p;
        return 1;

    }
    return 0;

}

int Delete (int value, list_node** head_p) {

    list_node* curr_p = *head_p;
    list_node* prev_p = NULL;

    while (curr_p != NULL && curr_p->data < value) {
        prev_p = curr_p;
        curr_p = curr_p->next;
    }

    if (curr_p != NULL && curr_p->data == value) {
        if (prev_p == NULL) {
            *head_p = curr_p->next;
            free (curr_p);
        } else {
            prev_p->next = curr_p->next;
            free (curr_p);
        }
        return 1;
    }
    return 0;
    
}

void* compute_operations_entire_list_mutex (void *args) {

    Barrier();
    
    auto start_time = chrono :: system_clock :: now();
    long my_count = no_of_operations/p;

    while (my_count--) {

        double choice = (double) rand() / RAND_MAX;
        int value = rand() % MAX_NODE_VALUE;

        if (choice <= member_fraction) {

            pthread_mutex_lock (&list_mutex);
            int c = Member (value, head);
            pthread_mutex_unlock (&list_mutex);
            // uncomment block below to get visual output
            /*
            printf (" Is \033[31m%d\033[0m member of linked list : \033[1;34m%s\033[0m\n", value, isMember (c));
            */
        } else if (choice <= (member_fraction + insert_fraction)) {

            pthread_mutex_lock (&list_mutex);
            int c = Insert (value, &head);
            pthread_mutex_unlock (&list_mutex);
            // uncomment block below to get visual output
            /* 
            if(c) 
                printf(" \033[31m%d\033[0m is \033[1;34minserted\033[0m successfully...\n", value);
            else
                printf(" \033[31m%d\033[0m already exists in the linked-list...\n", value);
            */
        } else {

            pthread_mutex_lock (&list_mutex);
            int c = Delete (value, &head);
            pthread_mutex_unlock (&list_mutex);
            // uncomment block below to get visual output
            /*
            if(c)
                printf(" \033[31m%d\033[0m is \033[1;34mdeleted\033[0m successfully...\n", value);
            else 
                printf(" \033[31m%d\033[0m is not in the linked-list...\n", value);
            */
        }
        
    }

    auto end_time   = chrono :: system_clock :: now();

    chrono::duration<double> time_taken = end_time - start_time;
    slowest_thread_time = max (slowest_thread_time, (double) time_taken.count());

    return NULL;

}

void* compute_operations_read_write_locks (void *args) {

    Barrier();
    
    auto start_time = chrono :: system_clock :: now();
    long my_count = no_of_operations/p;

    while (my_count--) {

        double choice = (double) rand() / RAND_MAX;
        int value = rand() % MAX_NODE_VALUE;

        if (choice <= member_fraction) {

            pthread_rwlock_rdlock (&read_write_lock);
            int c = Member (value, head);
            pthread_rwlock_unlock (&read_write_lock);
            // uncomment block below to get visual output
            /*
            printf (" Is \033[31m%d\033[0m member of linked list : \033[1;34m%s\033[0m\n", value, isMember (c));
            */
        } else if (choice <= (member_fraction + insert_fraction)) {

            pthread_rwlock_wrlock (&read_write_lock);
            int c = Insert (value, &head);
            pthread_rwlock_unlock (&read_write_lock);
            // uncomment block below to get visual output
            /*
            if(c) 
                printf(" \033[31m%d\033[0m is \033[1;34minserted\033[0m successfully...\n", value);
            else
                printf(" \033[31m%d\033[0m already exists in the linked-list...\n", value);
            */
        } else {

            pthread_rwlock_wrlock (&read_write_lock);
            int c = Delete (value, &head);
            pthread_rwlock_unlock (&read_write_lock);
            // uncomment block below to get visual output
            /*
            if(c)
                printf(" \033[31m%d\033[0m is \033[1;34mdeleted\033[0m successfully...\n", value);
            else 
                printf(" \033[31m%d\033[0m is not in the linked-list...\n", value);
            */
        }
        
    }

    auto end_time   = chrono :: system_clock :: now();

    chrono::duration<double> time_taken = end_time - start_time;
    slowest_thread_time = max (slowest_thread_time, (double) time_taken.count());

    return NULL;

}

// initialize the linked-list with <no_of_elements> elements
void initial_insert (int no_of_elements) {

    int count_insertion = 0;
    if (MAX_NODE_VALUE < no_of_elements) {
        cout << "Err : MAX_NODE_VALUE should be at least INITIAL_NUMBER_OF_KEYS\n";
    }
    // insert until distinct <no_of_elements> elements are inserted
    while (count_insertion < no_of_elements) {
        int value = rand() % MAX_NODE_VALUE;
        count_insertion += Insert (value, &head);
    }

}

int main (int argc, char* argv[]) {

    srand (time(NULL));
    initial_insert (INITIAL_NUMBER_OF_KEYS);
    long no_of_iterations = 5;

    if (argc > 1) {
        p = strtol (argv[1], NULL, 10);
        if (p <= 0) {
            cout << "Invalid Argument 1 : Number of threads can't be negative or zero\n";
            return -1;
        }
    }

    if (argc > 2) {
        member_fraction = strtof (argv[2], NULL);
        if (member_fraction > 1 || member_fraction < 0) {
            cout << "Invalid Argument 2 : Fraction of member operation should be between 0 and 1\n";
            return -2;
        }
        insert_fraction = (1-member_fraction)/2;
    }

    if (argc > 3) {
        insert_fraction = strtof (argv[3], NULL);
        if (insert_fraction > 1 || insert_fraction < 0 || (member_fraction+insert_fraction > 1)) {
            cout << "Invalid Argument 3 : Fraction of insertion operation is not valid\n";
        }
    }

    // using entire-list mutex
    for (int i = 0; i < no_of_iterations; i++) {

        pthread_t threads[p];
        for (long thread = 0; thread < p; thread++) {
            pthread_create (&threads[thread], NULL, compute_operations_entire_list_mutex, NULL);
        }

        for (long thread = 0; thread < p; thread++) {
            pthread_join (threads[thread], NULL);
        }

        entire_list_mutex_average += slowest_thread_time;
        entire_list_mutex_max      = max (entire_list_mutex_max, slowest_thread_time);
        entire_list_mutex_min      = min (entire_list_mutex_min, slowest_thread_time);

    }    

    // using read-write locks
    for (int i = 0; i < no_of_iterations; i++) {

        pthread_t threads[p];
        slowest_thread_time = 0;

        for (long thread = 0; thread < p; thread++) {
            pthread_create (&threads[thread], NULL, compute_operations_read_write_locks, NULL);
        }

        for (long thread = 0; thread < p; thread++) {
            pthread_join (threads[thread], NULL);
        }

        read_write_average += slowest_thread_time;
        read_write_max      = max (read_write_max, slowest_thread_time);
        read_write_min      = min (read_write_min, slowest_thread_time);

    }

    cout << "\n \t\033[1;31mFINAL OUTPUT :\033[0m " << endl;

    cout << "\n Number of threads used : " << p << endl;
    cout << "\n Number of iterations for each  : " << no_of_iterations << endl;

    cout << "\n \033[31mFor entire-list mutex :\033[0m " << endl;
    cout << "\n \033[34mMax. time taken :\033[0m " << entire_list_mutex_max << endl;
    cout << " \033[34mMin. time taken :\033[0m " << entire_list_mutex_min << endl;
    cout << " \033[34mAvg. time taken :\033[0m " << entire_list_mutex_average   << endl;

    cout << "\n \033[31mFor read-write locks :\033[0m " << endl;
    cout << "\n \033[34mMax. time taken :\033[0m " << read_write_max << endl;
    cout << " \033[34mMin. time taken :\033[0m " << read_write_min << endl;
    cout << " \033[34mAvg. time taken :\033[0m " << read_write_average   << endl;

    return 0;

}