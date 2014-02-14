/*
* Copyright 2014 Adam Hollidge <adam@anh.io>
*/


#include <SGP4.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int c;
    int digit_optind = 0;
    int gopt = 0, bopt = 0;
    char *zero_opt = 0, *one_opt = 0, *two_opt = 0;

    /*

    */
    std::string options("0:1:2:3:g");
    while ( (c = getopt(argc, argv, options.c_str())) != -1) {
        int this_option_optind = optind ? optind : 1;
        switch (c) {
        case '0':
        	std::cout << "Option 0: " << optarg << "\n";
        	zero_opt = optarg;
        	break;
        case '1':
        std::cout << "Option 0: " << optarg << "\n";
        	zero_opt = optarg;
        	break;
        case '2':
        	std::cout << "Option 0: " << optarg << "\n";
        	zero_opt = optarg;
            break;
        case 'g':
        	break;
        case '?':
            break;
        default:
            printf ("?? getopt returned character code 0%o ??\n", c);
        }
    }
    if (optind < argc) {
        printf ("non-option ARGV-elements: ");
        while (optind < argc)
            printf ("%s ", argv[optind++]);
        printf ("\n");
    }
    exit (0);
	std::cout << argc << std::endl;
	return 0;
}