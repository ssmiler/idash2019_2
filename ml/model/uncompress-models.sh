#!/bin/sh

DIR=`dirname $0`
cd "$DIR"

if [ ! -d ../../ml/model ]; then
    echo please run uncompress from the ml/model directory
    exit 1
fi

tar -xvf '10k_neighbors=25_scale=16384_final.tar.xz'
tar -xvf '10k_neighbors=35_scale=16384_final.tar.xz'
tar -xvf '10k_neighbors=50_scale=16384_final.tar.xz'

ln -sf './10k_neighbors=25_scale=16384_final' 10k_a
ln -sf './10k_neighbors=35_scale=16384_final' 10k_b
ln -sf './10k_neighbors=50_scale=16384_final' 10k_c


tar -xvf '1k_neighbors=50_scale=16384_final.tar.xz'
tar -xvf '1k_neighbors=70_scale=16384_final.tar.xz'
tar -xvf '1k_neighbors=100_scale=16384_final.tar.xz'

ln -sf './1k_neighbors=50_scale=16384_final' 1k_a
ln -sf './1k_neighbors=70_scale=16384_final' 1k_b
ln -sf './1k_neighbors=100_scale=16384_final' 1k_c
