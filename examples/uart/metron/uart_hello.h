#pragma once
#include "metron_tools.h"

//==============================================================================

class uart_hello {
 public:
  uart_hello() {
    readmemh("examples/uart/message.hex", memory, 0, 511);
  }

  //----------------------------------------

  logic<8> o_data() const { return data; }
  logic<1> o_req()  const { return s == SEND; }
  logic<1> o_done() const { return s == DONE; }

  void tock(logic<1> i_rstn, logic<1> i_cts, logic<1> i_idle) {
    tick(i_rstn, i_cts, i_idle);
  }

  //----------------------------------------

 private:
  void tick(logic<1> i_rstn, logic<1> i_cts, logic<1> i_idle) {
    if (!i_rstn) {
      s = WAIT;
      cursor = 0;
    } else {
      data = memory[cursor];
      if (s == WAIT && i_idle) {
        s = SEND;
      } else if (s == SEND && i_cts) {
        if (cursor == b9(message_len - 1)) {
          s = DONE;
        } else {
          cursor = cursor + 1;
        }
      } else if (s == DONE) {
        // s = WAIT;
        cursor = 0;
      }
    }
  }

  static const int message_len = 512;
  static const int cursor_bits = clog2(message_len);

  static const int WAIT = 0;
  static const int SEND = 1;
  static const int DONE = 2;

  //enum class state : logic<2>::BASE{WAIT, SEND, DONE};

  logic<2> s;
  logic<cursor_bits> cursor;
  logic<8> memory[512];
  logic<8> data;
};

//==============================================================================
