mkdir -p data/vw/1k
mkdir -p data/vw/10k

python3 to_vw.py -o data/vw/1k -t data/snp_tag_1k.pickle --target_file data/snp_target.pickle &
python3 to_vw.py -o data/vw/10k -t data/snp_tag_10k.pickle --target_file data/snp_target.pickle -d 200 &

wait


exit

mkdir -p data/vw_sep/1k
mkdir -p data/vw_sep/10k

python3 to_vw.py -o data/vw_sep/1k -t data/snp_tag_1k.pickle &
python3 to_vw.py -o data/vw_sep/10k -t data/snp_tag_10k.pickle  &

wait
