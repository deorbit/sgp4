/*
* Copyright 2014 Adam Hollidge <adam@anh.io>
*/


#include <SGP4.h>
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

using namespace std;

// Chronological sort.
bool chron(const Tle& a, const Tle& b)
{
    return a.Epoch() < b.Epoch();
}

class Groundtrack
{
public:
    Groundtrack(DateTime start_date, 
                DateTime end_date_, 
                std::vector<Tle> tles)
                : start_date_(start_date),
                  end_date_(end_date_),
                  tles_(tles)
{
    std::sort(tles_.begin(), tles_.end(), chron);
}

private:
    DateTime start_date_;
    DateTime end_date_;
    std::vector<Tle> tles_;
};

int main(int argc, char **argv)
{
    struct tm start_tm, end_tm;
    DateTime start_time, end_time;
	int c;
    char *s_opt = 0, *e_opt = 0;
    char *zero_opt = 0, *one_opt = 0, *two_opt = 0;

    std::string options("0:1:2:3:s:e:");
    while ( (c = getopt(argc, argv, options.c_str())) != -1) {
        switch (c) {
        case '0':
        	std::cout << "Option 0: " << optarg << "\n";
        	zero_opt = optarg;
        	break;
        case '1':
        std::cout << "Option 0: " << optarg << "\n";
        	one_opt = optarg;
        	break;
        case '2':
        	std::cout << "Option 0: " << optarg << "\n";
        	two_opt = optarg;
            break;
        case 's':
            s_opt = optarg;
            if (strptime(s_opt, "%Y-%m-%d %H:%M:%S", &start_tm) == NULL) {
                cerr << "Error parsing start time.\n";
                exit(0);
            }
            cout << start_tm.tm_mon << endl;
            start_time.Initialise(1900 + start_tm.tm_year, 
                                  start_tm.tm_mon + 1,
                                  start_tm.tm_mday, 
                                  start_tm.tm_hour,
                                  start_tm.tm_min,
                                  start_tm.tm_sec, 
                                  0);
            break;
        case 'e':
            e_opt = optarg;
            if (strptime(e_opt, "%Y-%m-%d %H:%M:%S", &end_tm) == NULL) {
                cerr << "Error parsing end time.\n";
                exit(0);
            }
            end_time.Initialise(1900 + end_tm.tm_year, 
                                end_tm.tm_mon + 1,
                                end_tm.tm_mday, 
                                end_tm.tm_hour,
                                end_tm.tm_min,
                                end_tm.tm_sec, 
                                0);
            break;
        case '?':
            break;
        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }

    // Parse non-option arguments.
    if (optind < argc) {
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        printf ("\n");
    }

    // Handle TLEs piped into stdin.
    string line1, line2;
    vector<Tle> tles;
    while(getline(cin, line1)) {
        getline(cin, line2);
        Tle tle(line1, line2);
        if (tle.Epoch() >= start_time && tle.Epoch() <= end_time)
            cout << tle.Epoch() << endl;
    }
    Groundtrack gt(start_time, end_time, std::move(tles));

    exit (0);
	return 0;
}