#ifndef INC_functions_H_
#define INC_functions_H_

#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

extern uint8_t rpm_line;

typedef struct PulseConfiguration
{
	uint16_t nTargetOverflowTMR3;
	uint16_t nTargetRemainTMR3;
	uint16_t nTargetOverflowTMR4;
	uint16_t nTargetRemainTMR4;
}PulseSettings;

extern PulseSettings EngineSpeedCurve[40];

#define blocksize sizeof (EngineSpeedCurve)

typedef union
{
	PulseSettings EngineSpeedCurve[40];
	uint8_t Array_EngineSpeedCurveAsBlock[blocksize];
}EngineSpeedBlock;

extern EngineSpeedBlock EngineSpeedUnion;

extern const EngineSpeedBlock Init_PulseSettings;

enum StateMachinePulse{Overflow,Remain};
extern enum StateMachinePulse ManagePulsePeriod, ManageDutyPulse;

//Pulse period
extern uint16_t nOverflowTMR3;
extern uint16_t nTargetOverflowTMR3;
extern uint16_t nTargetRemainTMR3;

//Pulse Duty cycle
extern uint16_t nOverflowTMR4;
extern uint16_t nTargetOverflowTMR4;
extern uint16_t nTargetRemainTMR4;

void InitializeEngineSpeedTable(void);
void SetPulseTMR3(void);
void ProgramTMR4(void);
void ResetPulseTMR4(void);
void GeneratePeriod(void);
void GenerateTooth(void);
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);

#endif /* INC_functions_H_ */
