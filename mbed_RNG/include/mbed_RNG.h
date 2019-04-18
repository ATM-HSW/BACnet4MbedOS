#ifndef _MBED_RNG_H_
#define _MBED_RNG_H_

/*----------*/
/* Includes */
/*----------*/
#include "mbed.h"


/*------------------*/
/* Calssdeclaration */
/*------------------*/
class mbed_RNG
{
  private:
    static mbed_RNG *_instance;
  
	private:
		bool _ready;
	
		void _init(void);
		void _deInit(void);
  
    /**
     * @brief     Constructor - Creates an object instance
     */
    mbed_RNG();
	
	public:
		uint32_t rand(void);
	
    /**
     * @brief     Creates or returns a singleton object instance
     * @return    Pointer to singleton object instance
     */
    static mbed_RNG *Instance();
		~mbed_RNG();
};

#endif
