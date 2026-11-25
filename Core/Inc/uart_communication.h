#ifndef INC_UART_COMMUNICATION_H_
#define INC_UART_COMMUNICATION_H_

#include "main.h"
#include "command_parser.h"

// hàm máy trạng thái điều khiển gửi gói adc và truyền lại sau mỗi 3 giây nếu chưa thấy !OK#
void uart_communication_fsm(ADC_HandleTypeDef *hadc, UART_HandleTypeDef *huart);

#endif /* INC_UART_COMMUNICATION_H_ */
