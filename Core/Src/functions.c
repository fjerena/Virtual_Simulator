#include "functions.h"
#include "main.h"

const EngineSpeedBlock Init_PulseSettings = {131,54784,10, 16640,
											  65,60160, 5,  8320,
		                                      43,61952, 3, 27392,
		                                      32,62848, 2, 36928,
		                                      26,24064, 2,  3328,
		                                      21,63744, 1, 46464,
		                                      18,54637, 1, 30464,
		                                      16,31424, 1, 18464,
		                                      14,42496, 1,  9130,
		                                      13,12032, 1,  1664,
		                                      11,64558, 0, 61090,
		                                      10,64640, 0, 56000,
		                                      10, 9255, 0, 51692,
		                                       9,27318, 0, 48000,
		                                       8,51712, 0, 44800,
		                                       8,15712, 0, 42000,
		                                       7,49483, 0, 39529,
		                                       7,21248, 0, 37333,
		                                       6,61520, 0, 35368,
		                                       6,38784, 0, 33600,
		                                       6,18212, 0, 32000,
		                                       5,65047, 0, 30545,
		                                       5,47972, 0, 29217,
		                                       5,32320, 0, 28000,
		                                       5,17920, 0, 26880,
		                                       5,4627,  0, 25846,
		                                       4,57856, 0, 24888,
		                                       4,46427, 0, 24000,
		                                       4,35787, 0, 23172,
		                                       4,25856, 0, 22400,
		                                       4,16565, 0, 21677,
		                                       4, 7856, 0, 21000,
		                                       3,65210, 0, 20363,
		                                       3,57509, 0, 19764,
		                                       3,50249, 0, 19200,
		                                       3,43392, 0, 18666,
		                                       3,36905, 0, 18162,
		                                       3,30760, 0, 17684,
		                                       3,24930, 0, 17230,
		                                       3,19392, 0, 16800};

uint8_t rpm_line=0u;
EngineSpeedBlock EngineSpeedUnion;
PulseSettings EngineSpeedCurve[40];

enum Pulse_Polarity Polarity=FallPulse;
enum Output OutputCtrl=same;

//Period
uint8_t program_TMR3=0b11;
uint8_t flg_time_is_over_TMR3=1u;
uint16_t nOverflowTMR3=0u;
uint16_t nTargetOverflowTMR3=0u;
uint16_t nTargetRemainTMR3=0u;

//Pulse Duty cycle
uint8_t program_TMR4=0b01;
uint16_t nOverflowTMR4=0u;
uint16_t nTargetOverflowTMR4=0u;
uint16_t nTargetRemainTMR4=0u;

void Set_Output(enum Pulse_Polarity Polarity, enum Output OutputCtrl)
{
	if(OutputCtrl==inverted)
	{
		(Polarity==FallPulse)?(Polarity=RisePulse):(Polarity=FallPulse);
	}

	if(Polarity==FallPulse)
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_SET);
	}
	else
	{
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_RESET);
	}
}

void InitializeEngineSpeedTable(void)
{
	uint16_t index;
	uint16_t number_of_bytes;

	number_of_bytes=sizeof(EngineSpeedCurve);

	for(index=0;index<number_of_bytes;index++)
	{
		EngineSpeedUnion.Array_EngineSpeedCurveAsBlock[index]=Init_PulseSettings.Array_EngineSpeedCurveAsBlock[index];
	}
}

void ChangeTableLine(uint8_t line)
{
	nTargetOverflowTMR3=EngineSpeedUnion.EngineSpeedCurve[line].nTargetOverflowTMR3;
	nTargetRemainTMR3=EngineSpeedUnion.EngineSpeedCurve[line].nTargetRemainTMR3;

	program_TMR3=0b00;

	(nTargetOverflowTMR3==0)?(program_TMR3=0b00):(program_TMR3=0b10);
	(nTargetRemainTMR3==0)?(program_TMR3=program_TMR3&0b10):(program_TMR3=program_TMR3|0b01);

	nTargetOverflowTMR4=EngineSpeedUnion.EngineSpeedCurve[line].nTargetOverflowTMR4;
	nTargetRemainTMR4=EngineSpeedUnion.EngineSpeedCurve[line].nTargetRemainTMR4;

	program_TMR4=0b00;

	(nTargetOverflowTMR4==0)?(program_TMR4=0b00):(program_TMR4=0b00000010);
	(nTargetRemainTMR4==0)?(program_TMR4=program_TMR4&0b10):(program_TMR4=program_TMR4|0b01);
}

void GeneratePeriod(void)
{
	switch(program_TMR3)
	{
		case 0b00:	// (nOverflowTMR3==0) && (nTargetRemainTMR3==0)    Do nothing...
					break;

		case 0b01:	// (nOverflowTMR3==0) && (nTargetRemainTMR3!=0)
					HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
					flg_time_is_over_TMR3=1u;
					program_TMR3=0b00;
					break;

		case 0b10:	// (nOverflowTMR3!=0) && (nTargetRemainTMR3==0)
					nOverflowTMR3++;

					if(nOverflowTMR3==nTargetOverflowTMR3)
					{
						HAL_GPIO_TogglePin(GPIOC,GPIO_PIN_13);
						flg_time_is_over_TMR3=1u;
						program_TMR3=0b00;
					}

					break;

		case 0b11:	// (nOverflowTMR3!=0) && (nTargetRemainTMR3!=0)
					nOverflowTMR3++;

					if(nOverflowTMR3==nTargetOverflowTMR3)
					{
						__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,nTargetRemainTMR3);
						program_TMR3=0b01;
					}

					break;

		default:    break;
	}
}

void GenerateTooth(void)
{
	switch(program_TMR4)
	{
		case 0b00:	// (nOverflowTMR4==0) && (nTargetRemainTMR4==0)    Do nothing...
					break;

		case 0b01:	// (nOverflowTMR4==0) && (nTargetRemainTMR4!=0)
					Set_Output(Polarity,same);

					HAL_TIM_OC_Stop_IT(&htim4,TIM_CHANNEL_1);
					program_TMR4=0b00;
					break;

		case 0b10:	// (nOverflowTMR4!=0) && (nTargetRemainTMR4==0)
					nOverflowTMR4++;

					if(nOverflowTMR4==nTargetOverflowTMR4)
					{
						Set_Output(Polarity,same);

						HAL_TIM_OC_Stop_IT(&htim4,TIM_CHANNEL_1);
						program_TMR4=0b00;
					}

					break;

		case 0b11:	// (nOverflowTMR4!=0) && (nTargetRemainTMR4!=0)
					nOverflowTMR4++;

					if(nOverflowTMR4==nTargetOverflowTMR4)
					{
						__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,nTargetRemainTMR4);
						program_TMR4=0b01;
					}

					break;

		default:    break;
	}
}

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if((htim->Instance == TIM3)&&
       (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1))
    {
        GeneratePeriod();
    }

    if((htim->Instance == TIM4)&&
       (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1))
    {
        GenerateTooth();
    }
}


























































