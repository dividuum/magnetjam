SRCS=\
	 font.c \
	 font_stroke.c \
	 main.c \
	 misc.c \
	 game.c \
	 video.c \
	 audio.c

OBJECTS = $(SRCS:.c=.o)

LDFLAGS+= -lGL -lopenal -lalut -lm -lpthread -lXrandr -lrt -lglfw 
CC      = gcc -std=c99 -I.
TARGET  = magnet

all: $(TARGET)

ifdef DEBUG
CFLAGS += -Wextra -Wredundant-decls -Wpointer-arith
CCOPT  += -ggdb -Wall
Q       =
E       = @:
else
CFLAGS += -O3 -fexpensive-optimizations -fomit-frame-pointer -DNDEBUG
Q       = @
E       = @echo
endif

$(TARGET): $(OBJECTS)
	$(E) "LINK      $@"
	$(Q) $(CC) $^ -o $@ $(LDFLAGS) 

%.o: %.c
	$(E) "CC        $@"
	$(Q) $(CC) $(CFLAGS) -c -o $@ $<

MAKEDEPEND = 
%.P : %.c
	$(E) "DEP       $@"
	$(Q) $(CC) -M $(CFLAGS) -o $*.P $<
-include $(SRCS:.c=.P)

clean:
	-rm -f magnet *.o *.P 

.PHONY: clean
