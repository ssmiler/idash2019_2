
for tag in 1k 10k
do
  for num in 0 1
  do
    mkdir -p ../data/vw/$tag"_"$num
    python3 to_vw.py -o ../data/vw/$tag"_"$num -t ../data/snp_tag_$tag.pickle --numerical=$num
  done
done
