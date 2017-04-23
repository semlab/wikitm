#include "wikitm.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cerrno>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>


// --verbose flag
static int verbose_flag;


void help()
{
	std::cout << "Usage: wikitm --input-folder=FOLDER --output-folder=FOLDER --date-start=STARTDATE --date-delta=DELTA --date-count=COUNT [OPTION]...\n";
	std::cout << "--help		Display this help\n";
	std::cout << "--input-folder=FOLDER		Wikipedia dump file with full history\n";
	std::cout << "--output-folder=FOLDER		File name of the generated dump\n";
	std::cout << "--date-start=yyy-MM-dd		the time used to generate the dump. Specified with ISO8601 format.\n";
	std::cout << "--date-delta=NUMBER		the time gap between each date of the timeline\n";
	std::cout << "--date-count=NUMBER		Specify how many date in the timeline \n";
	std::cout << "--verbose		Display processing informations\n";
	std::cout << "  " << std::endl;
}


int main(int argc, char *argv[])
{

	/*
	wikitm wt;
	boost::gregorian::date date_start(boost::gregorian::from_simple_string("2002-01-01"));
	boost::gregorian::date_duration date_delta(365);
	int date_count = 5;
	std::string input_folder = "/media/datahd/dumps.wikimedia.org/enwiki/20170101-uncompressed/history1";
	std::string output_folder = "/media/datahd/dumps.wikimedia.org/enwiki/snapshots-c";
	//*/

	//*
	char* arg_input_folder = NULL; 
	char* arg_output_folder = NULL;
	char* arg_date_start = NULL;
	char* arg_date_delta = NULL;
	char* arg_date_count = NULL;
	// Getting options
	int c;
	while(1){
		static struct option long_options[] = {
			// set flag
			{"verbose", no_argument, &verbose_flag, 1},
			// no flag options
			{"help", no_argument, 0, 'h'},
			{"input-folder", required_argument, 0, 'i'},
			{"output-folder", required_argument, 0, 'o'},
			{"date-start", required_argument, 0, 's'},
			{"date-delta", required_argument, 0, 'd'},
			{"date-count", required_argument, 0, 'c'},
		};
		int option_index = 0;
		c = getopt_long (argc, argv, "hi:o:s:d:c:", long_options, &option_index);
		
		if (c == -1) break; // end of options

		switch(c){
			case 0:
				 // If this option set a flag, do nothing else now. 
				if (long_options[option_index].flag != 0)
					break;
				printf ("option %s", long_options[option_index].name);
				if (optarg)
					printf (" with arg %s", optarg);
				printf ("\n");
				break;
			case 'h':
				help();
				exit(0);
				break;
			case 'i':
				arg_input_folder = optarg;
				break;
			case 'o':
				arg_output_folder = optarg;
				break;
			case 's':
				arg_date_start = optarg;
				break;
			case 'd':
				arg_date_delta = optarg;
				break;
			case 'c':
				arg_date_count = optarg;
				break;
			case '?':
				help();
				exit(0);
				break;
			default:
				help();
				abort();
		}
	}

	if ( arg_input_folder == NULL || arg_output_folder == NULL 
		|| arg_date_start == NULL || arg_date_delta == NULL 
		|| arg_date_count == NULL){
		help();
		abort();
	}
	
	wikitm wt;
	std::string input_folder = std::string(arg_input_folder); 
	std::string output_folder = std::string(arg_output_folder);
	boost::gregorian::date date_start(boost::gregorian::from_simple_string( std::string(arg_date_start) ));
	boost::gregorian::date_duration date_delta( std::atoi(arg_date_delta) );
	int date_count = std::atoi(arg_date_count);
	/*
	std::cout << date_start << std::endl;
	std::cout << date_delta << std::endl;
	std::cout << date_count << std::endl;
	std::cout << input_folder << std::endl;
	std::cout << output_folder << std::endl;
	//*/

	wt.wiki_timelapse( date_start, date_delta, date_count, input_folder, output_folder ); 			

}
