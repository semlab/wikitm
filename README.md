# wikitm - Wikipedia Time Machiine 

Use full dump of Wikipedia (including all revision) to rebuild snapshots of wikipedia dumps 
representing the state of Wikipedia for a given timeline. 


Usage: wikitm --input-folder=FOLDER --output-folder=FOLDER --date-start=STARTDATE --date-delta=DELTA --date-count=COUNT [OPTION]...
--help		Display this help
--input-folder=FOLDER		Wikipedia dump file with full history
--output-folder=FOLDER		File name of the generated dump
--date-start=yyy-MM-dd		the time used to generate the dump. Specified with ISO8601 format.
--date-delta=NUMBER		the time gap between each date of the timeline
--date-count=NUMBER		Specify how many date in the timeline 
--verbose		Display processing informations



