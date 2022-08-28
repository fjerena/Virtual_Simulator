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

uint8_t rpm_line=0;
EngineSpeedBlock EngineSpeedUnion;
PulseSettings EngineSpeedCurve[40];

uint16_t nOverflowTMR3=0u;
uint16_t nTargetOverflowTMR3=0u;
uint16_t nTargetRemainTMR3=0u;
enum StateMachinePulse ManagePulsePeriod=Overflow;

//Pulse Duty cycle
uint16_t nOverflowTMR4=0u;
uint16_t nTargetOverflowTMR4=0u;
uint16_t nTargetRemainTMR4=0u;
enum StateMachinePulse ManageDutyPulse=Overflow;

void InitializeEngineSpeedTable(void)
{
	uint16_t i;
	uint16_t f = sizeof(EngineSpeedCurve);

	for(i=0;i<f;i++)
	{
		EngineSpeedUnion.Array_EngineSpeedCurveAsBlock[i]=Init_PulseSettings.Array_EngineSpeedCurveAsBlock[i];
	}
}

void SetPulseTMR3(void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13|GPIO_PIN_14,GPIO_PIN_SET);
	__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,65535u);
	nOverflowTMR3=0;
}

void ChangeTableLine(uint8_t line)
{
	nTargetOverflowTMR3=EngineSpeedUnion.EngineSpeedCurve[line].nTargetOverflowTMR3;
	nTargetRemainTMR3=EngineSpeedUnion.EngineSpeedCurve[line].nTargetRemainTMR3;
	nTargetOverflowTMR4=EngineSpeedUnion.EngineSpeedCurve[line].nTargetOverflowTMR4;
	nTargetRemainTMR4=EngineSpeedUnion.EngineSpeedCurve[line].nTargetRemainTMR4;
}

void ProgramTMR4(void)
{
	HAL_TIM_OC_Start_IT(&htim4,TIM_CHANNEL_1);
	__HAL_TIM_SET_COUNTER(&htim4,0u);

	if(nOverflowTMR4==0)
	{
		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,nTargetRemainTMR4);
		ManageDutyPulse=Remain;
	}
	else
	{
		__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,65535u);
	}

	ManageDutyPulse=Overflow;
	nOverflowTMR4=0;
}

void ResetPulseTMR4(void)
{
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_13|GPIO_PIN_14,GPIO_PIN_RESET);
	HAL_TIM_OC_Stop_IT(&htim4,TIM_CHANNEL_1);
	nOverflowTMR4=0;
}

void GeneratePeriod(void)
{
	switch(ManagePulsePeriod)
	{
		case Overflow: 	if(nOverflowTMR3==nTargetOverflowTMR3)
						{
							__HAL_TIM_SET_COUNTER(&htim3,0u);

							if(nTargetRemainTMR3==0)
							{
								SetPulseTMR3();
								ChangeTableLine(rpm_line);
								ProgramTMR4();
							}
							else
							{
								ManagePulsePeriod=Remain;
								__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_1,nTargetRemainTMR3);
							}
						}

						nOverflowTMR3++;

						break;

		case Remain:	SetPulseTMR3();
						ChangeTableLine(rpm_line);
						ProgramTMR4();
						ManagePulsePeriod=Overflow;
						break;

		default: 		break;
	}
}

void GenerateTooth(void)
{
	switch(ManageDutyPulse)
	{
		case Overflow: 	if(nOverflowTMR4==nTargetOverflowTMR4)
						{
							__HAL_TIM_SET_COUNTER(&htim4,0u);

							if(nTargetRemainTMR4==0)
							{
								ResetPulseTMR4();
							}
							else
							{
								ManageDutyPulse=Remain;
								__HAL_TIM_SET_COMPARE(&htim4,TIM_CHANNEL_1,nTargetRemainTMR4);
							}
						}

						nOverflowTMR4++;

						break;

		case Remain:	ResetPulseTMR4();
						ManageDutyPulse=Overflow;
						break;

		default: 		break;
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
