ver        = s
CTESTOUT   = ctestout
COUNT      = 50
SOURCE     = knapPI_3_200_1000
SOURCEFILE = $(SOURCE).csv
TESTSET    = testset
TIMELIMIT  = 120      # minutes
THREADNUM  = 36
RESULTS    = results_$(SOURCE)_$(TIMELIMIT)min_$(COUNT)count_$(THREADNUM)thread
 
CC = g++
CFLAGS =

ifeq ($(ver), p)
    NAME     = BestFirstSearch_p
	CFLAGS   += -pthread
else
    NAME     = BestFirstSearch_s
endif

BINFILE   = $(NAME)

$(BINFILE):
	$(CC) $(NAME).cpp -o $@ $(CFLAGS)

# correct test
ctest: $(BINFILE)
	@rm -rf $(CTESTOUT)
	@./ctest.sh $(BINFILE) $(CTESTOUT)

# memory check
memcheck: $(BINFILE)
	valgrind ./$(BINFILE) inputMem.csv outputMem.txt

# generate input files and put them in $(TESTSET)
genIns:
	@rm -rf $(TESTSET)
	@mkdir $(TESTSET)
	@awk -vcount=$(COUNT) -vtestset=$(TESTSET) -f inputGenerate.awk $(SOURCEFILE)

# test on cluster, output to $(RESULTS)
testCluster:
	@rm -rf $(RESULTS)
	@mkdir $(RESULTS); \
		cd ./$(RESULTS); \
		mkdir out txt err; \
		cd ..
	@./runClusterJob.sh $(COUNT) $(TIMELIMIT) $(TESTSET) $(RESULTS) 

cmpResults:
	./compareResults.sh $(COUNT) $(RESULTS)


.PHONY: clean submit
clean:
	@rm -rf $(CTESTOUT)
#	@rm -rf $(RESULTS)
	@rm -f $(BINFILE)
	@rm -f *.txt
	@rm -f core*

submit:
	@cd ../; \
		scp -P 10190 -r knapsack pq@172.21.0.13:~/changyu/
