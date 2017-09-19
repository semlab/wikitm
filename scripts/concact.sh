for file in `find -name $1` 
do
	echo "adding $file..."
	cat $file >> history/enwiki-$1.xml
done
echo "Concatenation done !"
