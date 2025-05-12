#include <dolphin.h>

// EXI error codes
typedef enum { AMC_EXI_NO_ERROR = 0, AMC_EXI_UNSELECTED } AmcExiError;

int AMC_IsStub();
void EXI2_Unreserve();
void EXI2_Reserve();
AmcExiError EXI2_WriteN();
AmcExiError EXI2_ReadN();
int EXI2_Poll();
void EXI2_EnableInterrupts();
void EXI2_Init();

int AMC_IsStub()
{
    return 1;
}

void EXI2_Unreserve() { }

void EXI2_Reserve() { }

AmcExiError EXI2_WriteN()
{
    return AMC_EXI_NO_ERROR;
}

AmcExiError EXI2_ReadN()
{
    return AMC_EXI_NO_ERROR;
}

int EXI2_Poll()
{
    return 0;
}

void EXI2_EnableInterrupts() { }
void EXI2_Init() { }
