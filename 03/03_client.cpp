#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

int main() {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1

  // 连接服务器
  int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("connect");
  }

  // 写数据
  char msg[] = "hello";
  write(fd, msg, strlen(msg));

  // 读数据
  char rbuf[64] = {};
  ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    die("read");
  }
  printf("server says: %s\n", rbuf);
  close(fd);
  return 0;
}

/**
 * 问题：
 * 1）服务器和客户端的套接字只能一个写一个读？不能同时写或者同时读？
 * 不是的。套接字是全双工（full-duplex）的通信通道，允许数据的双向流动。这允许服务器和客户端能够同时发送和接收数据，而不必等待对方完成读取或写入操作。
 */
