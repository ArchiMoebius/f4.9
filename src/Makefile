#	NOTE:
#     ARGS:
#		O=
#			to add an object file to compilation use the arg O=nameoffile and 
#			place the nameoffile.o into the $(ODIR) dir
#		D=
#			to add an header file to compilation use the arg D=nameoffile and 
#			place the nameoffile.h into the $(IDIR)
#		target=
#			to compile something you must supply a target.
#			(with out the .c extension)
#	    For reference: http://www.gnu.org/software/make/manual/html_node/Text-Functions.html

BDIR =../bin
IDIR =../include
CC=gcc
CFLAGS=-I$(IDIR) -pthread -L/usr/local/lib

ODIR=obj
LDIR =../lib
DEBUG=0
LIBS=-ljpeg -lm

NAME = $(target)

ifeq ($(out),)
    BOUT = $(NAME)
else
    BOUT = $(out) $(out).c
endif

ifeq ($(BOUT),)
    ifneq ($(MAKECMDGOALS), clean)
        $(error Usage: make target=file D=whatever.h O=whatever.o out=binaryname)
    endif
else
    $(info )
    $(info Compiling output binary to $(BDIR)/$(BOUT))
    $(info )
endif

_DEPS = $(D)
DEPS = $(patsubst %, $(IDIR)/%.h, $(_DEPS))
#foreach item in $(_DEPS) prepend $(IDIR) and postpend .h


_OBJS = $(NAME) $(D) $(O)
OBJ = $(patsubst %,$(ODIR)/%.o,$(_OBJS))
#foreach item in $(_OBJS) prepend $(ODIR) and postpend .o


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -ggdb -c -o $@ $< $(CFLAGS) -DDEBUG=$(DEBUG)

all: $(OBJ)
	gcc -ggdb -o $(BDIR)/$(BOUT) $^ $(CFLAGS) $(LIBS) -DDEBUG=$(DEBUG)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ $(BDIR)/* *.ppm

