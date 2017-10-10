/*
 *    IPS domaca uloha 1.
 *    Paralelni GREP
 *    2016/17
 *    xmarko15
 *    xnanoa00
 */

// C
#include <stdio.h>
#include <unistd.h>
#include <string.h>
// C++11
#include <vector>
#include <regex>
#include <thread>
#include <mutex>
#include <queue>
#include <iostream>


/*******************************
    GLOBALS
*******************************/

std::vector<std::mutex *> locks;    // vector of mutexes
std::mutex writelock;

int line_score;                     // current score of a line
char* line;
int num_threads_finished = 0;



/*******************************
    DECLARATIONS
*******************************/

// reads line from input returns c-string
char* read_line(int* end);

// callback function for thread regex check
void f(int ID, char* regex_string, int regex_score);

// error printout and return
int print_err(void);



/*******************************
    MAIN
*******************************/

int main(int argc, char *argv[])
{

    unsigned int min_score; // min score for printing a line

    typedef struct RegexPair // argument pairs structure
    {
        char *regex_string;
        int regex_score;
    } REGEXPAIR;

    /****************
        ARGUMENTS
    *****************/

    if (argc % 2 == 1 || argc < 4)
        return print_err();
    if (sscanf(argv[1], "%d", &min_score) != 1)
        return print_err();

    // number of pairs
    const unsigned int pairs_count = (argc - 1) / 2;

    // new regexpairs array
    REGEXPAIR pairs[pairs_count];

    // explicit scope (because of i & count)
    {
        int i, count;

        // fill pairs struct with data from program arguments
        for (i = 3, count = 0 ; i < argc ; i += 2,count++)
        {
            pairs[count].regex_string = argv[i - 1];

            if (sscanf(argv[i], "%d", &(pairs[count].regex_score)) != 1)
                return print_err();
        }
    }


    /****************
        LOCKS
    *****************/

    int num_locks = pairs_count;

    // resize locks vector
    locks.resize(num_locks);

    // create new locks
    for (int i = 0; i < num_locks; i++)
    {
        std::mutex *new_lock = new std::mutex();        // new lock pointer
        locks[i] = new_lock;                            // move pointer to locks vector
        (*(locks[i])).lock();     /* Pokud je treba, tak vytvoreny zamek muzeme rovnou zamknout */
    }


    /****************
        THREADS
    *****************/

    int num_threads = pairs_count; // == number of regex pairs

    std::vector<std::thread *> threads; /* pole threadu promenne velikosti */
    threads.resize(num_threads);

    // create new threads
    for (int i = 0; i < num_threads; i++)
    {
        std::thread *new_thread = new std::thread(f, i, pairs[i].regex_string, pairs[i].regex_score);
        threads[i] = new_thread;
    }

    /**********************************
	 * LINE READING
	 * ********************************/

    int end;
    line_score = 0;
    line = read_line(&end);

    printf("end is %d \n", end);

    while (! end) // while there is something to read
    {
        // printf("num_threads_finished = %d\n", num_threads_finished);
        // printf("line_score = %d\n", line_score);
        // printf("line_score = %d\n", min_score);

        // unlock lock for each thread
        for (int i=0; i < num_threads; i++)
            locks[i]->unlock();

        while (num_threads_finished != num_threads)
            usleep(1);

        if (line_score >= min_score)
            printf("%s\n", line);
        printf("been here\n");
        line_score = 0; // reset back to 0
        free(line);
        line = read_line(&end);
    }


    /**********************************
	   CLEANING & JOINING
	*********************************/

    // join all threads and delete their memory
    for (int i = 0; i < num_threads; i++)
    {
        (*(threads[i])).join();
        delete threads[i];
    }

    for (int i = 0; i < num_locks; i++)
    {
        delete locks[i];
    }


    printf("everything finished\n");
    return 0;
}



void f(int ID, char* regex_string, int regex_score)
{

        while(! locks[ID]->try_lock())
        {
            usleep(1);
        }

        printf("UThread %i started with regex : %s w/ score of : %i\n", ID, regex_string, regex_score);

        std::regex re(regex_string);

        if( std::regex_match(line, re) )
        {
            writelock.lock();
            line_score += regex_score;   // atomic writing to global variable
            writelock.unlock();
        }

        ++num_threads_finished;
        printf("thread %i | num_threads_finished %i | line_score %i\n", ID, num_threads_finished, line_score);
        locks[ID]->unlock(); // TODO

    printf("Thread %i finished... \n", ID);
}



char* read_line(int* end)
{
    std::string line;
    char *str;

    if (std::getline(std::cin, line))
    {
        str = (char *)malloc(sizeof(char) * (line.length() + 1));
        strcpy(str, line.c_str());  //convert c++ string to c-string and copy to str char*
        *end = 0;
        return str;
    }
    else
    {
        *end = 1;
        return NULL;
    }
}

int print_err(void)
{
    fprintf(stderr, "USAGE: pgrep MIN_SCORE RE1 SC1 [ RE2 SC2 ] [ RE3 SC3 ] ...\n");
    return 1;
}




//
