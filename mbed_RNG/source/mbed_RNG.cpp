/*----------*/
/* Includes */
/*----------*/
#include "mbed_RNG.h"

/*------------------*/
/* Static Variables */
/*------------------*/
mbed_RNG *mbed_RNG::_instance = (mbed_RNG *)NULL;

/*-----------------------------*/
/* Private - Methoddefinitions */
/*-----------------------------*/
void mbed_RNG::_init()
{
	/* Enable RNG clock source */
	__HAL_RCC_RNG_CLK_ENABLE();

	/* RNG Peripheral enable */
	RNG->CR |= RNG_CR_RNGEN;
}

void mbed_RNG::_deInit()
{
	/* Disable RNG peripheral */
	RNG->CR &= ~RNG_CR_RNGEN;

	/* Disable RNG clock source */
	__HAL_RCC_RNG_CLK_DISABLE();
}

/*----------------------------*/
/* Public - Methoddefinitions */
/*----------------------------*/
uint32_t mbed_RNG::rand()
{
	/* Wait until one RNG number is ready */
	while (!(RNG->SR & (RNG_SR_DRDY)))
	{
#if MBED_CONF_RTOS_PRESENT
#if MBED_VERSION >= MBED_ENCODE_VERSION(5, 10, 0)
		ThisThread::yield();
#else
		Thread::yield();
#endif
#endif /* MBED_CONF_RTOS_PRESENT */
	}

	/* Get a 32-bit Random number */
	return RNG->DR;
}

/*-------------------------*/
/* Constuctor & Destructor */
/*-------------------------*/
mbed_RNG *mbed_RNG::Instance()
{
	if (_instance == NULL)
	{
		mbed_RNG *tmp = new mbed_RNG();

		// Atomically set _instance
		core_util_critical_section_enter();

		if (_instance == NULL)
		{
			_instance = tmp;
		}

		core_util_critical_section_exit();

		// Another thread got there first so delete ours
		if (_instance != tmp)
		{
			delete tmp;
		}
	}

	return _instance;
}

mbed_RNG::mbed_RNG()
{
	/* Check if RNG already enabled */
	this->_ready = RNG->CR & RNG_CR_RNGEN;

	if (!this->_ready)
	{
		this->_init();
	}
}

mbed_RNG::~mbed_RNG()
{
	/* Check if RNG still enabled */
	this->_ready = RNG->CR & RNG_CR_RNGEN;
	if (this->_ready)
	{
		this->_deInit();
	}

	_instance = NULL;
}
