#include <iostream>

#include <queue>
#include <chrono>
#include <mutex>
#include <thread>
#include <iostream>
#include <condition_variable>
#include <time.h>

#define NUM_OF_THREADS 100


int main() {
   std::queue<int> produced_nums;
   std::mutex mtx;
   std::condition_variable cv;
   std::mutex mtx2;
   std::condition_variable cv2;
   int count[NUM_OF_THREADS] = { 0 };
   int done = 0;


   auto service_thread = [&]() {

      for (int i = 0; ; i++) {
         clock_t start = clock();
         std::unique_lock<std::mutex> lock(mtx);
         std::cout << "producing " << i << std::endl;

         if (!produced_nums.empty()) 
            produced_nums.pop();

         produced_nums.push(i);


         for (int j = 0; j < NUM_OF_THREADS; j++)
            count[j] = 1;

         cv.notify_all();
         std::cout << "notify_all" << std::endl;

         lock.unlock();


         std::unique_lock<std::mutex> lock2(mtx2);
         while (done != NUM_OF_THREADS)
         {
            cv2.wait(lock2);
         }
         done = 0;

         clock_t end = clock();

         printf("Time: %lf\n", (double)(end - start) / CLOCKS_PER_SEC);
      }


   };

   auto worker_thread = [&](int idx) {
      while (true) {

         std::unique_lock<std::mutex> lock(mtx);

         while (count[idx] == 0)
         {  
            cv.wait(lock);
         }
         lock.unlock();

         
         int data;

         if (!produced_nums.empty()) 
            data = produced_nums.front();
         
         std::this_thread::sleep_for(std::chrono::milliseconds(2000));

         lock.lock();
         if (!produced_nums.empty()) {
            std::cout << "consuming [" << idx << "] " << data << std::endl;
         }

         count[idx] = 0;
         done++;
         lock.unlock();

         std::unique_lock<std::mutex> lock2(mtx2);
         if (done == NUM_OF_THREADS)
         {
            cv2.notify_one();
            std::cout << "notify_one" << std::endl;
         }

      }
   };

   std::vector<std::thread> cs(NUM_OF_THREADS);
   for (int i = 0; i < NUM_OF_THREADS; ++i) {
      cs[i] = std::thread(worker_thread, i);
   }
   std::thread p(service_thread);

   p.join();
   for (int i = 0; i < NUM_OF_THREADS; ++i) {
      cs[i].join();
   }

   return 0;
}