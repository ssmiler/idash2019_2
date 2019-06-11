
parallel mkdir -p ../data/vw/{1}_{2} ::: 1k 10k ::: 0 1

parallel -j 4 python3 to_vw.py -o ../data/vw/{1}_{2} -t ../data/snp_tag_{1}.pickle --numerical={2} ::: 1k 10k ::: 0 1

