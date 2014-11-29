*************************************************************************************************
*************************************************************************************************
*******************************Richard Mpanga****************************************************
**********************************README*********************************************************
*************************************************************************************************


I made 3 locks in this program...


 An Active Mapper Lock
   - Locks my int active mapper variable

 addLock
   -Locks the inverted Index when a word is being added to it

 bufferLocks
  - Locks the buffers of each reducer thread when it is added or removed

I used 2 condition variables
  
  Full Condition  Variables 

   - When a buffer is full 

  Empty Condition Variables 
    When a buffer is empty


The amount of bufferLocks , Full Condition variables and empty condition variables depends on the amount of reducers.

A mapper locks a a reducer thread using the bufferlock when it is adding a word to it. It then signals the reducer thread which may be sleeping to let it know the it has a word to process. If the reducer thread is full the mapper thread will wait for a signal from the reducer thread(full condition variable).


When inserting to the invertedIdex a lock is requreid to ensure that multiple threads are not adding to the same index. 

