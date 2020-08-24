all: create_append_sync create_append_sync_local

create_append_sync_local: create_append_sync_local.c
	gcc create_append_sync_local.c -o create_append_sync_local

create_append_sync: create_append_sync.c
	gcc create_append_sync.c -o create_append_sync

clean:
	rm -rf create_append_sync create_append_sync_local
