all: init roofline sequential best-sequential omp roofline-omp speedup

%:
	bash src/$@.sh | tee data/$@.txt

