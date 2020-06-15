CONF=Makefile-final.inc
include ${CONF}

# LINUX vs. OSX specifics
UNAME:=$(shell uname)

ifeq (${UNAME}, Linux)
CMAKE_OPTIONS=-DENABLE_SPQLIOS_FMA=on -DENABLE_SPQLIOS_AVX=off -DENABLE_NAYUKI_PORTABLE=off -DENABLE_NAYUKI_AVX=off
endif
ifeq (${UNAME}, Darwin)
CMAKE_OPTIONS=-DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DENABLE_SPQLIOS_FMA=on -DENABLE_SPQLIOS_AVX=off -DENABLE_NAYUKI_PORTABLE=on -DENABLE_NAYUKI_AVX=off
endif

all: bin/keygen auc

clean:
	rm -rf params.bin keys.bin encrypted_data.bin encrypted_prediction.bin result_bypos.csv || true

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
	./bin/keygen ${TARGET_HEADERS} ${CHALLENGE_FILE} ${TARGET_HEADERS_POS_ONLY}
	@echo ===============================================================

params.bin: keys.bin
	@[ -f $@ ] # keygen should already have created this file

encrypted_data.bin: keys.bin
	@echo ===============================================================
	./bin/encrypt ${CHALLENGE_FILE}
	@[ -f $@ ] # verify that the target is present
	@ls -l $@
	@echo ===============================================================

encrypted_prediction.bin: params.bin encrypted_data.bin
	@echo ===============================================================
	./bin/cloud ${MODEL_FILE}
	@[ -f $@ ] # verify that the target is present
	@ls -l $@
	@echo ===============================================================

result_bypos.csv: keys.bin encrypted_prediction.bin
	@echo ===============================================================
	./bin/decrypt bypos
	@[ -f $@ ] # verify that the target is present
	@ls -l $@
	@echo ===============================================================

auc: result_bypos.csv
	python3 ../validate.py --pred_file result_bypos.csv --target_file ${TARGET_FILE}
