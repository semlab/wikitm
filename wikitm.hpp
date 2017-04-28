#include "rapidxml/rapidxml.hpp"
#include "rapidxml/rapidxml_print.hpp"
#include "rapidxml/rapidxml_utils.hpp"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/filesystem.hpp>
//#include <boost/log/trivial.hpp>

#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <cmath>
#include <cstring>
#include <ctime>

#define DUMPFILE_PREFIX "enwiki-20170101-pages-meta-history"
#define PAGE_TAG_START  "<page>"
#define PAGE_TAG_END  "</page>"
#define REVISION_TAG_TITLE "revision"
#define NOWHERE -1;

class wikitm {

	public: 
		wikitm( boost::gregorian::date date_start, 
			boost::gregorian::date_duration date_delta, int date_count, 
			std::string input_folder, std::string output_folder);
		std::vector<std::string> find_pages(std::string& chunk);
		boost::gregorian::date get_time(rapidxml::xml_node<> *revision_node);
		std::vector<std::string> get_latest_revisions(
				std::vector<boost::gregorian::date> timeline, std::string page_s);
		//void wiki_timelapse( boost::gregorian::date date_start, 
		//		boost::gregorian::date_duration date_delta, int date_count, 
		//		std::string dumpfolder, std::string outfolder);
		void run();



		//const std::string PAGE_TAG_START = "<page>";
		//const std::string PAGE_TAG_END = "</page>";
		//const std::string REVISION_TAG_TITLE = "revision";
		//const std::streampos NOWHERE = -1;
		const unsigned long CHUNK_SIZE = 1073741824; // 1GB

		//const char *ISO8601 = "%Y-%m-%dT%H:%M:%SZ";

	private: 
		std::vector<boost::filesystem::path> gen_dumplist(std::string input_folder);
		std::vector<boost::gregorian::date> gen_timeline( boost::gregorian::date date_start, 
				boost::gregorian::date_duration date_delta, int  date_count );
		std::vector<boost::filesystem::path> gen_outfiles( 
				const std::vector<boost::gregorian::date>& timeline, 
				const std::string& output_folder);



		std::vector<boost::filesystem::path> m_dumpfiles;
		std::vector<boost::filesystem::path> m_outfiles;
		std::vector<boost::gregorian::date> m_timeline;
		int m_pages_count = 0;		



};


