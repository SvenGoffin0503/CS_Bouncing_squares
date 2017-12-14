# Makefile

CFLAGS = `sdl-config --cflags --libs` --std=c99 --pedantic -Wall -Wmissing-prototypes -g

OBJS = parallel.o synchronization.o output.o main.o #Objects to build
TARGET = main       #Target executable name

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) `sdl-config --cflags --libs`


clean:
	rm -f $(OBJS) *.d core* *.c~ *.h~ *~ $(TARGET)


# Automatic maintenance of dependencies:

OBJMAKETRANSCMD = sed "s/$*.o:/$*.o $*.d:/g"

%.d: %.c
	$(SHELL) -ec '$(CC) -c -MM $(CFLAGS) $< | $(OBJMAKETRANSCMD) > $@'

include $(addsuffix .d,$(basename $(OBJS)))
