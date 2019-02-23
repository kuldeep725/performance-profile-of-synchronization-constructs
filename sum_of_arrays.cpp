#include <bits/stdc++.h>
#include <pthread.h>
#include <chrono>

using namespace std;

long n = 1e8;
vector<long> arr;

int main (int argc, char* argv[]) {

    if(argc > 1) {
        n = (long) strtof (argv[1], NULL);
        if(n < 0) {
            cout << "Invalid Argument 1 : <size of the array> can't be negative\n";
            return -1;
        }
    }
    
    for (int i = 1; i <= n; i++) {
        // arr[i-1] = i;
        arr.push_back(i);
    }
    
    long sum = 0;
    
    auto start_time = chrono :: system_clock :: now();
    for(int i = 0; i < n; i++) {
        sum += arr[i];
    }
    auto end_time = chrono :: system_clock :: now();

    chrono::duration<double> time_taken = end_time - start_time;
    cout << "Sum of the array is : " << sum << endl;
    cout << "Time taken to compute is : " << time_taken.count() << "s\n";
    return 0;
    
}