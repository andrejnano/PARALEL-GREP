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
int32_t score;
int32_t start_score;
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

int print_err(void) {
    fprintf(stderr, "USAGE: pgrep MIN_SCORE RE1 SC1 [ RE2 SC2 ] [ RE3 SC3 ] ...\n");
    return 1;
}
int main(int argc, char *argv[])
{
    if (argc % 2 == 1 || argc < 4)
        return print_err();
    if (sscanf(argv[1], "%d", &start_score) != 1) {
        return print_err();
    }
    score = -start_score;
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
    // uint8_t match = 0;
    int res;
    line = read_line(&res);
    char *regex[(argc - 1) / 2];
    int32_t local_score[(argc - 1) / 2];
    uint32_t count = 0;
    // preparing inputs into arrays starting from 0
    for (uint32_t i = 3; i < argc;i += 2) {
        regex[count] = argv[i - 1];
        printf("i %s\n", argv[i - 1]);
        if (sscanf(argv[i], "%d", &(local_score[count])) != 1) {
            local_score[count] = 0;
        }
        count++;
    }

    while (res)
    {
        // non parallel version
        std::string s (line);
        for (uint32_t j = 0; j < count;j++) { // TODO create 'count' threads
            std::regex re(regex[j]);
            std::smatch m;
            if (std::regex_match ( s, m, re )) {
                score += local_score[j];
            }
            if (score >= 0) {
                printf("%s\n", line);
            }
        }

        score = -start_score;
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
