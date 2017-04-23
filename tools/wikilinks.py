#!/usr/bin/python 

import urllib2
import re 

def retrieve_links(lang, dumptype, filetype, day):
    url = "https://dumps.wikimedia.org/"+ lang +"wiki/"+ day
    print url
    response = urllib2.urlopen(url)
    html = response.read()
    #print html[:100]
    p_link_s = lang +"wiki-"+ day +"-"+ dumptype +"[0-9]*\.xml-[a-zA-Z0-9]*\." + filetype
    print p_link_s
    p_link = re.compile(p_link_s)
    links = re.findall(p_link, html)
    print "nb links=" + str(len(links))
    links = list(set(links))
    links.sort(key=str.lower)
    print "nb links=" + str(len(links))
    fout = open(lang + "wiki-" + day + "-" + dumptype + "-links.txt", 'w') 
    try :
        for link in links:
            link = url + "/" + link
            fout.write(link + "\n")
    finally :
        fout.close()




if __name__ == "__main__":
    retrieve_links("en", "pages-meta-history", "7z", "20170101") 
