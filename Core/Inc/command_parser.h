#ifndef INC_COMMAND_PARSER_H_
#define INC_COMMAND_PARSER_H_

#include <stdint.h>

// kích thước tối đa của bộ đệm vòng nhận dữ liệu uart
#define MAX_BUFFER_SIZE 30

// các loại lệnh hợp lệ mà hệ thống hỗ trợ
typedef enum {
    CMD_NONE = 0,
    CMD_RST,
    CMD_OK
} CommandType;

// biến tạm hứng từng byte từ ngắt uart2
extern uint8_t temp;
// mảng vòng lưu các byte vừa nhận được
extern uint8_t buffer[MAX_BUFFER_SIZE];
// vị trí ghi hiện tại trong buffer và cờ báo có dữ liệu mới (dùng volatile vì thay đổi trong ngắt)
extern volatile uint8_t index_buffer;
extern volatile uint8_t buffer_flag;

// cờ báo đã tách xong một lệnh hoàn chỉnh và mã lệnh tương ứng
extern uint8_t command_flag;
extern CommandType command_data;

// hàm chạy máy trạng thái để bóc tách lệnh !RST# và !OK#
void command_parser_fsm(void);

#endif /* INC_COMMAND_PARSER_H_ */
