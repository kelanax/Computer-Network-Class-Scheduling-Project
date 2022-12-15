#
# This is the Makefile that can be used to create the "ee450_proj" executable
# To create "ee450_proj" executable, do:
#	make all
#
FILES_TO_BACKUP = client.cpp globals.h serverC.cpp serverCS.cpp serverM.cpp serverEE.cpp
BACKUP_FNAME = ee450_proj-A-backup-`date +%d%b%Y-%H%M%S`.tar.gz
BACKUP_DIR = $(HOME)/Shared-ubuntu-ee

all: serverM serverC serverEE serverCS client

serverM: serverM.o
	g++ -o serverM -g serverM.o

serverM.o: serverM.cpp globals.h
	g++ -g -c -Wall serverM.cpp

serverC: serverC.o
	g++ -o serverC -g serverC.o

serverC.o: serverC.cpp globals.h
	g++ -g -c -Wall serverC.cpp

serverEE: serverEE.o
	g++ -o serverEE -g serverEE.o

serverEE.o: serverEE.cpp globals.h
	g++ -g -c -Wall serverEE.cpp

serverCS: serverCS.o
	g++ -o serverCS -g serverCS.o

serverCS.o: serverCS.cpp globals.h
	g++ -g -c -Wall serverCS.cpp

client: client.o
	g++ -o client -g client.o

client.o: client.cpp globals.h
	g++ -g -c -Wall client.cpp

clean:
	rm -f *.o client serverC serverCS serverEE serverM 

backup:
	# if you want to backup different files, change FILES_TO_BACKUP at the top of this Makefile
	tar cvzf $(BACKUP_FNAME) $(FILES_TO_BACKUP)
	@if [ -d $(BACKUP_DIR)/ ]; then \
		mv $(BACKUP_FNAME) $(BACKUP_DIR)/$(BACKUP_FNAME); \
		echo ; \
		echo "Backup file created in shared folder: $(BACKUP_DIR)/$(BACKUP_FNAME)"; \
		/bin/ls -l $(BACKUP_DIR)/$(BACKUP_FNAME); \
	else \
		echo ; \
		echo "$(BACKUP_DIR) inaccessible, local backup file created: $(BACKUP_FNAME)"; \
		/bin/ls -l $(BACKUP_FNAME); \
	fi
