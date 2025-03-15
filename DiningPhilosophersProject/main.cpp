#include <iostream>
#include <thread>
#include <semaphore>
#include <mutex>
#include <vector>
#include <chrono>

using namespace std;
// Partners: Simon Liang and Ben Richeson
class Philosopher{
    private:
        int death;
        int needfood;
        int rate;
        int hunger;
        bool thinking;
        bool eating;
        bool isDead;
        int id;

        const int PHILOSOPHERS = 5;
        mutex *leftFork;
        mutex *rightFork;

    public:

    Philosopher(int i,mutex *left, mutex*right): leftFork(left), rightFork(right)
    {
        hunger = 10;
        thinking = false;
        eating = false;
        isDead = false;
        death = 100;
        needfood = 40;
        rate = 5;

        id = i;
        
    }

    int getHunger(){
        return hunger;
    }

    int getID(){
        return id;
    }

    void increaseHunger(){
        hunger += rate;
    }

    void decreaseHunger(){
        hunger -= rate;
    }

    bool isThinking(){
        return thinking;
    }
    
    bool isEating(){
        return eating;
    }

    bool isHungry(){
        if (hunger >= needfood){
            return true;
        }
        else{
            return false;
        }
    }

    void eat(){ // Attempts to Eat, Will Think Instead if Fork(s) are taken already
        if (leftFork->try_lock()){ // Checks if the left Fork is available, locks automatically
            leftFork->unlock(); // Unlocks manually in order to check the right fork as well
            if(rightFork->try_lock()){ // Checks if the Right Fork is available, locks automatically
                rightFork->unlock(); 
                // Locks both forks now that it has verified both forks are available
                leftFork->lock(); 
                rightFork->lock();
                thinking = false;
                eating = true;
                cout << "Philosopher " << id << " picks up forks and starts eating. Hunger: " <<  hunger<<endl;
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
        else{ // What happens if at least one of the forks are occupied
            cout<< "Philosopher " << id << " tries to pick up the forks but at least one of them is currently being used." <<endl;

            if((!(leftFork->try_lock()))){ // Checks if left fork is being used by someone
                if ((id-1)!=0){
                    cout<< "Philosopher " << (id-1) << " is using your left fork so you can't eat." <<endl;
                }
                else{
                    cout<< "Philosopher "<< PHILOSOPHERS << " is using your left fork so you can't eat." << endl;
                }
            }
            else{ // Manually unlocks since using try_lock() locks the mutex if it was available
                leftFork->unlock();
            }

            if((!(rightFork->try_lock()))){ // Checks if right fork is being used by someone
                if ((id)!=PHILOSOPHERS){
                    cout<< "Philosopher " << (id+1) << " is using your right fork so you can't eat." <<endl;
                }
                else{
                    cout<< "Philosopher 1 is using your right fork so you can't eat." << endl;
                }
            }
            else{ // Manually unlocks since using try_lock() locks the mutex if it was available
                rightFork->unlock();
            }

            if (!isThinking()){ // Checks if they're not already thinking
                think(); // Forces them to think because the fork(s) are being occupied.
            }
            if(isThinking()){
                increaseHunger();  
            }

            this_thread::sleep_for(chrono::seconds(1));
        }
    }
    void think(){
        if (isEating()){ 
            eating = false;
            cout << "Philosopher " << id << " puts down their forks.\n";
            leftFork->unlock();
            rightFork->unlock();
        }
        cout << "Philosopher " << id << " is thinking. Hunger: " << hunger << endl;
        thinking = true;
        this_thread::sleep_for(chrono::seconds(1));

    }

    void deathcheck(){
        if (hunger >= 100){
            isDead = true;
        }
    }

    thread start() {
        return thread(&Philosopher::run, this);
    }

    void run(){
        while (!isDead){
            if (isHungry()){ // Checks if their hunger is greater than or equals needfood, which is 40
                if (!isEating()){ // Checks if they're not already eating, so it applies once
                    eat(); // Attempts to Eat, Will Think Instead if Forks are Occupied in the Eat Function
                }
                if (isEating()){ 
                    decreaseHunger();
                }    
                cout<< "Philosopher "<< id << " Hunger: " << hunger << endl;
                this_thread::sleep_for(chrono::seconds(1));
            }
            else{ //If they are not hungry
                if (!isThinking()){ // Checks if they're not already thinking, so it applies once
                    think();
                }
                if(isThinking()){
                    increaseHunger();  
                }
                cout<< "Philosopher "<< id << " Hunger:" << hunger << endl;
                this_thread::sleep_for(chrono::seconds(1));
            };
        }
    }
};





int main(){
    
    vector<Philosopher> p;
    
    const int NUM_PHILOSOPHERS=5;
    mutex forks[NUM_PHILOSOPHERS];;
    
    for(int i=0; i<NUM_PHILOSOPHERS;i++){
        forks[i].unlock();
    }

    vector <thread> threads;
    
    p.emplace_back(Philosopher(1,&forks[NUM_PHILOSOPHERS-1],&forks[0])); // Creates the First Philosopher
    // Left Fork is the Last Philosopher's Right Fork, Right Fork is 2nd Philosopher's Left Fork

    for (int i = 1; i < NUM_PHILOSOPHERS-1; i++){ // Creates the Second to Second to Last Philosopher
        p.emplace_back(Philosopher(i+1,&forks[i-1],&forks[i]));
    }
    // Left Fork is the Previous Philosopher's Right Fork, Right Fork is the next Philosopher's Left Fork
    

    // Creates the Last Philosopher
    p.emplace_back(Philosopher(NUM_PHILOSOPHERS,&forks[NUM_PHILOSOPHERS-2],&forks[NUM_PHILOSOPHERS-1]));
    //Left Fork is the Second to Last Philosopher's Right Fork, Right Fork is the First Philosopher's Left Fork
    
    for(int i=0; i<NUM_PHILOSOPHERS; i ++){
        cout<< "Philosopher " << p.at(i).getID() << " is generated." << endl;
        threads.emplace_back(p.at(i).start());
    }


    for(int i=0; i<NUM_PHILOSOPHERS; i ++){
        threads.at(i).join();
    }

    return 0;

}

/*
For better understanding of the Philosophers in their vector with the left and right forks
And the placements of the fork mutex in the array in the code above

Philosopher Position in Vector (ID is Position + 1)
                 0       1       2       3      4
Philosophers-    O       O       O       O      O
Forks-               I       I       I       I      I
                     0       1       2       3      4
Fork Position in Array
*/