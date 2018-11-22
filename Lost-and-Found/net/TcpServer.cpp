#include "TcpServer.h"
#include "TcpConnection.h"

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
	connections_.erase(conn->name());
}

void TcpServer::newConn(int fd, const NetAddress& peerAddr) {
	std::string connName = name_ + std::to_string(++nextConnid_);
	NetAddress localAddr(Socket::getLocalAddr(fd));
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(loop_, connName, fd, localAddr, peerAddr);
	connections_[connName] = conn;
	conn->setNewConnCallback(new_conn_callback_);
	conn->setMessageCallback(message_callback_);
	conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
	conn->connEstablished();
}
