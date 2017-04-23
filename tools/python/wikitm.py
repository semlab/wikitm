#!/usr/bin/python
import os, sys, getopt#, io
import logging
import re, string
#import psutil
import datetime
import multiprocessing as mproc


logging.basicConfig(format='%(asctime)s : %(levelname)s : %(message)s', level=logging.INFO)

DEFAULT_CHUNK_SIZE = 1073741824 # 1GB # double of '</page>' tag len is recommended
NOWHERE = -1
PAGE_START = "<page>"
PAGE_END = "</page>"
ISO8601 = "%Y-%m-%dT%H:%M:%SZ"
MEDIAWIKI_MIME = '<mediawiki xmlns="http://www.mediawiki.org/xml/export-0.10/" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.mediawiki.org/xml/export-0.10/ http://www.mediawiki.org/xml/export-0.10.xsd" version="0.10" xml:lang="en">\n'

m_processed_page = 0

# Regex patterns
P_TITLE_TAG = re.compile("<title>.*?</title>")
P_REVISION_TAG = re.compile("<revision>.*?</revision>", re.DOTALL)
P_TIMESTAMP_TAG = re.compile("<timestamp>.*?</timestamp>")
P_TIMESTAMP = re.compile("\d\d\d\d-\d\d-\d\dT\d\d:\d\d:\d\dZ")


# Human reading memory size 
def memsize_h(obj):
    unit_delim = 1024
    t_suffixes = ["B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"]
    mem_size = sys.getsizeof(obj)
    s_i = 0
    while(mem_size > unit_delim):
        mem_size = mem_size / unit_delim
        s_i+=1
    if ( s_i > len(t_suffixes) ):
        return "TOO HUGE!!"
    return str(mem_size) + t_suffixes[s_i]



def gen_dumplist(path):
    dumpfiles = []
    for (root, dirnames, filenames) in os.walk(path) :
        for filename in filenames: 
            if ( string.find( filename, ".xml" ) > -1 ):
                f = open(os.path.join(root,filename))
                line = f.readline()
                if ( line == MEDIAWIKI_MIME ):
                    dumpfiles.append(filename)
                f.close()
    return dumpfiles



def gen_timeline(startdate, delta, count):
    """
    startdate: datetime
    delta: timedelta
    count: number of datetime wanted for the timeline
    """
    timeline = []
    inc_datetime = startdate
    for i in range(count) :
        timeline.append(inc_datetime)
        inc_datetime += delta
    return timeline



def find_pages(chunk):
    pages = []
    while ( True ) :
        has_page_start = False
        has_page_end = False
        i_pstart = chunk.find( PAGE_START )
        if ( i_pstart > -1 ) :
            has_page_start = True
        i_pend = chunk.find( PAGE_END )
        if ( i_pend > -1 ) :
            has_page_end = True
            i_pend += len(PAGE_END)

    
        if ( has_page_start and has_page_end ) :
            if ( i_pstart < i_pend ) :
                pages.append( chunk[i_pstart:i_pend] )
            else :
                has_page_end = False
            chunk = chunk[i_pend:]
        elif ( not has_page_start and has_page_end ):
            chunk = chunk[i_pend:]
        elif ( has_page_start and not has_page_end):
            chunk = chunk[i_pstart:]
            break
        elif ( not has_page_start and not has_page_end ):
            break
    return pages, chunk
    



def get_datetime(revision_s):
    revision_ts = None
    m_ts_tag = P_TIMESTAMP_TAG.search(revision_s)
    if ( m_ts_tag != None ):
        revision_ts = datetime.datetime.strptime(m_timestamp.group(), ISO8601 )
    return revision_ts


def get_latest_revisions(timeline, page_s):
    # TODO Ensure timeline is ordered
    latest_revisions = []
    i = 0
    while( i < len(timeline) ):
        latest_revisions.append(None)
        i += 1
    #latest_revisions_time = [ len( timeline ) ]
    sys.stdout.write("\tsearching latest revisions ...")
    revisions = P_REVISION_TAG.findall(page_s)
    sys.stdout.write("in " + str(len(revisions)) + " revisions: ")
    ind = 0
    ind_latest = 0
    while ( ind < len(revisions) ) : #for revision in revisions:
        revision_ts = get_datetime(revisions[ind])
        if ( revision_ts == None ):
            continue
        if ( revision_ts <= timeline[ len( timeline ) - 1 ] ) : 
            if ( revision_ts > timeline[ind_latest] ):
                if ( ind > 0 ): 
                    latest_revisions[ind_latest] = revisions[ind-1]
                else :
                    latest_revisions[ind_latest] = ""
                ind_latest += 1
            ind += 1
        else :
            break
    while ( ind_latest < len( timeline ) - 1 ):
        if ( ind_latest > 0 ):
            latest_revisions[ind_latest] = latest_revisions[ind_latest - 1]
        else :
            latest_revisions[ind_latest] = ""
        ind_latest += 1

    return latest_revisions 




def get_latest_revision(timestamp, page_s):
    latest_revision = None
    latest_revision_ts = None
    sys.stdout.write("\tsearching latest revision...")
    revisions = P_REVISION_TAG.findall(page_s)
    sys.stdout.write("in " + str(len(revisions)) + " revisions: ")
    page_latest_rev = ""
    for revision in revisions:
        m_ts_tag = P_TIMESTAMP_TAG.search(revision)
        if ( m_ts_tag == None): 
            #sys.stdout.write("skipped (no <timestamp> tag) ")
            continue
        m_timestamp = P_TIMESTAMP.search(m_ts_tag.group())
        if ( m_timestamp == None ): 
            #sys.stdout.write("skipped (no <datetime> tag)")
            continue
        revision_ts = datetime.datetime.strptime(m_timestamp.group(), ISO8601)
        if (timestamp > revision_ts):
            if (latest_revision == None):
                latest_revision = revision
                latest_revision_ts = revision_ts
            elif ( latest_revision_ts < revision_ts):
                latest_revision = revision
                latest_revision_ts = revision_ts
        else : 
            # We suppose the revisions are ordered chronologically, for speed
            sys.stdout.write("...pass dates in revision...")
            break 
    if ( latest_revision == None ) :
        pass
        sys.stdout.write("...not found !\n")
    else :
        page_latest_rev = "<page>\n\t" + latest_revision + "\n</page>"
        sys.stdout.write("...found !\n")
    return page_latest_rev + "\n"




        

    

def wiki_timemachine(before_datetime, input_filename, output_filename, 
        continue_flag = True):
    global m_processed_page
    chunk_size = DEFAULT_CHUNK_SIZE
    pages = []
    chunk = ""
    prev_chunk = ""
    fin = None
    fout = None

    try :
        fin_size = os.path.getsize(input_filename)
        fin = open(input_filename, 'r')
        fout_flag =  'w'
        if ( continue_flag ): fout_flag = 'a'
        fout = open(output_filename, fout_flag)
        chunks_read = 0
        filtering_start = datetime.datetime.now()
        sys.stdout.write("\nStart filtering:\n")
        sys.stdout.write(" File: " + input_filename + "\n")
        sys.stdout.write(" Filtering time: " + str(before_datetime) + "\n")
        sys.stdout.write(" Save in : " + output_filename + "\n")
        while (True): 
            chunks_read += 1
            sys.stdout.write("Reading chunk No"+ str(chunks_read) + "... ")
            chunk =  fin.read(chunk_size)
            if ( chunk != "" ) :
                chunk =  prev_chunk + chunk 
                # TODO check if chunk get too big if not encounting any tag page for a while
                pages, prev_chunk = find_pages(chunk)
                sys.stdout.write(str(len(pages)) + " pages found...\n")
                revisions = []
                for page in pages :
                    revisions.append ( get_latest_revision(before_datetime, page) )
                for revision in revisions :
                    fout.write( revision + "\n" )
                m_processed_page += len(pages)
                sys.stdout.write( str(m_processed_page) + " pages processed\n" )
                sys.stdout.write(input_filename + " | " + str(before_datetime) + 
                        " | " + str( 100 * (chunks_read * chunk_size)/fin_size ) + "%\n" )
            else : 
                break
        sys.stdout.write("\nFiltering complete:\n")
        sys.stdout.write(" File: " + input_filename + "\n")
        sys.stdout.write(" Filtering time: " + str(before_datetime) + "\n")
        sys.stdout.write(" Saved in : " + output_filename + "\n")
        sys.stdout.write(" Done in : " + str( datetime.datetime.now() - filtering_start ) + "\n")

    except Exception, e :
        sys.stdout.write("skipped (processing/parsing error)\n")
        print "Exception : %s" % e

    finally : 
        if (fin != None and not fin.closed) : fin.close()
        if (fout != None and not fout.closed) : fout.close()





def wiki_timelapse(startdate, delta, count, dumpfolder, outfolder):
    n_proc = 5
    dumpfiles = gen_dumplist(dumpfolder)
    timeline = gen_timeline(startdate, delta, count)
    for dumpfile in dumpfiles :
        print "Processing file " + dumpfile
        file_start_time = datetime.datetime.now()
        dumpfilepath = os.path.join(dumpfolder, dumpfile)
        for i_datetime in timeline :
            outfilepath = os.path.join(outfolder, 
                    i_datetime.strftime("%Y%m%d_%H%M%S")+ ".xml")
            print " Extracting latest revision by: " + str(i_datetime)
            revision_start_time = datetime.datetime.now()
            wiki_timemachine(i_datetime, dumpfilepath, outfilepath,
                    True)
            revision_time_span = datetime.datetime.now() - revision_start_time  
            print " Revisions by: " + str(i_datetime)+ " extracted in " + str(revision_time_span)
        file_time_span = file_start_time - datetime.datetime.now()
        print dumpfile + "Processing done in " + str()
    print "ALL " + str(len(dumpfiles)) + " FILES DONE!!!"




def wiki_timelapse2(startdate, delta, count, dumpfolder, outfolder):
    global m_processed_page
    chunk_size = DEFAULT_CHUNK_SIZE
    pages = []
    chunk = ""
    prev_chunk = ""
    fin = None
    fout = None

    dumpfiles = gen_dumplist(dumpfolder)
    timeline = gen_timeline(startdate, delta, count)
    outfiles = gen_outfiles(timeline, outfolder)
    try:
        for dumpfile in dumpfiles :
            print "Processing file " + dumpfile
            dumpfilepath = os.path.join(dumpfolder, dumpfile)
            file_start = datetime.datetime.now()
            fin_size = os.path.getsize(dumpfilepath)
            chunks_read = 0
            fin_size = os.path.getsize(dumpfilepath)
            print "opening " + dumpfilepath + "\n"
            fin = open(dumpfilepath, 'r')
            #file_start_time = datetime.datetime.now()

            while (True): 
                chunks_read += 1
                sys.stdout.write("Reading chunk No"+ str(chunks_read) + "... ")
                chunk =  fin.read(chunk_size)
                if ( chunk != "" ) :
                    chunk =  prev_chunk + chunk 
                    # TODO check if chunk get too big if not encounting any tag page for a while
                    pages, prev_chunk = find_pages(chunk)
                    sys.stdout.write(str(len(pages)) + " pages found...\n")
                    page_revisions = []
                    for page in pages :
                        page_revisions = get_latest_revisions(timeline, page)
                        for i in range( len(timeline) ) :
                            fout = open(outfile[i], 'a')
                            fout.write( page_revisions[i] + "\n" )
                            fout.close()
                    m_processed_page += len(pages)
                    sys.stdout.write( str(m_processed_page) + " pages processed\n" )
                    sys.stdout.write(input_filename + " | " + str(before_datetime) + 
                            " | " + str( 100 * (chunks_read * chunk_size)/fin_size ) + "%\n" )
                else : 
                    break
 
            file_timespan = file_start - datetime.datetime.now()
            print dumpfile + "Processing done in " + str(file_timespan)
        print "ALL " + str(len(dumpfiles)) + " FILES DONE!!!"
    except Exception, e :
        sys.stdout.write("skipped (processing/parsing error)\n")
        print "Exception : %s" % e

    finally : 
        if (fin != None and not fin.closed) : fin.close()
        if (fout != None and not fout.closed) : fout.close()



def gen_outfiles(timeline, outfolder): 
    outfiles = []
    for dtime in timeline :
        outfilename = dtime.strftime("%Y%m%d_%H%M%S")+ ".xml"
        outfiles.append( os.path.join( outfolder, outfilename ) )
    return outfiles



def printhelp():
    print "Usage : wikitm.py --datetime=DATETIME --input-file=FILE [OPTIONS]" 
    print "-c --continue                   append to the output file."
    print "-C --datetime-count=COUNT       specify the number of datetime wanted in a timeline"
    print "-d --datetime=DATETIME          specify the iso8601 datetime of the output dump."
    print "-D --datetime-delta=DELTA       specify the delta between each datetime of a timeline"
    print "                                the date format is yyy-MM-ddTHH:mm:ssZ "
    print "-h --help                       display this help."
    print "-i --input-file=FILE            specify the input file"
    print "-I --input-folder=FOLDER        specify the dumps folder"
    print "-o --output-file=FILE           specify the output file"
    print "-O --output-folder=FOLDER       specify the output folder"
    print "-p --predefined-args            run the scripts using predefined args in the script"
    print "-v --verbose                    display more messages."




def with_predifined_args():
    startdate = datetime.datetime(2002, 01, 01) 
    delta = datetime.timedelta(days = 360)
    count = 12
    dumpfolder = "/media/datahd/dumps.wikimedia.org/enwiki/20170101-uncompressed/history12" 
    outfolder =  "/media/datahd/dumps.wikimedia.org/enwiki/snapshots12"
    #wiki_timelapse( startdate, delta, count, dumpfolder, outfolder)
    wiki_timelapse2( startdate, delta, count, dumpfolder, outfolder)
    sys.exit(0)
    

if __name__ == "__main__":
    continue_flag = False
    datetime_count = None 
    at_datetime = None
    datetime_delta = None
    depth = 1
    input_filename = None
    input_folder = None
    output_filename = None
    output_folder = None
    with_predifined = False
    verbose_flag = False

    try :
        opts, args = getopt.getopt(sys.argv[1:], 
                "cC:d:D:hi:I:o:O:pv", 
                ["continue", "datetime-count", "datetime=", "datetime-delta"
                    "help", "input-file=", "input-folder"
                    "output-file=", "output-folder", "predefined-args", "verbose"])
    except  getopt.GetoptError:
        print "Incorrect usage"
        printhelp()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-p", "--predefined-args") :
            with_predifined = True
            break
        elif opt in ("-c", "--continue") :
            continue_flag = True
        elif opt in ("-C", "-datetime-count") :
            datetime_count = arg
        elif opt in ("-d", "--datetime"):
            at_datetime = datetime.datetime.strptime(arg, ISO8601)
            # TODO warn when the format is not good
        elif opt in ("-D", "--datetime-delta") :
            datetime_delta = arg
        elif opt in ("-h", "--help"):
            printhelp()
            sys.exit()
        elif opt in ("-i", "--input-file"):
            input_filename = arg
        elif opt in ("-I", "--input-folder"):
            input_folder = arg
        elif opt in ("-o", "--output-file"):
            output_filename = arg
        elif opt in ("-O", "--output-folder"):
            output_folder = arg
        elif opt in ("-v", "--verbose"):
            verbose_flag = True

    if ( with_predifined ) :
        with_predifined_args()
        sys.exit(0)
    else :        
        if ( input_filename==None or at_datetime==None ):
            print "Please specify the input-file and the datetime"
            printhelp()
            sys.exit(2)
        if ( output_filename == None ):
             output_filename = "out-dump.xml"
        if ( ( datetime_delta != None and (at_datetime  == None or datetime_count == None) )  
                or ( datetime_count != None and (at_datetime == None or datetime_delta == None  )  ) ):
            print "To use a timeline please specify --datetime, --datetime-delta and --datetime-count"
            printhelp()

        wiki_timemachine(at_datetime, input_filename, output_filename, continue_flag)

