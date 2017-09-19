export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
IN_DIR=/home/gr0259sh/Downloads/enwiki-20170101/history22
OUT_DIR=/home/gr0259sh/Downloads/enwiki-20170101/history22-snapshots
for file in `ls $IN_DIR/enwiki-20170101-pages-meta-history*.xml-*.7z`
do
	echo "Extracting  ".$file
	p7zip -d $file 
done
echo "Starting wikitm"
~/bin/wikitm/wikitm --input-folder=$IN_DIR --output-folder=$OUT_DIR --date-start=2002-01-01 --date-delta=600 --date-count=10 &
