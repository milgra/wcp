#!/bin/bash

exe="$1/draw"

mkdir -p tst/result

$exe -r tst/configa -t tst/result/result0.bmp -f 300x190

diff tst/master tst/result

error=$?
if [ $error -eq 0 ]
then
    echo "No differences found between master and result images"
    exit 0
elif [ $error -eq 1 ]
then
    echo "Differences found between master and result images"
    exit 1
else
    echo "Differences found between master and result images"
    exit 1
fi
