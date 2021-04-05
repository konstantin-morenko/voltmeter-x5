
PROD_DIR = production

all: volt

$(PROD_DIR):
	mkdir $(PROD_DIR)

volt: $(PROD_DIR)
	cp voltmeter-x5.ino $(PROD_DIR)/voltmeter-x5-$(shell git describe).ino

clear:
	rm $(PROD_DIR)/*
	rmdir $(PROD_DIR)
