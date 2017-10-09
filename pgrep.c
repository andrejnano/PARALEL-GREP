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

// logging for debug
#define LOG(x) printf(x "\n")

/*****************************************************************/
// KOSTRA PGREP.C
/**************************************************************** */
std::vector<std::mutex *> zamky;

char *line;

char *read_line(int *res)
{
    std::string line;
    char *str;
    if (std::getline(std::cin, line))
    {
        str = (char *)malloc(sizeof(char) * (line.length() + 1));
        strcpy(str, line.c_str());
        *res = 1;
        return str;
    }
    else
    {
        *res = 0;
        return NULL;
    }
}

void f(int ID)
{
    printf("Thread %i started\n", ID);
}

int main(int argc, char *argv[])
{
    /*******************************
	 * Inicializace threadu a zamku
	 * *****************************/
    int num = 10;
    int num_zamky = 15;
    std::vector<std::thread *> threads; /* pole threadu promenne velikosti */

    /* vytvorime zamky */
    zamky.resize(num_zamky); /* nastavime si velikost pole zamky */
    for (int i = 0; i < num_zamky; i++)
    {
        std::mutex *new_zamek = new std::mutex();
        zamky[i] = new_zamek;
        /* Pokud je treba, tak vytvoreny zamek muzeme rovnou zamknout */
        (*(zamky[i])).lock();
    }

    /* vytvorime thready */
    threads.resize(num); /* nastavime si velikost pole threads */
    for (int i = 0; i < num; i++)
    {
        std::thread *new_thread = new std::thread(f, i);
        threads[i] = new_thread;
    }
    /**********************************
	 * Vlastni vypocet pgrep
	 * ********************************/
    int res;
    line = read_line(&res);
    std::regex re(argv[1]);
    std::string s (line);
    std::smatch m;
    while (res)
    {

        std::regex_search ( s, m, re );
        for (unsigned i=0; i<m.size(); ++i) {
            std::cout << "match " << i << " (" << m[i] << ") ";
            std::cout << "at position " << m.position(i) << std::endl;
        }
        

        printf("%s\n", line);
        free(line); /* uvolnim pamet */
        line = read_line(&res);
    }

    /**********************************
	 * Uvolneni pameti
	 * ********************************/

    /* provedeme join a uvolnime pamet threads */
    for (int i = 0; i < num; i++)
    {
        (*(threads[i])).join();
        delete threads[i];
    }
    /* uvolnime pamet zamku */
    for (int i = 0; i < num_zamky; i++)
    {
        delete zamky[i];
    }

    printf("everything finished\n");
}
