#include "uart_communication.h"
#include <stdio.h>

// hai trạng thái giao tiếp: nghỉ chờ lệnh !RST# và đang chờ terminal xác nhận !OK#
typedef enum {
    COMM_IDLE = 0,
    COMM_WAIT_OK
} CommState;

static CommState comm_state = COMM_IDLE;
// lưu giá trị adc của lần đo gần nhất để gửi lại y nguyên nếu bị timeout 3 giây
static uint32_t adc_value = 0;
// mốc thời gian lần gửi gần nhất (tính theo ms của systick)
static uint32_t last_tick = 0;
static char tx_buf[32];

// hàm phụ định dạng chuỗi !ADC=giá_trị# rồi đẩy qua uart (thêm xuống dòng ở đầu khi là gói mới)
static void send_adc_packet(UART_HandleTypeDef *huart, uint32_t value, uint8_t leading_newline)
{
    int len = snprintf(tx_buf, sizeof(tx_buf),
                       leading_newline ? "\r\n!ADC=%lu#\r\n" : "!ADC=%lu#\r\n",
                       (unsigned long)value);
    if (len > 0)
    {
        HAL_UART_Transmit(huart, (uint8_t *)tx_buf, (uint16_t)len, 1000);
    }
}

// máy trạng thái xử lý yêu cầu !RST#, gửi giá trị adc và canh timeout 3 giây cho !OK#
void uart_communication_fsm(ADC_HandleTypeDef *hadc, UART_HandleTypeDef *huart)
{
    switch (comm_state)
    {
        case COMM_IDLE:
            // ở trạng thái nghỉ, chỉ phản hồi khi nhận được lệnh !RST#
            if (command_flag == 1)
            {
                command_flag = 0;
                if (command_data == CMD_RST)
                {
                    // kích hoạt chuyển đổi adc và đợi xong mới lấy giá trị mới nhất từ biến trở
                    HAL_ADC_Start(hadc);
                    HAL_ADC_PollForConversion(hadc, 10);
                    adc_value = HAL_ADC_GetValue(hadc);
                    send_adc_packet(huart, adc_value, 1);
                    // lưu mốc thời gian lúc gửi để bắt đầu đếm 3 giây chờ !OK#
                    last_tick = HAL_GetTick();
                    comm_state = COMM_WAIT_OK;
                }
            }
            break;

        case COMM_WAIT_OK:
            if (command_flag == 1)
            {
                command_flag = 0;
                // nhận được !OK# thì kết thúc phiên truyền, quay về trạng thái nghỉ
                if (command_data == CMD_OK)
                {
                    comm_state = COMM_IDLE;
                    break;
                }
                // nếu đang chờ mà người dùng gửi !RST# mới thì đo lại giá trị mới và gửi ngay
                else if (command_data == CMD_RST)
                {
                    HAL_ADC_Start(hadc);
                    HAL_ADC_PollForConversion(hadc, 10);
                    adc_value = HAL_ADC_GetValue(hadc);
                    send_adc_packet(huart, adc_value, 1);
                    last_tick = HAL_GetTick();
                    break;
                }
            }

            // quá 3 giây mà chưa thấy !OK# thì gửi lại đúng giá trị adc cũ (không đo lại)
            if (HAL_GetTick() - last_tick >= 3000)
            {
                send_adc_packet(huart, adc_value, 0);
                last_tick = HAL_GetTick();
            }
            break;

        default:
            comm_state = COMM_IDLE;
            break;
    }
}
