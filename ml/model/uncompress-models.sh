#!/bin/sh

DIR=`dirname $0`
cd "$DIR"

if [ ! -d ../../ml/model ]; then
    echo please run uncompress from the ml/model directory
    exit 1
fi

tar -xvf 'neighbors=45_scale=16384_final.tar.xz'
ln -sf './neighbors=45_scale=16384_final' final
