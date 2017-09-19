//#define BOOST_LOG_DYN_LINK 1

#include "wikitm.hpp"


namespace fs = boost::filesystem;


wikitm::wikitm(){
	m_pages_count = 0;

}



wikitm::wikitm( boost::gregorian::date date_start, 
		boost::gregorian::date_duration date_delta, int date_count, 
		std::string input_folder, std::string output_folder){

	m_dumpfiles = gen_dumplist(input_folder); 
	m_timeline = gen_timeline(date_start, date_delta, date_count);
	m_outfiles = gen_outfiles(m_timeline, output_folder);
	m_pages_count = 0;

}






std::vector<std::string> wikitm::find_pages(std::string& chunk){
	std::vector<std::string> pages;
	std::string page;
	bool has_page_start = true;
	bool has_page_end = true;
	size_t i_pos = 0;
	size_t i_plength = 0;
	size_t i_pstart = 0;
	size_t i_pend = 0;

	//while(has_page_end){  
	while(true){  
		has_page_start = false;
		has_page_end = false;
		i_pstart = chunk.find( PAGE_TAG_START, i_pos);
		i_pend = chunk.find( PAGE_TAG_END, i_pos);

		if ( i_pstart != std::string::npos ){
			has_page_start = true;
		}
		if ( i_pend != std::string::npos ){
			has_page_end = true;
			i_pend += std::string(PAGE_TAG_END).size();
		}

		if ( has_page_start && has_page_end ) {
			if ( i_pstart < i_pend ) {
				i_plength = i_pend - i_pstart;
				page = chunk.substr(i_pstart, i_plength);
				pages.push_back( page );
			} 
			else { 
				has_page_end = false;
			}
			i_pos = i_pend;
		} 
		else if ( !has_page_start && has_page_end ) {
			i_pos = i_pend;
		}
		else if ( has_page_start && !has_page_end ) {
			chunk = chunk.substr(i_pstart, std::string::npos);
			break;
		}
		else if ( !has_page_start && !has_page_end) {
			break;
		}
	}
	return pages;
}




// */
boost::gregorian::date wikitm::get_time(rapidxml::xml_node<> *revision_node){
	boost::gregorian::date d;
	rapidxml::xml_node<> *timestamp_node = revision_node->first_node("timestamp");
	if ( timestamp_node != NULL ){ 
		std::string date_str = timestamp_node->value();
		date_str.erase(10);
		d = boost::gregorian::from_simple_string(date_str);
	}
	return d;
}




//std::vector<std::string> wikitm::get_latest_revisions( std::vector<boost::gregorian::date> timeline, 
//		std::string page_s ) {
std::vector<std::string> wikitm::get_latest_revisions( std::vector<boost::gregorian::date> timeline, 
		char* page_s ) {
	// TODO  Get the  effective range 
	std::vector<std::string> latest_revisions(timeline.size()); 
	int i_timeline = 0;
	//int i_revisions = 0;

	try {
		rapidxml::xml_document<> page_doc;
		//page_doc.parse<0>(&page_s[0]);
		page_doc.parse<0>(page_s);
		rapidxml::xml_node<> *page_node = page_doc.first_node("page");
		if ( page_node == NULL ){
			std::cout << "Parsing yield a NULL node" << std::endl;
			return latest_revisions; // TODO throw an exception
		} 
		rapidxml::xml_node<> *ith_revision_node = page_node->first_node(REVISION_TAG_TITLE);
		if ( ( timeline[timeline.size()-1] < get_time(page_node->first_node(REVISION_TAG_TITLE)) ) || 
				( timeline[0] > get_time(page_node->last_node(REVISION_TAG_TITLE)) ) ){
			std::cout << "Timeline is out of wikipedia timeline range" << std::endl;
			return latest_revisions; // TODO throw exception
		}
		while ( timeline[i_timeline] < get_time(ith_revision_node)  
				&& i_timeline < timeline.size() ) {
			latest_revisions[i_timeline] = "";
			std::cout << "Page has no revision for this date" << std::endl;
			i_timeline++;
		}
		while( i_timeline < timeline.size() && ith_revision_node != NULL ) {
			auto ith_revision_time = get_time(ith_revision_node);
			std::cout << "timeline[" << i_timeline << "]='" << timeline[i_timeline] \
				<< "' ith_revision_time='" << ith_revision_time << "'" << std::endl;
			if ( ith_revision_time > timeline[timeline.size() - 1]){
				if ( ith_revision_node->previous_sibling(REVISION_TAG_TITLE) != NULL ){
					for ( int i = i_timeline; i < timeline.size(); i++){
						std::stringstream ss ;
						ss  << PAGE_TAG_START << std::endl \
							<< *(ith_revision_node->previous_sibling(REVISION_TAG_TITLE)) \
							<< PAGE_TAG_END ; // TODO ith_revision_node->previous_sibling(REVISION_TAG_TITLE)
						latest_revisions[i_timeline] = ss.str();
					}
				}
				else {
					for ( int i = i_timeline; i < timeline.size(); i++){
						latest_revisions[i_timeline] = "";
					}
				}
				break;
			}
			else if( ith_revision_time <= timeline[i_timeline]  ){
				if ( ith_revision_node->next_sibling(REVISION_TAG_TITLE) == NULL ){
					for ( int i = i_timeline; i < timeline.size(); i++){
						std::stringstream ss ;
						ss 	<< PAGE_TAG_START << std::endl \
							<< *ith_revision_node << PAGE_TAG_END ; // TODO ith_revision_node->previous_sibling(REVISION_TAG_TITLE)
						latest_revisions[i_timeline] = ss.str();
					}	
					break;
				}
				else {
					ith_revision_node = ith_revision_node->next_sibling(REVISION_TAG_TITLE);
				}
			}
			else if ( ith_revision_time > timeline[i_timeline] ){
				if ( ith_revision_node->previous_sibling(REVISION_TAG_TITLE) != NULL ){
					std::stringstream ss ;
					ss << PAGE_TAG_START << std::endl << *(ith_revision_node->previous_sibling(REVISION_TAG_TITLE)) << PAGE_TAG_END;
					latest_revisions[i_timeline] = ss.str();
				}
				else {
					for (int i=i_timeline; i >= 0; i--){
						latest_revisions[i_timeline] = "";
					}
				}
				i_timeline++;
			}
		}
	}
	catch ( rapidxml::parse_error& e) {
	  	std::cerr << "Parsing ...failed!" << std::endl;
	  	std::cout << "Parsing ...failed!" << std::endl;
		std::cerr << "\t" << e.what() << " at " << e.where<std::streampos>(); 
		std::cerr <<  std::endl;
		return latest_revisions;
	}

	std::cout << "Number of revisions for the page"<< latest_revisions.size() << std::endl;
	return latest_revisions;
}




std::string wikitm::read_chunk(std::ifstream& infile, /*size_t offset,*/ size_t size){
	std::cout << "Reading from file position: " << infile.tellg() << std::endl;
	std::string contents;
	contents.resize(size);//size);
	infile.read(&contents[0], size);// contents.size());
	std::cout << "Read : " << contents.size() << std::endl;
	if(!infile.bad()){
		//std::cout << "Content read from the file:\n"<< contents << std::endl;
		return contents;
	}
	std::cerr << "Error reading file..." << std::endl;
	throw(errno);
	
}


void wikitm::show_progress(size_t read_size, size_t file_size, 
		std::chrono::system_clock::time_point starttime ){
	//double file_progress = (double(read_size)/double(file_size)) * 100  ;
	int file_progress = (double(read_size)/double(file_size)) * 100  ;
	auto t_file_delta = std::chrono::system_clock::now() - starttime;
	std::cout << "File " << m_current_file_index +1 << "/" <<  m_dumpfiles.size() \
		<< " [" << file_progress << "%], ellapsed time: " \
		<< std::chrono::duration_cast<std::chrono::minutes>(t_file_delta).count() \
		<< "min"<< std::endl; 
}



void wikitm::run(){
	char* trail = new char[0];
	char* chunk = new char[0];
	char* buf;
	auto t_file_end = std::chrono::system_clock::now();
	auto t_file_start = std::chrono::system_clock::now();
	auto t_file_delta = t_file_start - t_file_end;

	long nb_pages_done = 0; // TODO 
	//std::vector<char*> pages;
	std::vector<std::string> pages;
	long chunks_read = 0;
	std::string chunk_str; 
	std::string prev_chunk; 
	size_t buf_size = CHUNK_SIZE;
	size_t size_read = 0;
	size_t trail_len = 0;

	boost::filesystem::path dumpfilepath;
	size_t dumpfile_size;
	int file_progress = 0;


	std::cout << "Dumpfiles:" << std::endl;
	for( int i = 0; i < m_dumpfiles.size(); i++){
		std::cout << "\t" << m_dumpfiles[i] << std::endl;
	}
	std::cout << "Timeline:" << std::endl;
	for( int i = 0; i < m_timeline.size(); i++){
		std::cout << "\t" << m_timeline[i] << std::endl;
	}
	std::cout << "Output Files:" << std::endl;
	for( int i = 0; i < m_outfiles.size(); i++){
		std::cout << "\t" << m_outfiles[i] << std::endl;
	}
	
	for ( int i = 0; i < m_dumpfiles.size(); i++ ){
		m_current_file_index = i;
		nb_pages_done = 0; // TODO 
		chunks_read = 0;
		pages.clear();
		chunk_str = ""; 
		prev_chunk = ""; 
		size_read = 0;
		trail_len = 0;

		std::cout << "Processing file" << m_dumpfiles[i] << std::endl;
		dumpfilepath = m_dumpfiles[i];
		//dumpfile_size = fs::file_size(m_dumpfiles[i].c_str()); 
		file_progress = 0;
		std::ifstream infile(dumpfilepath.c_str());
		infile.seekg(0, infile.end);
		dumpfile_size = infile.tellg();
		infile.seekg(0, infile.beg);
		infile.clear();

		t_file_start = std::chrono::system_clock::now();
		while( infile.good() /*true*/ ){

			chunk_str = read_chunk(infile, buf_size); // TODO copy by ref ? 
			pages = find_pages(chunk_str);
			std::cout << "Number of pages found: " << pages.size() << std::endl;
			prev_chunk = chunk_str; 
			if( prev_chunk.size() > buf_size ){// check if the chunk will get too big 
				std::cout << "FATAL ERROR: Not enough space to parse File "<< i << std::endl;
				std::cout << "Try increasing the chunk size and run again "<< i << std::endl;
				throw (errno);
			}
			chunks_read += 1;
			for ( int i_page = 0; i_page < pages.size(); i_page++ ){
				auto page_revisions = get_latest_revisions(m_timeline, &pages[i_page][0]);
				std::cout << "Number of revisions got: " << page_revisions.size() << std::endl;
				for (int i_revision = 0; i_revision < page_revisions.size(); i_revision++ ){
					std::ofstream fout;
					fout.open( m_outfiles[i_revision].string() , std::ios_base::app);
					fout << page_revisions[i_revision] << std::endl;
					fout.close();
				}
			}
			nb_pages_done += pages.size();
			size_read += buf_size;
			show_progress(size_read, dumpfile_size, t_file_start);
		}
		std::cout << "File " << i+1 << " done " << std::endl; 
	}
}




std::vector<boost::filesystem::path> wikitm::gen_dumplist(std::string filename){
	std::cout << "Generate dump with file" << std::endl;
	std::vector<boost::filesystem::path> dumplist;
	fs::path dumpfile(filename);
	dumplist.push_back(dumpfile);
	std::cout << "adding " << dumpfile << std::endl;
	this->m_dumpfiles = dumplist;
	return dumplist; 
}




std::vector<boost::filesystem::path> wikitm::gen_dumplist_from_file(std::string input_file_path){
	std::vector<boost::filesystem::path> dumplist;
	std::string line;
	std::ifstream input_file(input_file_path);
	if(input_file.is_open() ) {
		while(!input_file.eof()){
			std::getline(input_file,line);
			fs::path dumpfile(line);
			dumplist.push_back(dumpfile);
		}
	}
	else{
		std::cerr << "Unable to open '" << input_file_path << "'" << std::endl;
	}
	this->m_dumpfiles = dumplist;
	return dumplist; 
}




std::vector<boost::gregorian::date> wikitm::gen_timeline(boost::gregorian::date date_start, 
			boost::gregorian::date_duration date_delta, int date_count) {
	std::vector<boost::gregorian::date> timeline;	
	boost::gregorian::date t = date_start;
	for (int i = 0; i < date_count; i++ ){
		timeline.push_back(t);
		t += date_delta;
	}
	this->m_timeline = timeline;
	return timeline;
}

std::vector<boost::gregorian::date> wikitm::gen_timeline_from_file( std::string timeline_file_path ){
	std::vector<boost::gregorian::date> timeline;	
	std::string line;
	std::ifstream timeline_file(timeline_file_path);
	if(timeline_file.is_open()){
		while(!timeline_file.eof()){
			std::getline(timeline_file,line);
			boost::gregorian::date d(boost::gregorian::from_simple_string( line ));
			timeline.push_back(d);
		}
	}
	else{
		std::cerr << "Unable to open '" << timeline_file_path << "'" << std::endl;
	}
	this->m_timeline = timeline;
	return timeline;
}



std::vector<boost::filesystem::path> wikitm::gen_outfiles(
			const std::vector<boost::gregorian::date>& timeline, 
			const std::string& output_folder){
	std::vector<fs::path> outfiles;
	std::string outfilename;
	fs::path outdir(output_folder);
	for ( int i = 0; i < timeline.size(); i++ ){
		outfilename = boost::gregorian::to_iso_string(timeline[i]);
		outfilename += ".xml";
		fs::path outfile(outfilename);
		outfiles.push_back( outdir / outfile );
	}
	this->m_outfiles = outfiles;
	return outfiles;
}


std::vector<boost::filesystem::path> wikitm::gen_outfiles( const std::string& output_folder ){
	std::vector<fs::path> outfiles;
	std::string outfilename;
	fs::path outdir(output_folder);
	for ( int i = 0; i < this->m_timeline.size(); i++ ){
		outfilename = boost::gregorian::to_iso_string(this->m_timeline[i]);
		outfilename += ".xml";
		fs::path outfile(outfilename);
		outfiles.push_back( outdir / outfile );
	}
	this->m_outfiles = outfiles;
	return outfiles;

}


