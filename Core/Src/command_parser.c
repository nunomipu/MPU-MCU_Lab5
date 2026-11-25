#include "command_parser.h"
#include <string.h>

uint8_t temp = 0;
uint8_t buffer[MAX_BUFFER_SIZE] = {0};
volatile uint8_t index_buffer = 0;
volatile uint8_t buffer_flag = 0;

uint8_t command_flag = 0;
CommandType command_data = CMD_NONE;

// hai trạng thái của bộ tách lệnh: chờ dấu '!' và đọc nội dung cho tới dấu '#'
typedef enum {
    WAIT_START = 0,
    READ_CMD
} ParserState;

static ParserState parser_state = WAIT_START;
// con trỏ đọc đuổi theo index_buffer để không bỏ sót byte nào khi dữ liệu tới liên tục
static uint8_t read_index = 0;
static char cmd_buf[MAX_BUFFER_SIZE];
static uint8_t cmd_idx = 0;

// hàm phân tích các ký tự trong buffer vòng để tìm đúng khung lệnh !RST# hoặc !OK#
void command_parser_fsm(void)
{
    // đọc hết các byte mới chưa được xử lý cho tới khi read_index đuổi kịp vị trí ghi
    while (read_index != index_buffer)
    {
        uint8_t ch = buffer[read_index++];
        // quay vòng về đầu mảng nếu đọc chạm cuối buffer
        if (read_index >= MAX_BUFFER_SIZE)
        {
            read_index = 0;
        }

        switch (parser_state)
        {
            case WAIT_START:
                // bỏ qua ký tự rác, chỉ bắt đầu ghi nhận khi gặp dấu '!'
                if (ch == '!')
                {
                    parser_state = READ_CMD;
                    cmd_idx = 0;
                }
                break;

            case READ_CMD:
                // nếu đang đọc dở mà gặp lại dấu '!' thì bỏ phần cũ, bắt đầu đọc lệnh mới luôn
                if (ch == '!')
                {
                    cmd_idx = 0;
                }
                // gặp dấu '#' là kết thúc khung, đóng chuỗi lại để so sánh lệnh
                else if (ch == '#')
                {
                    cmd_buf[cmd_idx] = '\0';
                    if (strcmp(cmd_buf, "RST") == 0)
                    {
                        command_flag = 1;
                        command_data = CMD_RST;
                    }
                    else if (strcmp(cmd_buf, "OK") == 0)
                    {
                        command_flag = 1;
                        command_data = CMD_OK;
                    }
                    // xử lý xong một khung thì quay về chờ dấu '!' tiếp theo
                    parser_state = WAIT_START;
                }
                else
                {
                    // lưu từng ký tự lệnh vào mảng tạm, nếu dài quá giới hạn thì hủy về trạng thái chờ
                    if (cmd_idx < MAX_BUFFER_SIZE - 1)
                    {
                        cmd_buf[cmd_idx++] = (char)ch;
                    }
                    else
                    {
                        parser_state = WAIT_START;
                        cmd_idx = 0;
                    }
                }
                break;

            default:
                parser_state = WAIT_START;
                break;
        }
    }
}
