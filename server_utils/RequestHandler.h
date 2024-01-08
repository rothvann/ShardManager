#pragma once

#include <mutex>

namespace server_utils {
enum class Operation {
  CONNECT = 0,
  READ = 1,
  WRITE = 2,
  FINISH = 3,
};

template <typename W, typename R>
class RequestHandler {
 public:
  virtual void handleConnect(bool ok){};
  virtual void handleRead(bool ok, bool& shouldAttemptNext){};
  virtual void handleWrite(bool ok, bool& shouldAttemptNext){};
  virtual void handleFinish(bool ok){};
  virtual void readFromStream() = 0;
  virtual void writeToStream(const W& msg) = 0;
  virtual void* getOpTag(Operation op) const = 0;

  void process(Operation op, bool ok) {
    switch (op) {
      case Operation::CONNECT: {
        handleConnect(ok);
        break;
      }
      case Operation::READ: {
        bool shouldAttemptNext = false;
        handleRead(ok, shouldAttemptNext);
        if(shouldAttemptNext) {
            attemptRead();
        }
        break;
      }
      case Operation::WRITE: {
        if (ok) {
          finishWrite(ok);
        }
        bool shouldAttemptNext = false;
        handleWrite(ok, shouldAttemptNext);
        if(shouldAttemptNext) {
            attemptWrite();
        }
        break;
      }
      case Operation::FINISH: {
        handleFinish(ok);
        break;
      }
    }
  }

 protected:
  R& getMessage() {
    return message_;
  }

  void write(W msg) {
    addToWriteQueue(msg);
    attemptWrite();
  }

  void attemptWrite() {
    std::lock_guard<std::mutex> lockGuard(writeLock_);
    std::cout << "Writing next message" << std::endl;
    if (msgQueue_.empty() || isWriting_) {
      std::cout << "Message queue empty" << std::endl;
      return;
    }
    isWriting_ = true;
    writeToStream(msgQueue_.front());
  }

  void attemptRead() {
    std::lock_guard<std::mutex> lockGuard(readLock_);
    if (isReading_) {
      std::cout << "Already reading" << std::endl;
      return;
    }
    isReading_ = true;
    readFromStream();
  }

private:
  void addToWriteQueue(W msg) {
    std::lock_guard<std::mutex> lockGuard(writeLock_);
    msgQueue_.push(std::move(msg));
  }
  
  void finishWrite(bool ok) {
    std::cout << "Write finished" << std::endl;
    std::lock_guard<std::mutex> lockGuard(writeLock_);
    if (ok && !msgQueue_.empty()) {
      msgQueue_.pop();
    }
    isWriting_ = false;
  }

  std::mutex readLock_;
  R message_;
  bool isReading_;

  std::mutex writeLock_;
  std::queue<W> msgQueue_;
  bool isWriting_;
};
}  // namespace server_utils