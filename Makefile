build:
	gcc -o server rpc_server.c auth_svc.c auth_xdr.c utils.c -g -lnsl -Wall
	gcc -o client rpc_client.c auth_clnt.c auth_xdr.c utils.c -g -lnsl -Wall

run_client:
	./client localhost ./tests/tests/test3/client.in

run_server:
	./server ./tests/tests/test3/userIDs.db ./tests/tests/test3/resources.db ./tests/tests/test3/approvals.db 2

clean:
	rm client server
