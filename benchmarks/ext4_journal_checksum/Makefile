all: create_append_sync set_journal_mode

create_append_sync: create_append_sync.c
	gcc create_append_sync.c -o create_append_sync

set_journal_mode: set_journal_mode.c
	gcc -c ../stream_common/common.c -o common.o
	gcc common.o set_journal_mode.c -o set_journal_mode

clean:
	rm -rf create_append_sync set_journal_mode
