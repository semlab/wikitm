for file in `ls enwiki-20170101-pages-meta-history1.xml-*.7z`
do
	echo "Uncompressing " $file
	p7zip -d $file >> log.txt
done
