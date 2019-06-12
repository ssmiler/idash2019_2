mkdir -p ../data/vw/1k
mkdir -p ../data/vw/10k

python3 to_vw.py -o ../data/vw/1k -t ../data/snp_tag_1k.pickle &
python3 to_vw.py -o ../data/vw/10k -t ../data/snp_tag_10k.pickle -d 200000 &

wait
