for file in `ls enwiki-20170101-pages-meta-history1.xml*`
do 
	echo "Compressing " $file
	p7zip $file
done

