.PHONY: test clean

test: 
	@cd $(DIR)
	@killall server 2> /dev/null; \
	killall client 2> /dev/null; \
	rm -rf $(DIR)/tests/results
	@rm -rf $(DIR)/tests/to_check
	@mkdir $(DIR)/tests/results
	@mkdir $(DIR)/tests/to_check
	@mkdir -p $(DIR)/tests/server
	@mkdir -p $(DIR)/tests/client
	@mkdir -p $(DIR)/tests/to_check
	@rm -f $(DIR)/tests/Tests.log
	@rm -f $(DIR)/tests/Debug.txt
	@touch $(DIR)/tests/Tests.log
	@touch $(DIR)/tests/Debug.txt
	@echo ""
	@echo "In an [X,Y] answer of a test, where X and Y are 'OK' or 'FAIL',"
	@echo "  - X checks the expected printings of the client on its standard output"
	@echo "  - Y checks the expected content of directories at the end of the test"
	@echo "Test logs (i.e. detailed outputs) are recorded in tests/Tests.log"
	@echo ""
	@echo "Test both sides together"
	@for t in $$(ls $(DIR)/tests/test*); do \
		echo "===================================================" >> $(DIR)/tests/Tests.log; \
		echo "=================================== Test: $$(basename $$t)" >> $(DIR)/tests/Tests.log; \
		rm -rf $(DIR)/tests/server/$$(basename $$t)/; \
		mkdir -p $(DIR)/tests/server/$$(basename $$t)/; \
		rm -rf $(DIR)/tests/client/$$(basename $$t)/; \
		mkdir -p $(DIR)/tests/client/$$(basename $$t)/; \
		cp $(DIR)/reset/EDclient/* $(DIR)/tests/client/$$(basename $$t)/; \
		cp $(DIR)/reset/EDserver/* $(DIR)/tests/server/$$(basename $$t)/; \
		cd $(DIR)/bin/EDserver&& \
		(./server -directory $(DIR)/tests/server/$$(basename $$t)/ -quotasize 200000 -quotanumber 8 >> $(DIR)/tests/Tests.log 2>&1 & sleep 0.01) >> $(DIR)/tests/Debug.txt 2>&1; \
		cd $(DIR)/bin/EDclient&& \
		(./client 127.0.0.1 49152 -directory $(DIR)/tests/client/$$(basename $$t)/ -analyze $(DIR)/tests/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t) 2>>$(DIR)/tests/Tests.log) >> $(DIR)/tests/Debug.txt 2>&1; \
		ls $(DIR)/tests/client/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t)_dirs; \
		ls $(DIR)/tests/server/$$(basename $$t) >> $(DIR)/tests/results/$$(basename $$t)_dirs; \
		if diff -q $(DIR)/tests/golden/$$(basename $$t) $(DIR)/tests/results/$$(basename $$t) > /dev/null; then \
			if diff -q $(DIR)/tests/golden/$$(basename $$t)_dirs $(DIR)/tests/results/$$(basename $$t)_dirs > /dev/null; then \
				echo "[OK,OK] $$(basename $$t .in)"; \
			else \
				echo "[OK,FAILED] $$(basename $$t .in)"; \
			fi; \
		else \
			if diff -q $(DIR)/tests/golden/$$(basename $$t)_dirs $(DIR)/tests/results/$$(basename $$t)_dirs > /dev/null; then \
				echo "[FAILED,OK] $$(basename $$t .in)"; \
			else \
				echo "[FAILED,FAILED] $$(basename $$t .in)"; \
			fi; \
		fi; \
		killall server 2> /dev/null; \
		killall client 2> /dev/null; \
		cd $(DIR); \
	done
	@echo ""
	@echo "Test client side w.r.t provided server"
	@for t in $$(ls $(DIR)/tests/test*); do \
		echo "===================================================" >> $(DIR)/tests/Tests.log; \
		echo "=================================== Test: $$(basename $$t)" >> $(DIR)/tests/Tests.log; \
		rm -rf $(DIR)/tests/server/$$(basename $$t)/; \
		mkdir -p $(DIR)/tests/server/$$(basename $$t)/; \
		rm -rf $(DIR)/tests/client/$$(basename $$t)/; \
		mkdir -p $(DIR)/tests/client/$$(basename $$t)/; \
		cp $(DIR)/reset/EDclient/* $(DIR)/tests/client/$$(basename $$t)/; \
		cp $(DIR)/reset/EDserver/* $(DIR)/tests/server/$$(basename $$t)/; \
		cd $(DIR)/tools&& \
		(./server -directory $(DIR)/tests/server/$$(basename $$t)/ -p -quotasize 200000 -quotanumber 8 >> $(DIR)/tests/Tests.log 2>&1 & sleep 0.01) >> $(DIR)/tests/Debug.txt 2>&1; \
		cd $(DIR)/bin/EDclient&& \
		(./client 127.0.0.1 49152 -directory $(DIR)/tests/client/$$(basename $$t)/ -analyze $(DIR)/tests/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t)_cs 2>>$(DIR)/tests/Tests.log) >> $(DIR)/tests/Debug.txt 2>&1; \
		ls $(DIR)/tests/client/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t)_cs_dirs; \
		ls $(DIR)/tests/server/$$(basename $$t) >> $(DIR)/tests/results/$$(basename $$t)_cs_dirs; \
		if diff -q $(DIR)/tests/golden/$$(basename $$t)_cs $(DIR)/tests/results/$$(basename $$t)_cs > /dev/null; then \
			if diff -q $(DIR)/tests/golden/$$(basename $$t)_cs_dirs $(DIR)/tests/results/$$(basename $$t)_cs_dirs > /dev/null; then \
				echo "[OK,OK] $$(basename $${t}_cs .in)"; \
			else \
				echo "[OK,FAILED] $$(basename $${t}_cs .in)"; \
			fi; \
		else \
			if diff -q $(DIR)/tests/golden/$$(basename $$t)_cs_dirs $(DIR)/tests/results/$$(basename $$t)_cs_dirs > /dev/null; then \
				echo "[FAILED,OK] $$(basename $${t}_cs .in)"; \
			else \
				echo "[FAILED,FAILED] $$(basename $${t}_cs .in)"; \
			fi; \
		fi; \
		killall server 2> /dev/null; \
		killall client 2> /dev/null; \
		cd $(DIR); \
	done
	@echo ""
	@echo "Test server side w.r.t provided client"
	@for t in $$(ls $(DIR)/tests/test*); do \
		echo "===================================================" >> $(DIR)/tests/Tests.log; \
		echo "=================================== Test: $$(basename $$t)" >> $(DIR)/tests/Tests.log; \
		rm -rf $(DIR)/tests/server/$$(basename $$t)/; \
		mkdir -p $(DIR)/tests/server/$$(basename $$t)/; \
		rm -rf $(DIR)/tests/client/$$(basename $$t)/; \
		mkdir -p $(DIR)/tests/client/$$(basename $$t)/; \
		cp $(DIR)/reset/EDclient/* $(DIR)/tests/client/$$(basename $$t)/; \
		cp $(DIR)/reset/EDserver/* $(DIR)/tests/server/$$(basename $$t)/; \
		cd $(DIR)/bin/EDserver&& \
		(./server -directory $(DIR)/tests/server/$$(basename $$t)/ -quotasize 200000 -quotanumber 8 >> $(DIR)/tests/Tests.log 2>&1 & sleep 0.01) >> $(DIR)/tests/Debug.txt 2>&1; \
		cd $(DIR)/tools&& \
		(./client 127.0.0.1 49152 -p -directory $(DIR)/tests/client/$$(basename $$t)/ -analyze $(DIR)/tests/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t)_ss 2>>$(DIR)/tests/Tests.log) >> $(DIR)/tests/Debug.txt 2>&1; \
		ls $(DIR)/tests/client/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t)_ss_dirs; \
		ls $(DIR)/tests/server/$$(basename $$t) >> $(DIR)/tests/results/$$(basename $$t)_ss_dirs; \
		if diff -q $(DIR)/tests/golden/$$(basename $$t)_ss $(DIR)/tests/results/$$(basename $$t)_ss > /dev/null; then \
			if diff -q $(DIR)/tests/golden/$$(basename $$t)_ss_dirs $(DIR)/tests/results/$$(basename $$t)_ss_dirs > /dev/null; then \
				echo "[OK,OK] $$(basename $${t}_ss .in)"; \
			else \
				echo "[OK,FAILED] $$(basename $${t}_ss .in)"; \
			fi; \
		else \
			if diff -q $(DIR)/tests/golden/$$(basename $$t)_ss_dirs $(DIR)/tests/results/$$(basename $$t)_ss_dirs > /dev/null; then \
				echo "[FAILED,OK] $$(basename $${t}_ss .in)"; \
			else \
				echo "[FAILED,FAILED] $$(basename $${t}_ss .in)"; \
			fi; \
		fi; \
		killall server 2> /dev/null; \
		killall client 2> /dev/null; \
		cd $(DIR); \
	done
	@echo ""
	@echo "Test client w.r.t buggy server"
	@echo "Tests with '_CHCK' suffix require human check as result is underspecified"
	@echo "They can be found in directory to_check"
	@for seed in 5 8 14 16 25 36 42; do \
		for t in $$(ls $(DIR)/tests/seedsrv$${seed}_*); do \
			echo "===================================================" >> $(DIR)/tests/Tests.log; \
			echo "=================================== Test: $$(basename $$t)" >> $(DIR)/tests/Tests.log; \
			rm -rf $(DIR)/tests/server/$$(basename $$t)/; \
			mkdir -p $(DIR)/tests/server/$$(basename $$t)/; \
			rm -rf $(DIR)/tests/client/$$(basename $$t)/; \
			mkdir -p $(DIR)/tests/client/$$(basename $$t)/; \
			cp $(DIR)/reset/EDclient/* $(DIR)/tests/client/$$(basename $$t)/; \
			cp $(DIR)/reset/EDserver/* $(DIR)/tests/server/$$(basename $$t)/; \
			cd $(DIR)/tools&& \
			(./server -directory $(DIR)/tests/server/$$(basename $$t)/ -p -seed $$seed>> $(DIR)/tests/Tests.log 2>&1 & sleep 0.01) >> $(DIR)/tests/Debug.txt 2>&1; \
			cd $(DIR)/bin/EDclient&& \
			(./client 127.0.0.1 49152 -directory $(DIR)/tests/client/$$(basename $$t)/ -analyze $(DIR)/tests/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t) 2>>$(DIR)/tests/Tests.log) >> $(DIR)/tests/Debug.txt 2>&1; \
			ls $(DIR)/tests/client/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t)_dirs; \
			ls $(DIR)/tests/server/$$(basename $$t) >> $(DIR)/tests/results/$$(basename $$t)_dirs; \
			if diff -q $(DIR)/tests/golden/$$(basename $$t) $(DIR)/tests/results/$$(basename $$t) > /dev/null; then \
				if diff -q $(DIR)/tests/golden/$$(basename $$t)_dirs $(DIR)/tests/results/$$(basename $$t)_dirs > /dev/null; then \
					echo "[OK,OK] $$(basename $$t .in)"; \
				else \
					echo "[OK,FAILED] $$(basename $$t .in)"; \
				fi; \
			else \
				if diff -q $(DIR)/tests/golden/$$(basename $$t)_dirs $(DIR)/tests/results/$$(basename $$t)_dirs > /dev/null; then \
					echo "[FAILED,OK] $$(basename $$t .in)"; \
				else \
					echo "[FAILED,FAILED] $$(basename $$t .in)"; \
				fi; \
			fi; \
			killall server 2> /dev/null; \
			killall client 2> /dev/null; \
			cd $(DIR); \
		done; \
	done
	@for t in $$(ls $(DIR)/tests/results/*_CHCK); do \
		touch $(DIR)/tests/to_check/$$(basename $$t); \
		cat $$t >> $(DIR)/tests/to_check/$$(basename $$t); \
		cat check_reference_text.txt >> $(DIR)/tests/to_check/$$(basename $$t); \
		cat $(DIR)/tests/golden/$$(basename $$t) >> $(DIR)/tests/to_check/$$(basename $$t); \
	done
	@echo ""
	@echo "Test server w.r.t buggy client"
	@echo "Tests require human check as result is underspecified"
	@echo "See file buggyclient_tests in to_check (nothing written in Tests.log)"
	@touch $(DIR)/tests/to_check/buggyclient_tests
	@for seed in 2 3 8 25; do \
		for t in $$(ls $(DIR)/tests/seedcli$${seed}_*); do \
			cat $(DIR)/tests/txt_$$(basename $$t) >> $(DIR)/tests/to_check/buggyclient_tests; \
			rm -rf $(DIR)/tests/server/$$(basename $$t)/; \
			mkdir -p $(DIR)/tests/server/$$(basename $$t)/; \
			rm -rf $(DIR)/tests/client/$$(basename $$t)/; \
			mkdir -p $(DIR)/tests/client/$$(basename $$t)/; \
			cp $(DIR)/reset/EDclient/* $(DIR)/tests/client/$$(basename $$t)/; \
			cp $(DIR)/reset/EDserver/* $(DIR)/tests/server/$$(basename $$t)/; \
			cd $(DIR)/bin/EDserver&& \
			(./server -directory $(DIR)/tests/server/$$(basename $$t)/ >> $(DIR)/tests/to_check/buggyclient_tests 2>&1 & sleep 0.01) >> $(DIR)/tests/Debug.txt 2>&1; \
			cd $(DIR)/bin/EDclient&& \
			(./client 127.0.0.1 49152 -directory $(DIR)/tests/client/$$(basename $$t)/ -analyze $(DIR)/tests/$$(basename $$t) -seed $$seed > $(DIR)/tests/results/$$(basename $$t) 2>&1) >> $(DIR)/tests/Debug.txt 2>&1; \
			ls $(DIR)/tests/client/$$(basename $$t) > $(DIR)/tests/results/$$(basename $$t)_dirs; \
			ls $(DIR)/tests/server/$$(basename $$t) >> $(DIR)/tests/results/$$(basename $$t)_dirs; \
			if diff -q $(DIR)/tests/golden/$$(basename $$t) $(DIR)/tests/results/$$(basename $$t) > /dev/null; then \
				if diff -q $(DIR)/tests/golden/$$(basename $$t)_dirs $(DIR)/tests/results/$$(basename $$t)_dirs > /dev/null; then \
					echo "[OK,OK] $$(basename $$t .in)"; \
				else \
					echo "[OK,FAILED] $$(basename $$t .in)"; \
				fi; \
			else \
				if diff -q $(DIR)/tests/golden/$$(basename $$t)_dirs $(DIR)/tests/results/$$(basename $$t)_dirs > /dev/null; then \
					echo "[FAILED,OK] $$(basename $$t .in)"; \
				else \
					echo "[FAILED,FAILED] $$(basename $$t .in)"; \
				fi; \
			fi; \
			killall -s SIGINT server 2> /dev/null; \
			killall -s SIGINT client 2> /dev/null; \
			cd $(DIR); \
		done; \
	done
	@echo ""
	@echo "Test if a file with unauthorized name is ignored by ls"; \
	echo "Test both sides"; \
	echo "===================================================" >> $(DIR)/tests/Tests.log; \
	echo "=================================== Test: badname both sides" >> $(DIR)/tests/Tests.log; \
	rm -rf $(DIR)/tests/server/badname/; \
	mkdir -p $(DIR)/tests/server/badname/; \
	rm -rf $(DIR)/tests/client/badname/; \
	mkdir -p $(DIR)/tests/client/badname/; \
	cp $(DIR)/reset/EDclient/* $(DIR)/tests/client/badname/; \
	cp $(DIR)/reset/EDserver/* $(DIR)/tests/server/badname/; \
	touch $(DIR)/tests/server/badname/bad:name; \
	cd $(DIR)/bin/EDserver&& \
	(./server -directory $(DIR)/tests/server/badname/ >> $(DIR)/tests/Tests.log 2>&1 & sleep 0.01) >> $(DIR)/tests/Debug.txt 2>&1; \
	cd $(DIR)/bin/EDclient&& \
	(./client 127.0.0.1 49152 -directory $(DIR)/tests/client/badname/ -analyze $(DIR)/tests/badname > $(DIR)/tests/results/badname 2>>$(DIR)/tests/Tests.log) >> $(DIR)/tests/Debug.txt 2>&1; \
	ls $(DIR)/tests/client/badname > $(DIR)/tests/results/badname_dirs; \
	ls $(DIR)/tests/server/badname >> $(DIR)/tests/results/badname_dirs; \
	if diff -q $(DIR)/tests/golden/badname $(DIR)/tests/results/badname > /dev/null; then \
		if diff -q $(DIR)/tests/golden/badname_dirs $(DIR)/tests/results/badname_dirs > /dev/null; then \
			echo "[OK,OK] badname"; \
		else \
			echo "[OK,FAILED] badname"; \
		fi; \
	else \
		if diff -q $(DIR)/tests/golden/badname_dirs $(DIR)/tests/results/badname_dirs > /dev/null; then \
			echo "[FAILED,OK] badname"; \
		else \
			echo "[FAILED,FAILED] badname"; \
		fi; \
	fi; \
	killall server 2> /dev/null; \
	killall client 2> /dev/null; \
	echo "Test server side"; \
	echo "===================================================" >> $(DIR)/tests/Tests.log; \
	echo "=================================== Test: badname server side" >> $(DIR)/tests/Tests.log; \
	rm -rf $(DIR)/tests/server/badname/; \
	mkdir -p $(DIR)/tests/server/badname/; \
	rm -rf $(DIR)/tests/client/badname/; \
	mkdir -p $(DIR)/tests/client/badname/; \
	cp $(DIR)/reset/EDclient/* $(DIR)/tests/client/badname/; \
	cp $(DIR)/reset/EDserver/* $(DIR)/tests/server/badname/; \
	touch $(DIR)/tests/server/badname/bad:name; \
	cd $(DIR)/bin/EDserver&& \
	(./server -directory $(DIR)/tests/server/badname/ >> $(DIR)/tests/Tests.log 2>&1 & sleep 0.01) >> $(DIR)/tests/Debug.txt 2>&1; \
	cd $(DIR)/tools&& \
	(./client 127.0.0.1 49152 -directory $(DIR)/tests/client/badname/ -analyze $(DIR)/tests/badname > $(DIR)/tests/results/badname_ss 2>>$(DIR)/tests/Tests.log) >> $(DIR)/tests/Debug.txt 2>&1; \
	ls $(DIR)/tests/client/badname > $(DIR)/tests/results/badname_ss_dirs; \
	ls $(DIR)/tests/server/badname >> $(DIR)/tests/results/badname_ss_dirs; \
	if diff -q $(DIR)/tests/golden/badname_ss $(DIR)/tests/results/badname_ss > /dev/null; then \
		if diff -q $(DIR)/tests/golden/badname_ss_dirs $(DIR)/tests/results/badname_ss_dirs > /dev/null; then \
			echo "[OK,OK] badname_ss"; \
		else \
			echo "[OK,FAILED] badname_ss"; \
		fi; \
	else \
		if diff -q $(DIR)/tests/golden/badname_ss_dirs $(DIR)/tests/results/badname_ss_dirs > /dev/null; then \
			echo "[FAILED,OK] badname_ss"; \
		else \
			echo "[FAILED,FAILED] badname_ss"; \
		fi; \
	fi; \
	killall server 2> /dev/null; \
	killall client 2> /dev/null; \
	cd $(DIR)

clean:
	rm -rf results server client to_check Debug.txt Tests.log


