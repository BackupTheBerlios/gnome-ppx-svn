
BASENAME = `basename $(PWD)`
DEST_DIST = `dirname $(PWD)`/dist/$(BASENAME)

dist:
	$(MAKE) clean
	mkdir -p $(DEST_DIST)
	cp -a * $(DEST_DIST)
clean:
	rm -f *.pyc *.gladep *.bak
