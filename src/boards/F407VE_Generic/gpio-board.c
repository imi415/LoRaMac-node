#include "stm32f4xx.h"
#include "utilities.h"
#include "board-config.h"
#include "rtc-board.h"
#include "gpio-board.h"
#if defined( BOARD_IOE_EXT )
#include "gpio-ioe.h"
#endif

static Gpio_t *GpioIrq[16];

void GpioMcuInit(Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value) {
    if(pin < IOE_0) {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        obj->pin = pin;

        if (pin == NC) {
            return;
        }

        obj->pinIndex = (0x01 << (obj->pin & 0x0F));

        if((obj->pin & 0xF0) == 0x00) {
            obj->port = GPIOA;
            __HAL_RCC_GPIOA_CLK_ENABLE();
        }
        else if((obj->pin & 0xF0) == 0x10) {
            obj->port = GPIOB;
            __HAL_RCC_GPIOB_CLK_ENABLE();
        }
        else if((obj->pin & 0xF0) == 0x20) {
            obj->port = GPIOC;
            __HAL_RCC_GPIOC_CLK_ENABLE();
        }
        else {
            obj->port = GPIOH;
            __HAL_RCC_GPIOH_CLK_ENABLE();
        }

        GPIO_InitStruct.Pin = obj->pinIndex;
        GPIO_InitStruct.Pull = obj->pull = type;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        if(mode == PIN_INPUT) {
            GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        }
        else if(mode == PIN_ANALOGIC) {
            GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        }
        else if(mode == PIN_ALTERNATE_FCT) {
            if(config == PIN_OPEN_DRAIN) {
                GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
            }
            else {
                GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
            }
            GPIO_InitStruct.Alternate = value;
        }
        else {
            if(config == PIN_OPEN_DRAIN) {
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
            }
            else {
                GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
            }
        }

        if(mode == PIN_OUTPUT) {
            GpioMcuWrite(obj, value);
        }

        HAL_GPIO_Init(obj->port, &GPIO_InitStruct);
    }
    else {
#if defined(BOARD_IOE_EXT)
        GpioIoeInit(obj, pin, mode, config, type, value);
#endif
    }
}

void GpioMcuSetContext(Gpio_t *obj, void* context) {
    obj->Context = context;
}

void GpioMcuSetInterrupt(Gpio_t *obj, IrqModes irqMode, IrqPriorities irqPriority, GpioIrqHandler *irqHandler) {
    if(obj->pin < IOE_0) {
        uint32_t priority = 0;

        IRQn_Type IRQnb = EXTI0_IRQn;
        GPIO_InitTypeDef GPIO_InitStruct;

        if(IrqHandler == NULL) {
            return;
        }

        obj->IrqHandler = IrqHandler;

        GPIO_InitStruct.Pin = obj->pinIndex;

        if(irqMode == IRQ_RISING_EDGE) {
            GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        }
        else if(irqMode == IRQ_FALLING_EDGE) {
            GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
        }
        else {
            GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
        }

        GPIO_InitStruct.Pull = obj->pull;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

        HAL_GPIO_Init(obj->port, &GPIO_InitStruct);

        switch(irqPriority) {
            case IRQ_VERY_LOW_PRIORITY:
            case IRQ_LOW_PRIORITY:
                priority = 3;
                break;
            case IRQ_MEDIUM_PRIORITY:
                priority = 2;
                break;
            case IRQ_HIGH_PRIORITY:
                priority = 1;
                break;
            case IRQ_VERY_HIGH_PRIORITY:
            default:
                priority = 0;
                break;
        }

        switch(obj->pinIndex) {
            case GPIO_PIN_0:
                IRQnb = EXTI0_IRQn;
                break;
            case GPIO_PIN_1:
                IRQnb = EXTI1_IRQn;
                break;
            case GPIO_PIN_2:
                IRQnb = EXTI2_IRQn;
                break;
            case GPIO_PIN_3:
                IRQnb = EXTI3_IRQn;
                break;
            case GPIO_PIN_4:
                IRQnb = EXTI4_IRQn;
                break;
            case GPIO_PIN_5:
            case GPIO_PIN_6:
            case GPIO_PIN_7:
            case GPIO_PIN_8:
            case GPIO_PIN_9:
                IRQnb = EXTI9_5_IRQn;
                break;
            case GPIO_PIN_10:
            case GPIO_PIN_11:
            case GPIO_PIN_12:
            case GPIO_PIN_13:
            case GPIO_PIN_14:
            case GPIO_PIN_15:
                IRQnb = EXTI15_10_IRQn;
                break;
            default:
                break;
        }

        GpioIeq[(obj->pin) & 0x0F] = obj;

        HAL_NVIC_SetPriority(IRQnb, priority, 0);
        HAL_NVIC_EnableIRQ(IRQnb);
    } else {
#if defined( BOARD_IOE_EXT )
        GpioIoeSetInterrupt( obj, irqMode, irqPriority, irqHandler );
#endif
    }
}