#include <cassert>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

using namespace std;

#define N 5

mutex mu;

enum PhilosopherState {
  Thinking,
  Hungry,
  Eating,
};

class Philosopher {
public:
  int id;
  int hunger;
  PhilosopherState state;
  vector<Philosopher*>& table;
  condition_variable cv;

  Philosopher(int id, vector<Philosopher*>& table) :
    id{id},
    hunger{0},
    state{PhilosopherState::Thinking},
    table{table},
    cv{condition_variable{}}
  {}

  void take() {
    unique_lock<mutex> lock(mu);

    if (hunger >= 20) {
      state = PhilosopherState::Hungry;
    } else {
      return;
    }

    checkstate();

    printf("Philosopher #%d waits for their forks to be available\n", id);
    cv.wait(lock, [this]{return this->state == PhilosopherState::Eating;});

    eat();
  }

  void replace() {
    unique_lock<mutex> lock(mu);
    printf("Philosopher #%d puts down their forks\n", id);

    state = PhilosopherState::Hungry;
    if (hunger <= 15) {
      state = PhilosopherState::Thinking;
    }

    think();

    left()->checkstate();
    right()->checkstate();
  }

  void think() {
    assert(state != PhilosopherState::Eating);
    printf("Philosopher #%d thinks a very intriguing thought\n", id);
    this->hunger++;
  }

  void eat() {
    assert(state == PhilosopherState::Eating);
    printf("Philosopher #%d picks up their forks and eats their food\n", id);
    this->hunger--;
  }

  void checkstate() {
    if (
      this->state == PhilosopherState::Hungry
        && left()->state != PhilosopherState::Eating
        && right()->state != PhilosopherState::Eating
    ) {
      this->state = PhilosopherState::Eating;
      this->cv.notify_all();
    }
  }

  Philosopher* left() {
    return table[(id + 1) % table.size()];
  }

  Philosopher* right() {
    return table[(id + table.size() - 1) % table.size()];
  }

  thread start() {
    return thread(&Philosopher::run, this);
  }

  void run() {
    while (true) {
      take();
      this_thread::sleep_for(chrono::seconds(1));
      // uncomment these to slow the program/logs down
      // this_thread::sleep_for(chrono::milliseconds(300));
      replace();
      this_thread::sleep_for(chrono::seconds(1));
      // this_thread::sleep_for(chrono::milliseconds(300));
      printf("Philosopher #%d hunger=%d\n", id, hunger);

      assert(hunger >= 0 && hunger < 100);
      this_thread::sleep_for(chrono::seconds(1));
    }
  }
};

int main() {
  vector<Philosopher*> table;
  for (int i = 0; i < N; i++) {
    Philosopher* p = new Philosopher(i, table);
    table.push_back(p);
  }

  vector<thread> threads;
  for (Philosopher* p : table) {
    threads.push_back(p->start());
  }

  for (thread& t : threads) {
    t.join();
  }

  return 0;
}