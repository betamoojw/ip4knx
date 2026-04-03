#ifdef ARDUINO_ARCH_ESP32
#include "TPUart/Interface/ESP32.h"

#include "driver/uart.h"

#ifndef TPUART_ESP32_TASK_STACK_SIZE
#define TPUART_ESP32_TASK_STACK_SIZE 2048
#endif

namespace TPUart
{
    namespace Interface
    {
        int _rx, _tx;
        uart_port_t _uart;
        bool _dma = false;
        volatile bool _overflow = false;
        QueueHandle_t _taskQueue = nullptr;
        TaskHandle_t _taskHandle = nullptr;
        std::function<void(void)> _callback;

        ESP32Interface::ESP32Interface(int rx, int tx, uart_port_t uart) : _rx(rx), _tx(tx), _uart(uart) {}
        ESP32Interface::~ESP32Interface() { end(); }

        void ESP32Interface::runTask(void *interface)
        {
            uart_event_t event;
            ESP32Interface *_interface = (ESP32Interface *)interface;
            while (true)
            {
                if (xQueueReceive(_interface->_taskQueue, (void *)&event, portMAX_DELAY))
                {
                    switch (event.type)
                    {
                        case UART_DATA:
                            if (_interface->_callback) _interface->_callback();
                            break;

                        case UART_FIFO_OVF:
                            _interface->_overflow = true;
                            break;

                        case UART_BUFFER_FULL:
                            _interface->_overflow = true;
                            break;

                        default:

                            break;
                    }
                }
                vTaskDelay(1);
            }
        }

        void ESP32Interface::begin(int baud)
        {
            if (_running) end();
            uart_config_t uart_config = {
                .baud_rate = baud,
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_EVEN,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                .source_clk = UART_SCLK_XTAL,
            };

            // UART-Konfiguration anwenden
            uart_param_config(_uart, &uart_config);
            uart_set_pin(_uart, _tx, _rx, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
            uart_driver_install(_uart, 512, 512, 32, &_taskQueue, 0);
            uart_set_rx_full_threshold(_uart, 1);
            _running = true;

            if (_uart == UART_NUM_1) xTaskCreate(&ESP32Interface::runTask, "uart_task1", TPUART_ESP32_TASK_STACK_SIZE, this, configMAX_PRIORITIES / 5 * 4, &_taskHandle);
          #if SOC_UART_HP_NUM > 2 // UART2 exists
            if (_uart == UART_NUM_2) xTaskCreate(&ESP32Interface::runTask, "uart_task2", TPUART_ESP32_TASK_STACK_SIZE, this, configMAX_PRIORITIES / 5 * 4, &_taskHandle);
          #endif
        }

        void ESP32Interface::end()
        {
            if (!_running) return;
            _running = false;

            uart_driver_delete(_uart);
            if (_taskHandle != nullptr)
            {
                vTaskDelete(_taskHandle);
                _taskHandle = nullptr;
            }
        }

        bool ESP32Interface::available()
        {
            if (!_running) return false;
            size_t len = 0;
            uart_get_buffered_data_len(_uart, &len);
            return len > 0;
        }

        bool ESP32Interface::availableForWrite()
        {
            if (!_running) return false;
            size_t len = 0;
            uart_get_tx_buffer_free_size(_uart, &len);
            return len > 0;
        }

        bool ESP32Interface::write(char value)
        {
            if (!_running) return false;
            // uart_wait_tx_done(_uart, pdMS_TO_TICKS(2));
            uart_write_bytes(_uart, &value, 1);
            return true;
        }

        int ESP32Interface::read()
        {
            if (!available()) return -1;
            char c;
            return uart_read_bytes(_uart, (uint8_t *)&c, 1, pdMS_TO_TICKS(1)) == 1 ? c : -1;
        }

        bool ESP32Interface::overflow()
        {
            if (_overflow)
            {
                _overflow = false;
                return true;
            }
            return false;
        };

        void ESP32Interface::flush()
        {
            if (!_running) return;

            uart_flush(_uart);
        }

        bool ESP32Interface::hasCallback()
        {
            return true;
        }

        void ESP32Interface::registerCallback(std::function<bool()> callback)
        {
            _callback = callback;
        }
    } // namespace Interface
} // namespace TPUart

#endif
