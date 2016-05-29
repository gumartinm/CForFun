#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <stdint.h>

//using namespace std;

union Float_t
{
    Float_t(float num = 0.0f) : f(num) {}
    // Portable extraction of components.
    bool Negative() const { return (i >> 31) != 0; }
    int32_t RawMantissa() const { return i & ((1 << 23) - 1); }
    int32_t RawExponent() const { return (i >> 23) & 0xFF; }

    int32_t i;
    float f;
#ifdef _DEBUG
    struct
    {
        // Bitfields for exploration. Do not use in production code.
        // least significant bit (LSB): 2^0
        uint32_t mantissa : 23;  // mantissa also called significand
        uint32_t exponent : 8;
        uint32_t sign : 1;
        // most significant bit (MSB): 2^31
    } parts;
#endif
};


int main (int argc, char *argv[])
{
    std::cout << "******************************************************************************" << std::endl;
    std::cout << std::endl;
    std::cout << "(SIGN) MANTISSA * 2^(EXPONENT-127)     (using base 10)" << std::endl;
    std::cout << std::endl;
    std::cout << "******************************************************************************" << std::endl;
    std::cout << std::endl;

    float number = atof(argv[1]);
    Float_t myfloat(number);

	std::cout << "Number after atof conversion (libc): ";
    std::cout << std::setprecision(32) << std::fixed << number << std::endl;
    //std::cout.unsetf(std::ios::fixed);  

    std::cout << std::endl;
    std::string signo = myfloat.Negative() ? "negative" : "positive";
    std::cout << "Sign: " + signo << std::endl;
    std::cout << std::endl;

    int32_t exponent = myfloat.RawExponent();
	int32_t mantissa = myfloat.RawMantissa();

	// See: http://docs.oracle.com/cd/E19957-01/806-3568/ncgTOC.html
	// NORMAL NUMBER
 	if (exponent > 0 && exponent < 255) {
		std::cout << "NORMAL NUMBER" << std::endl;
	}

	// SUBNORMAL NUMBER
	if (exponent == 0 && mantissa != 0) {
		std::cout << "SUBNORMAL NUMBER" << std::endl;
	}

	// SIGNED ZERO
	if (exponent == 0 && mantissa == 0) {
        std::cout << "SIGNED ZERO" << std::endl;
    }

	// INFINITY
    if (exponent == 255 && mantissa == 0) {
		std::cout << "INFINITY, SEE SIGN" << std::endl;
	}	

	// NAN
	if (exponent == 255 && mantissa != 0) {
        std::cout << "NAN" << std::endl;
    }

    std::bitset<8> exponent_bits(exponent);
    std::cout << "Exponent, binary value: " << exponent_bits << std::endl;
    std::cout << "Exponent, decimal value: " << exponent << std::endl;
    std::cout << "Exponent, real value: " << exponent << " - 127 = " << (exponent - 127) << std::endl;
    std::cout << std::endl;

    std::bitset<23> mantissa_bits(mantissa);
    std::cout << "Mantissa, binary value: " << mantissa_bits << std::endl;
    std::cout << "Mantissa, hexadecimal value: ";
    std::cout << std::hex << std::uppercase << std::setw(6) << std::setfill('0') << mantissa << std::endl;
    std::cout << "Mantissa, binary value (fraction in base 2): 1." << mantissa_bits << std::endl;
    std::cout << "Mantissa, decimal value: TODO" << std::endl;
    std::cout << std::endl;
 

    std::cout << std::endl;
    std::cout << "Hexadecimal value (integer representation): ";
    std::cout << std::hex << std::setw(8) << std::setfill('0') << std::uppercase << myfloat.i << std::endl;

    return 0;
}




/***************************************************************************************************



          me@mycomputer:~$ ./floatconverter 0.01
          ******************************************************************************
          
          (SIGN) MANTISSA * 2^(EXPONENT-127)     (using base 10)
          
          ******************************************************************************
          
          Sign: positive
          
          Exponent, binary value: 01111000
          Exponent, decimal value: 120
          Exponent, real value: 120 - 127 = -7
          
          Mantissa, binary value: 01000111101011100001010
          Mantissa, binary value (fraction in base 2): 1.01000111101011100001010
          Mantissa, decimal value: TODO
          
          
          Hexadecimal value: 3C23D7A
          me@mycomputer:~$ echo "ibase=2; 1.01000111101011100001010" | bc
          1.27999997138977050781250
          me@mycomputer:~$ echo "obase=10; scale=10; 1.27999997138977050781250*(2^-7)" | bc
          .00999999977648258209228    <------------ closest integer (never 0.01)




******************************************************************************************************/
