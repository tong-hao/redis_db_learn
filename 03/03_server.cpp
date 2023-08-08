#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> // Linux 操作系统中提供，sys/前缀通常用于指示系统级别的操作、系统调用和系统头文件。
#include <unistd.h>

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

// 用于在发生错误时终止程序的执行并打印相关错误信息
static void die(const char *msg) {
  int err = errno;
  fprintf(stderr, "[%d] %s\n", err, msg);
  abort();
}

static void do_something(int connfd) {
  char rbuf[64] = {};

  // 从套接字里读取数据
  ssize_t n = read(connfd, rbuf, sizeof(rbuf) - 1);
  if (n < 0) {
    msg("read() error");
    return;
  }
  printf("client says: %s\n", rbuf);

  // 往套接字里写数据
  char wbuf[] = "world";
  write(connfd, wbuf, strlen(wbuf));
}

int main() {
  /**
   * socket(): 用于创建一个套接字。
   * 返回值：这是一个整数变量，它将用于存储新创建的套接字的文件描述符。文件描述符是操作系统用于标识打开文件、套接字等资源的整数。
   *
   * 参数：
   * AF_INET 这是一个常量，表示使用IPv4地址族
   * SOCK_STREAM: TCP（Transmission Control Protocol）套接字
   * 0: 这是套接字类型的可选参数，通常为0。
   */
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    die("socket()");
  }

  /**
   * setsockopt 设置套接字选项
   * SOL_SOCKET - 表示要设置的选项位于套接字级别
   * SO_REUSEADDR - 设置的具体选项
   * SO_REUSEADDR - 允许在套接字关闭后，即使还有处于 TIME_WAIT
   * 状态的连接，也可以立即重用该地址
   * val - 将 SO_REUSEADDR 的选项值（通常为整数1）存储在变量 val 中。
   *
   */
  // this is needed for most server applications
  int val = 1;
  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

  // bind
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_port = ntohs(1234);
  addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0
  int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
  if (rv) {
    die("bind()");
  }

  // listen
  rv = listen(fd, SOMAXCONN);
  if (rv) {
    die("listen()");
  }

  while (true) {
    // accept
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);

    /**
     * 当服务器监听套接字接收到传入的连接请求时，accept()
     * 函数会阻塞程序的执行，直到有新的连接请求到达。一旦有连接请求到达，accept()
     * 将会：
     * 1）创建一个新的套接字，用于与客户端建立通信。这个套接字就是 connfd。
     * 2）填充 client_addr 结构，以便您可以了解连接的客户端的地址和端口等信息。
     *
     * connfd - 当成功接受传入的连接请求后，accept()
     * 将返回一个新的文件描述符，该描述符代表与客户端建立的连接
     */
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
    if (connfd < 0) {
      continue; // error
    }

    do_something(connfd);
    close(connfd);
  }

  return 0;
}

/**
 * 想法：
 * 1）为什么要创建新的套接字？
 * 为了隔离 每个client的连接。
 *
 * 2）套接字可以创建多少个？
 * 不确定
 *
 * 3）如何理解套接字？
 * 套接字相当于与其他服务器连接的通道
 */
