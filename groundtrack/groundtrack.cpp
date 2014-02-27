/*
* Copyright 2014 Adam Hollidge <adam@anh.io>
*/


#include <SGP4.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

static inline std::istream& safeGetline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case EOF:
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}

// Chronological TLE sort.
bool chron(const Tle& a, const Tle& b)
{
    return a.Epoch() < b.Epoch();
}


class Groundtrack
{
public:
    enum Format {
        GeoJSON
    };
    static const int max_prop_days = 7;

    Groundtrack(DateTime start_date, 
                DateTime end_date,
                int dt,
                std::vector<Tle> tles)
                : start_date_(start_date),
                  end_date_(end_date),
                  dt_(TimeSpan(0, 0, dt)),
                  tles_(tles),
                  active_tle_(0),
                  max_terminal_propagation_(max_prop_days, 0, 0, 0, 0)
    {
        std::sort(tles_.begin(), tles_.end(), chron);

        // Roll back end date if it goes too far beyond
        // the last TLE.
        if (!tles_.empty()) {
            Tle last_tle = tles_.back();
            if ((end_date_ - last_tle.Epoch()) > max_terminal_propagation_)
                end_date_ = last_tle.Epoch().Add(max_terminal_propagation_);
        }
    }

    std::string Generate(Groundtrack::Format format) 
    {
        size_t num_tles = tles_.size();

        if (num_tles == 0) return "";

        DateTime currtime(start_date_);
        DateTime tle_transition(currtime.Add(max_terminal_propagation_));
        if (num_tles > 1) tle_transition = TLETransitionTime(active_tle_, 
                                                             active_tle_+1);
        

        SGP4 sgp4(tles_[active_tle_]);
        while (currtime < end_date_)
        {
            Eci eci = sgp4.FindPosition(currtime);
            CoordGeodetic geo = eci.ToGeodetic();
            latlons_.push_back(std::make_tuple(currtime, geo));

            if (currtime >= tle_transition && active_tle_ < num_tles - 1) 
            {
                active_tle_++;
                sgp4.SetTle(tles_[active_tle_]);
                tle_transition = TLETransitionTime(active_tle_, active_tle_+1);
            }
            currtime = currtime.Add(dt_);
        }

        std::string gt_out;

        switch(format)
        {
        case(Groundtrack::Format::GeoJSON):
            gt_out = GenGeoJSON();
        break;
        }
        return gt_out;
    }

private:
    DateTime                                start_date_;
    DateTime                                end_date_;
    TimeSpan                                dt_;
    std::vector<Tle>                             tles_;
    std::vector<std::tuple<DateTime, CoordGeodetic> > latlons_;
    size_t                                  active_tle_; // index into tles_.
    const TimeSpan                          max_terminal_propagation_; // 7 days

    /**
     * Calculate the midpoint in time between TLEs with the
     * given indices into the tle_ vector.
     */ 
    DateTime TLETransitionTime(const size_t tle1, const size_t tle2) const
    {
        DateTime t;

        if (tle1 < tles_.size() - 1 && tle2 < tles_.size()) {
            t = tles_[tle1].Epoch() +
                            (tles_[tle2].Epoch() - 
                             tles_[tle1].Epoch()).Multiply(0.5);
        }
        else 
            t = tles_.back().Epoch().Add(max_terminal_propagation_);

        return t;
    }

    std::string GenGeoJSON()
    {
        std::string geojson_preamble = "{\"type\":\"FeatureCollection\","
                         "\"features\":["
                         "{"
                         "\"type\": \"Feature\","
                         "\"properties\":"
                         "{"
                         "\"name\":\"[...]\""
                         "},"
                         "\"geometry\":"
                         "{"
                         "\"type\":\"LineString\","
                         "\"coordinates\": [";
        std::string geojson_terminator = "]}}]}"; 
        std::string coords = "";

        size_t numpoints = latlons_.size();
        for (size_t i = 0; i < numpoints; ++i)
        {
            std::string comma = ",";
            if (i == numpoints - 1)
                comma = "";
            std::string coord = "[" + std::get<1>(latlons_[i]).ToStringLonLat() + 
                            "]" + comma; 
            coords.append(coord);
        }
        std::string geojson = geojson_preamble + coords + geojson_terminator;
        return geojson;
    }

};

int main(int argc, char **argv)
{
    struct tm start_tm, end_tm;
    DateTime start_time, end_time;
    std::string tle_filename;
    int dt = 60; // delta time between groundtrack points
	int c;
    char *s_opt = 0, *e_opt = 0, *t_opt = 0, *f_opt = 0;
    char *zero_opt = 0, *one_opt = 0, *two_opt = 0;

    std::string options("0:1:2:3:s:e:t:f:");
    while ( (c = getopt(argc, argv, options.c_str())) != -1) {
        switch (c) {
        case '0':
        	zero_opt = optarg;
        	break;
        case '1':
        	one_opt = optarg;
        	break;
        case '2':
        	two_opt = optarg;
            break;
        case 's':
            s_opt = optarg;
            if (strptime(s_opt, "%Y-%m-%d %H:%M:%S", &start_tm) == NULL) {
                std::cerr << "Error parsing start time: " << s_opt << "\n";
                exit(0);
            }
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
                std::cerr << "Error parsing end time: " << e_opt << "\n";
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
        case 't':
            t_opt = optarg;
            dt = atoi(t_opt);
            break;
        case 'f':
            f_opt = optarg;
            tle_filename = f_opt;
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

    // Read TLEs from the input file or piped into stdin.
    std::istream* tle_source;
    std::ifstream tle_file;
    tle_source = &std::cin;
    if (!tle_filename.empty()) {
        tle_file.open(tle_filename.c_str());
        tle_source = &tle_file;
    }
    std::string line1, line2;
    std::vector<Tle> tles;

    while(!safeGetline(*tle_source, line1).eof()) {
        safeGetline(*tle_source, line2);
        if (line1.empty() || line2.empty())
            continue;
        // line1 = rtrim(line1);
        // line2 = rtrim(line2);
        try {
            Tle tle(line1, line2);
            if (tle.Epoch() >= start_time.AddDays(-1.0 * Groundtrack::max_prop_days) && 
                tle.Epoch() <= end_time.AddDays(Groundtrack::max_prop_days)) 
            {
                tles.push_back(tle);
            }
        } catch(TleException& e) {
            std::cerr << "TleException: " << e.what() << "\n";
            continue;   
        }
    }
    Groundtrack gt(start_time, end_time, dt, std::move(tles));
    std::cout << gt.Generate(Groundtrack::Format::GeoJSON) << std::endl;

    exit (0);
	return 0;
}