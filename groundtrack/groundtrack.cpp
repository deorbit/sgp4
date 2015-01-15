/*
* Copyright 2014 Adam Hollidge <adam@anh.io>
*/

#include <Groundtrack.h>

int main(int argc, char **argv)
{
    struct tm start_tm, end_tm;
    DateTime start_time, end_time;
    std::string tle_filename;
    int dt = 60; // delta time between groundtrack points
	int c;
    char *s_opt = 0, *e_opt = 0, *t_opt = 0, *f_opt = 0;
    char *zero_opt = 0, *one_opt = 0, *two_opt = 0;
    bool verbose = false;

    std::string options("0:1:2:3:s:e:t:f:v");
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
        case 'v':
            verbose = true;
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

    if (verbose) std::cerr << "Generating track from " << start_time.ToString() <<
        " to " << end_time.ToString() << ".\nReading TLEs.\n";
    
    while(!safeGetline(*tle_source, line1).eof()) 
    {
        safeGetline(*tle_source, line2);
        if (line1.empty() || line2.empty())
        {
            continue;
        }

        if (line1.length() != 69 || line2.length() != 69)
        {
            continue;
        }

        // line1 = rtrim(line1);
        // line2 = rtrim(line2);
        try {
            Tle tle(line1, line2);
            if (tle.Epoch() >= start_time.AddDays(-1.0 * Groundtrack::max_prop_days) && 
                tle.Epoch() <= end_time.AddDays(Groundtrack::max_prop_days)) 
            {
                tles.push_back(tle);
            }
        } catch(TleException& e) 
        {
            if (verbose) std::cerr << "TleException: " << e.what() << "\n";
            continue;   
        }
    }
    if (verbose) std::cerr << "Done reading TLEs.\nGenerating groundtrack.\n";

    Groundtrack gt(start_time, end_time, dt, std::move(tles));
    std::cout << gt.Generate(Groundtrack::Format::GeoJSON) << std::endl;

    if (verbose) std::cerr << "Done generating groundtrack. Exiting.\n";

    exit (0);
	return 0;
}