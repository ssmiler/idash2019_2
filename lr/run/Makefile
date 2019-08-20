CHALLENGE_FILE="../../data/sorted_tag_SNPs_1k_genotypes_test.txt"
#CHALLENGE_FILE="../../data/sorted_tag_SNPs_10k_genotypes_test.txt"
TARGET_HEADERS="../../data/sorted_target_SNP_genotypes.txt"
TARGET_FILE="../../data/sorted_target_SNP_genotypes.txt"
MODEL_FILE="../../ml/model/hr/10k_best"
#MODEL_FILE="../../ml/model/hr/10k"
#MODEL_FILE="../../ml/model/hr/1k"

# LINUX vs. OSX specifics
UNAME:=$(shell uname)

ifeq (${UNAME}, Linux)
TIME_CMD=/usr/bin/time -f '%e seconds -  %M KB RAM'
CMAKE_OPTIONS=-DENABLE_SPQLIOS_FMA=on -DENABLE_SPQLIOS_AVX=off -DENABLE_NAYUKI_PORTABLE=off -DENABLE_NAYUKI_AVX=off
endif
ifeq (${UNAME}, Darwin)
TIME_CMD=/usr/bin/time -l
CMAKE_OPTIONS=-DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DENABLE_SPQLIOS_FMA=on -DENABLE_SPQLIOS_AVX=off -DENABLE_NAYUKI_PORTABLE=on -DENABLE_NAYUKI_AVX=off
endif

all: bin/keygen params.bin keys.bin encrypted_data.bin encrypted_prediction.bin result.csv

clean:
	rm -rf params.bin keys.bin encrypted_data.bin encrypted_prediction.bin result.csv result_bypos.csv || true

distclean: clean
	rm -rf bin || true

build:
	[ -d bin ] || mkdir bin
	cd bin; cmake ../.. -DCMAKE_BUILD_TYPE=optim -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DENABLE_SPQLIOS_FMA=on -DENABLE_SPQLIOS_AVX=off -DENABLE_NAYUKI_PORTABLE=on -DENABLE_TESTS=off
	cd bin; make -j 4

bin/keygen:
	make build

keys.bin:
	@echo ===============================================================
	${TIME_CMD} ./bin/keygen ${TARGET_HEADERS} ${CHALLENGE_FILE}
	@echo ===============================================================

params.bin: keys.bin
	@[ -f $@ ] # keygen should already have created this file

encrypted_data.bin: keys.bin
	@echo ===============================================================
	${TIME_CMD} ./bin/encrypt ${CHALLENGE_FILE}
	@[ -f $@ ] # verify that the target is present
	@ls -lh $@
	@echo ===============================================================

encrypted_prediction.bin: params.bin encrypted_data.bin
	@echo ===============================================================
	${TIME_CMD} ./bin/cloud ${MODEL_FILE}
	@[ -f $@ ] # verify that the target is present
	@ls -lh $@
	@echo ===============================================================

result.csv: keys.bin encrypted_prediction.bin
	@echo ===============================================================
	${TIME_CMD} ./bin/decrypt
	@[ -f $@ ] # verify that the target is present
	@echo ===============================================================

result_bypos.csv: result.csv
	@[ -f $@ ] # verify that the target is present

