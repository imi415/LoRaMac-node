/*!
 * \file      adc-board.c
 *
 * \brief     Target board ADC driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "stm32f4xx.h"
#include "board-config.h"
#include "adc-board.h"

ADC_HandleTypeDef AdcHandle;

void AdcMcuInit(Adc_t *obj, PinNames adcInput) {
    AdcHandle.Instance = ADC1;
    __HAL_RCC_ADC1_CLK_ENABLE();
    HAL_ADC_DeInit(&AdcHandle);

    if(adcInput != NC) {
        GpioInit(&obj->AdcInput, adcInput, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
    }
}

void AdcMcuConfig(void) {
    AdcHandle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    AdcHandle.Init.Resolution = ADC_RESOLUTION_12B;
    AdcHandle.Init.ScanConvMode = DISABLE;
    AdcHandle.Init.ContinuousConvMode = DISABLE;
    AdcHandle.Init.DiscontinuousConvMode = DISABLE;
    AdcHandle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    AdcHandle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    AdcHandle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    AdcHandle.Init.NbrOfConversion = 1;
    AdcHandle.Init.DMAContinuousRequests = DISABLE;
    AdcHandle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;

    HAL_ADC_Init(&AdcHandle);
}

uint16_t AdcMcuReadChannel(Adc_t *obj, uint32_t channel) {
    ADC_ChannelConfTypeDef adcConf = {0};
    uint16_t adcData = 0;

    adcConf.Channel = channel;
    adcConf.Rank = 1;
    adcConf.SamplingTime = ADC_SAMPLETIME_3CYCLES;

    HAL_ADC_ConfigChannel(&AdcHandle, &adcConf);

    HAL_ADC_Start(&AdcHandle);

    HAL_ADC_PollForConversion(&AdcHandle, HAL_MAX_DELAY);

    adcData = HAL_ADC_GetValue(&AdcHandle);

    HAL_ADC_Stop(&AdcHandle);

    return adcData;
}