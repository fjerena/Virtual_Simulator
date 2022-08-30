#ifndef INC_functions_H_
#define INC_functions_H_

#include "stm32f1xx_hal.h"

extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;

typedef struct PulseConfiguration
{
	uint16_t nTargetOverflowTMR3;
	uint16_t nTargetRemainTMR3;
	uint16_t nTargetOverflowTMR4;
	uint16_t nTargetRemainTMR4;
}PulseSettings;

extern PulseSettings EngineSpeedCurve[40];

#define blocksize sizeof(EngineSpeedCurve)

typedef union
{
	PulseSettings EngineSpeedCurve[40];
	uint8_t Array_EngineSpeedCurveAsBlock[blocksize];
}EngineSpeedBlock;

extern EngineSpeedBlock EngineSpeedUnion;
extern const EngineSpeedBlock Init_PulseSettings;

extern uint8_t rpm_line;

enum Pulse_Polarity{RisePulse,FallPulse};
extern enum Pulse_Polarity Polarity;

enum Output{same,inverted};
extern enum Output OutputCtrl;

//Pulse period
extern uint8_t program_TMR3;
extern uint8_t flg_time_is_over_TMR3;
extern uint16_t nOverflowTMR3;
extern uint16_t nTargetOverflowTMR3;
extern uint16_t nTargetRemainTMR3;

//Pulse Duty cycle
extern uint8_t program_TMR4;
extern uint16_t nOverflowTMR4;
extern uint16_t nTargetOverflowTMR4;
extern uint16_t nTargetRemainTMR4;

void Set_Output(enum Pulse_Polarity Polarity, enum Output OutputCtrl);
void InitializeEngineSpeedTable(void);
void ChangeTableLine(uint8_t line);
void GeneratePeriod(void);
void GenerateTooth(void);
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);

#endif /* INC_functions_H_ */
