/*
 *    IPS domaca uloha 1.
 *    Paralelni GREP
 *    2016/17
 *    xmarko15
 *    xnanoa00
 *
 *      @TODO osetrit chyby
 *      @TODO vycistit kod
 *      @TODO otestovat
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
std::mutex writelock;               // atomic writing lock

int line_score;                     // current score of a line
char* line;                         // current line pointer
int num_threads_finished = 0;       // number of finished threads
int finished = 0;                   // not finished

/*******************************
    DECLARATIONS
*******************************/

// reads line from input returns c-string
char* read_line(int* res);

// callback function for thread regex check
void f(int ID, char* regex_string, int regex_score);

// error printout and return
int print_err(void);


/*******************************
    MAIN
*******************************/

int main(int argc, char *argv[])
{

    // min score for printing a line
    unsigned int min_score;

    // argument pairs structure
    typedef struct RegexPair
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
        for (i = 3, count = 0 ; i < argc ; i += 2, count++)
        {
            pairs[count].regex_string = argv[i - 1];

            if (sscanf(argv[i], "%d", &(pairs[count].regex_score)) != 1)
                return print_err();
        }
    }

    /****************
        LOCKS SETUP
    *****************/

    int num_locks = pairs_count;

    // resize locks vector
    locks.resize(num_locks);

    for (int i = 0; i < num_locks; i++)
    {
        std::mutex *new_lock = new std::mutex();        // new lock pointer
        locks[i] = new_lock;                            // move pointer to locks vector
        (*(locks[i])).lock();
    }

    /****************
        THREADS SETUP
    *****************/

    int num_threads = pairs_count;

    std::vector<std::thread *> threads;
    threads.resize(num_threads);

    for (int i = 0; i < num_threads; i++)
    {
        std::thread *new_thread = new std::thread(f, i, pairs[i].regex_string, pairs[i].regex_score);
        threads[i] = new_thread;
    }


    /**********************************
       MAGIC
    *********************************/

    int res = 1;
    line_score = 0;
    line = read_line(&res);

    while (res) // while there is something to read
    {

        for (int i=0; i < num_threads; i++)
            locks[i]->unlock();

        while (num_threads_finished != num_threads)
            usleep(1);

        if (line_score >= min_score)
            printf("%s\n", line);

        writelock.lock();

            line_score = 0;
            free(line);
            line = read_line(&res);
            num_threads_finished = 0;

        writelock.unlock();
    }

    finished = 1;
    for (int i=0; i < num_threads; i++)
        locks[i]->unlock();

    /**********************************
       CLEANING & JOINING
    *********************************/
    free(line);

    for (int i = 0; i < num_threads; i++)
    {
        (*(threads[i])).join();
        delete threads[i];
    }

    for (int i = 0; i < num_locks; i++)
    {
        delete locks[i];
    }
    return 0;
}


void f(int ID, char* regex_string, int regex_score)
{
    while(! finished)
    {
        locks[ID]->lock();

        if(finished)
            break;

        std::regex re(regex_string);

        if( std::regex_match(line, re) )
        {
            writelock.lock();
                line_score += regex_score;   // atomic writing to global variable
            writelock.unlock();
        }

        writelock.lock();
            ++num_threads_finished; // atomic writing to global variable
        writelock.unlock();
    }
}



char *read_line(int *res) {
	std::string l;
	char *str;
	if (std::getline(std::cin, l)) {
		str=(char *) malloc(sizeof(char)*(l.length()+1));
		strcpy(str,l.c_str());
		*res=1;
		return str;
	} else {
		*res=0;
		return NULL;
	}
}

int print_err(void)
{
    fprintf(stderr, "USAGE: pgrep MIN_SCORE RE1 SC1 [ RE2 SC2 ] [ RE3 SC3 ] ...\n");
    return 1;
}
