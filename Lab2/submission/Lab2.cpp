#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<stdlib.h>
#include<queue>
#include<pthread.h>
#include<unordered_map>



using namespace std;
 int maxMappers;
 int maxReducers;
 int active_mappers;
 int active_reducers;

struct wordNode; 
class ReducerBuffer;


pthread_mutex_t active_mappers_lock;
pthread_mutex_t addLock;

//Vector of Reducer Buffers
vector <ReducerBuffer> reducerBuffers;

// Vector of Locks and Condition Variables
vector <pthread_mutex_t> bufferLocks;
vector <pthread_cond_t>  fullConditions;
vector <pthread_cond_t>  emptyConditions;
 
//Inverted Index
unordered_map<string, string> invertedIndex;

//For traversing info to mapper->reducer->invertedIndex
struct wordNode 
{
 string word;
 string filename;
 int linenumber;
};

//Reducer Buffer definition
class ReducerBuffer
{
 private:
    std::queue<wordNode> bufferqueue;
    int size = 0;
    const int  max = 10;

 public:

   queue<wordNode> getQueue()
   {
    return bufferqueue;
   } 
  
   //Insert word node in buffer
   void insert(wordNode node)
   {
    if (size< 10 )
     {
     bufferqueue.push(node);
     size++;
     } 

  }
  //Remove word and return node
    wordNode remove ()
    { 
      if (size > 0)
       {
       size--;
       wordNode temp = bufferqueue.front();
       bufferqueue.pop();
       return temp;  
      }
    }
  //Returns true if buffer is = 10
   int isFull()
    {
     if (size == max)
      return 1; 
     else 
       return 0; 
    }

   int isEmpty()
   {
    return (size == 0);
   }
   int getSize()
   {
    return size;
   }



};


int hashfunction(string word)
{
  int val = 0;
  for(unsigned int j =0; j < word.length(); j++)
      val = val + int(word[i]);
 return val;
}


//Mapper Class definiton
class mapper
{
 public:
  int linenumber = 1;
  ifstream readin; 
  string word;
  vector<wordNode> wordlist;
  string filename;

   //Read file and store in word list vector
   void readFile(string fn)
   {
     filename = fn;
     readin.open(filename);
     while(!readin.eof())
     {
       getline(readin, word);
      
       if (word.size() !=0)
       {
       wordNode wn;
       wn.word = word;
       wn.linenumber = linenumber;
       wn.filename = filename;
       wordlist.push_back(wn);
       linenumber++;
       
       }
     }
     readin.close();
   } 


   vector<wordNode> getWordList()
   {
    return wordlist;
   }
   
   int getLineNumber()
   {
    return linenumber;
   }
   
  string getFileName()
  {
   return filename;
  }
  
   string getWord()
   {
    return word;
   }

};

//Mapping thread fuction
void *mapping(void *threadID)
{
  int id = (long )threadID;
  cout <<"    Mapper thread[" << id << "] is starting...\n" << flush;
  string filename = "foo" + to_string(id) +".txt";
  mapper mp;
  vector<wordNode> wordlist; //word list
  mp.readFile(filename); //generating wordlist
  wordlist = mp.getWordList();
//  cout << "     " <<"FileName:  " <<  mp.getFileName() <<"\n";
  
  
  for (unsigned int i =0; i <wordlist.size(); i++)
   {
   int hashVal = hashfunction(wordlist[i].word);    
   int reducer = hashVal % maxReducers;   
   //Grab lock of the apporiate reducer 
   pthread_mutex_lock(&bufferLocks[reducer]); 
    while( reducerBuffers[reducer].isFull())
     { //Buffer is full sleep 
       cout <<"    Mapper thread[ " << id << "] is waiting for  reducer buffer[" << reducer << "]\n" << flush;
      pthread_cond_wait(&(fullConditions[reducer]) , &(bufferLocks[reducer])); 
     }
   //cout <<"    Mapper thread[" << id << "] is added word to reducer buffer[" << reducer << "]\n" << flush;
   reducerBuffers[reducer].insert(wordlist[i]); //Adding to reducer buffer
   
   //Signal waiting reducer thread - this case reducer
   pthread_cond_signal(&(emptyConditions[reducer])); 
   pthread_mutex_unlock(&(bufferLocks[reducer]));
   
  }
   pthread_mutex_lock(&active_mappers_lock); //Lock active mappers operation
   active_mappers--;  
    if (active_mappers == 0)
   {   //Last mapper signals all waiting reducers
    cout <<"  Mapper ThreadID: " << id <<" " << "is last mapper and is signaling reducers\n" << flush;
    for (int j = 0; j <emptyConditions.size(); j++){
     pthread_cond_signal(&(emptyConditions[j]));
     }
    }
   pthread_mutex_unlock(&active_mappers_lock);

  cout <<"  Mapper ThreadID: " << id <<" " << "has terminated\n" << flush;
  pthread_exit(NULL);  
return 0;
}

//Reducer thread
void *reducing(void *threadID)
{
  int tid = (long) threadID;
while ((active_mappers != 0 ) || !(reducerBuffers[tid].isEmpty()))
 {
   pthread_mutex_lock(&(bufferLocks[tid]));
   while(reducerBuffers[tid].isEmpty())
   { //if empty wait for mapper to insert word or last mapper
	if((active_mappers ==0) && (reducerBuffers[tid].isEmpty())){
           break;
	}
    pthread_cond_wait(&(emptyConditions[tid]), &(bufferLocks[tid])); 
   }
    if(!((active_mappers ==0) && (reducerBuffers[tid].isEmpty())))
    {
    wordNode element = reducerBuffers[tid].remove();
    string value = " ( "+element.filename + ": " + to_string(element.linenumber) +" ) ";
    unordered_map<string,string>::const_iterator t = invertedIndex.find(element.word);
     pthread_mutex_lock(&addLock);
      if (t == invertedIndex.end())
      {
       invertedIndex.insert(std::pair<string, string> (element.word, value));
      }
      else{
       invertedIndex.at(element.word) +=", " + value;
	}
      pthread_mutex_unlock(&addLock);
        pthread_cond_signal(&(fullConditions[tid]));     
	
    }

    /** Insert wordNode to hashtable **/
  //  pthread_cond_signal(&(fullConditions[tid]));
    pthread_mutex_unlock(&(bufferLocks[tid]));
 }
   pthread_exit(NULL);
return 0;
}
 


int main(int argc, char* argv[])
{
   
  if (argv[1] == nullptr || argv[2] == nullptr)
  {
    cout << " ERROR: Entered NULL values for mappers and reducers\n" << endl;
    return 0;
  }

  maxMappers =  atoi(argv[1]);
  maxReducers = atoi(argv[2]);
  pthread_t mThreads[maxMappers];
  pthread_t rThreads[maxReducers];

  
 // pthread_cond_init(&fullCondition  , NULL );
 // pthread_cond_init(&emptyCondition , NULL );
//  pthread_mutex_init(&bufferLock , NULL);
  pthread_mutex_init(&active_mappers_lock , NULL);
  pthread_mutex_init(&addLock, NULL);
  

   // cout << " Main : Creating Mapper Threads...\n" << flush;
    active_mappers = maxMappers;
  
    
   // cout << " Main : Creating Reducer Threads...\n" << flush;
   
 active_reducers = maxReducers;
 
  //Creating and intialiazing locks and condition variables
  for (int n = 0; n <maxReducers; n++)
   {
   
       pthread_mutex_t bufferLock;
       pthread_cond_t emptyCondition;
       pthread_cond_t fullCondition;
 
       bufferLocks.push_back(bufferLock); 
       emptyConditions.push_back(emptyCondition);
       fullConditions.push_back(fullCondition);
       reducerBuffers.push_back(ReducerBuffer());

       pthread_cond_init(&(emptyConditions[n]) , NULL);
       pthread_cond_init(&(fullConditions[n]) , NULL);
       pthread_mutex_init(&(bufferLocks[n]) , NULL);


       
     //  cout <<" Main created a reducer thread\n" << flush;
 

   }
  //Spawning reducer threads
  for (int j = 0; j < maxReducers; j++)
   {
    pthread_create(&(rThreads[j]), NULL, reducing , (void *)j);
   }
  //Spawning mapper threads
    for (int m = 0; m <maxMappers; m++)
    {
  
       pthread_create(&(mThreads[m]), NULL , mapping ,(void *)m );
    //   cout << " Main created a mapper thread\n" <<flush;
  
  
    }

  
 //Joining reducer threads
    for (int n = 0; n < maxReducers; n++)
   { 
    pthread_join((rThreads[n]) , NULL); 
   }
//Joining mapper threads
   for (int m = 0; m < maxMappers; m++)
   {
    
    pthread_join((mThreads[m]) , NULL);
    
   }

 //Printing Inverted Index 
  for (auto& x:invertedIndex)
   {
     cout << "{" << x.first << " : " <<x.second << "]\n";
   }
 


//Main thread OUT
 return 0; 
}
