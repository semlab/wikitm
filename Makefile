CC=gcc
CXX=g++
RM=rm -f

LIBS=-L/usr/local/lib


wikitm: main.cpp wikitm.cpp wikitm.hpp 
	$(CXX)  -std=c++11 -I /usr/local/boost_1_63_0 \
	main.cpp wikitm.cpp wikitm.hpp rapidxml/rapidxml.hpp rapidxml/rapidxml_print.hpp rapidxml/rapidxml_utils.hpp \
	-o bin/wikitm \
	$(LIBS) -lboost_system -lboost_filesystem -lboost_date_time #-lboost_log -lpthread \
		

clean: 
	$(RM) *.o *.gch *.*~
