/*
* Copyright 2014 Adam Hollidge <adam@anh.io>
*/


#include <SGP4.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
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
                DateTime end_date,
                int dt,
                std::vector<Tle> tles)
                : start_date_(start_date),
                  end_date_(end_date),
                  dt_(TimeSpan(0, 0, dt)),
                  tles_(tles),
                  active_tle_(0)
{
    std::sort(tles_.begin(), tles_.end(), chron);
    cout << "dt:\t" << dt_ << endl;
}

    std::string GeoJSON() 
    {
        DateTime currtime(start_date_);
        SGP4 sgp(tles_[active_tle_]);
        cout << currtime.ToString() << endl;
        cout << end_date_.ToString() << endl;
        while (currtime < end_date_)
        {
            currtime = currtime.Add(dt_);
            cout << currtime.ToString() << endl;
        }
        return "GeoJSON";
    }

private:
    DateTime            start_date_;
    DateTime            end_date_;
    TimeSpan            dt_;
    std::vector<Tle>    tles_;
    size_t              active_tle_; // index into tles_.
};

int main(int argc, char **argv)
{
    struct tm start_tm, end_tm;
    DateTime start_time, end_time;
    long long dt = 60; // delta time between groundtrack points
	int c;
    char *s_opt = 0, *e_opt = 0, *t_opt = 0;
    char *zero_opt = 0, *one_opt = 0, *two_opt = 0;

    std::string options("0:1:2:3:s:e:t:");
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
            cout << end_time.ToString() << endl;
            break;
        case 't':
            t_opt = optarg;
            dt = atoll(t_opt);
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
            tles.push_back(tle);
    }
    Groundtrack gt(start_time, end_time, dt, std::move(tles));
    cout << gt.GeoJSON() << endl;

    exit (0);
	return 0;
}