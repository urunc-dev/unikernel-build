/*
 * Minimal HTTP server: answers every request with a fixed plain-text reply.
 */
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define MESSAGE "Hello from urunc!\n"

static const char reply[] =
	"HTTP/1.1 200 OK\r\n"
	"Content-Type: text/plain\r\n"
	"Connection: close\r\n"
	"\r\n"
	MESSAGE;

int main(void)
{
	struct sockaddr_in addr = {
		.sin_family = AF_INET,
		.sin_port = htons(PORT),
		.sin_addr.s_addr = htonl(INADDR_ANY),
	};
	char buf[1024];
	int srv, conn, one = 1;

	srv = socket(AF_INET, SOCK_STREAM, 0);
	if (srv < 0) {
		perror("socket");
		return 1;
	}
	setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	if (bind(srv, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}
	if (listen(srv, 16) < 0) {
		perror("listen");
		return 1;
	}

	printf("Listening on port %d...\n", PORT);
	fflush(stdout);

	for (;;) {
		conn = accept(srv, NULL, NULL);
		if (conn < 0) {
			perror("accept");
			continue;
		}
		/* The request content does not matter, just drain it. */
		if (read(conn, buf, sizeof(buf)) < 0)
			perror("read");
		if (write(conn, reply, sizeof(reply) - 1) < 0)
			perror("write");
		close(conn);
	}

	return 0;
}
