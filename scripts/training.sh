#!/bin/bash

DATA_DIR=./trained-data
BIN_DIR=/opt/word2vec/bin

for WIKI in `ls *.xml`
do
  echo [`date`] "Filtering '" $WIKI "'..."
  /opt/word2vec/scripts/wikifil.pl $WIKI > $WIKI.txt
done

for TEXT in `ls *.txt`
do
  echo [`date`] "Training '" $TEXT "'..."
  $BIN_DIR/word2vec -train $TEXT -output $TEXT.bin -cbow 0 -size 200 -window 5 -negative 0 -hs 1 -sample 1e-3 -threads 12 -binary 1
done  

echo [`date`] "Everthing done !"
