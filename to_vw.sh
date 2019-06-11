mkdir -p ../data/vw/1k
mkdir -p ../data/vw/1k_n
mkdir -p ../data/vw/10k
mkdir -p ../data/vw/10k_n

python3 to_vw.py -o ../data/vw/1k -t ../data/snp_tag_1k.pickle &
python3 to_vw.py -o ../data/vw/1k_n -t ../data/snp_tag_1k.pickle -n &
python3 to_vw.py -o ../data/vw/10k -t ../data/snp_tag_10k.pickle &
python3 to_vw.py -o ../data/vw/10k_n -t ../data/snp_tag_10k.pickle -n &

wait
