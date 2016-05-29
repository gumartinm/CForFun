#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <iomanip>
#include <bitset>
#include <stdint.h>

//using namespace std;

union Double_t
{
    Double_t(double num) : f(num) {}
    // Portable extraction of components.
    bool Negative() const { return (i >> 63) != 0; }
    int64_t RawMantissa() const { return i & ((1LL << 52) - 1); }
    int64_t RawExponent() const { return (i >> 52) & 0x7FF; }

    int64_t i;
    double f;
#ifdef _DEBUG
    struct
    {
        // Bitfields for exploration. Do not use in production code.
        // least significant bit (LSB): 2^0
        uint64_t mantissa : 52;  // mantissa also called significand
        uint64_t exponent : 11;
        uint64_t sign : 1;
        // most significant bit (MSB): 2^64
    } parts;
#endif
};


int main (int argc, char *argv[])
{
    std::cout << "******************************************************************************" << std::endl;
    std::cout << std::endl;
    std::cout << "(SIGN) MANTISSA * 2^(EXPONENT-1023)     (using base 10)" << std::endl;
    std::cout << std::endl;
    std::cout << "******************************************************************************" << std::endl;
    std::cout << std::endl;

    double number = atof(argv[1]);
    Double_t mydouble(number);

    std::cout << std::setprecision(64);
    std::cout << std::fixed;
    std::cout << "Number after atof conversion (libc): ";
    std::cout << number << std::endl;
    std::cout.unsetf(std::ios::fixed);  

    std::cout << std::endl;
    std::string signo = mydouble.Negative() ? "negative" : "positive";
    std::cout << "Sign: " + signo << std::endl;
    std::cout << std::endl;

    int64_t exponent = mydouble.RawExponent();
    std::bitset<11> exponent_bits(exponent);
    std::cout << "Exponent, binary value: " << exponent_bits << std::endl;
    std::cout << "Exponent, decimal value: " << exponent << std::endl;
    std::cout << "Exponent, real value: " << exponent << " - 1023 = " << (exponent - 1023) << std::endl;
    std::cout << std::endl;

    int64_t mantissa = mydouble.RawMantissa();
    std::bitset<52> mantissa_bits(mantissa);
    std::cout << "Mantissa, binary value: " << mantissa_bits << std::endl;
    std::cout << "Mantissa, hexadecimal value: ";
	std::cout << std::hex << std::uppercase << std::setw(13) << std::setfill('0') << mantissa << std::endl;
    std::cout << "Mantissa, binary value (fraction in base 2): 1." << mantissa_bits << std::endl;
    std::cout << "Mantissa, decimal value: TODO" << std::endl;
    std::cout << std::endl;
 

	std::cout << std::endl;
    std::cout << "Hexadecimal value (integer representation): ";
	std::cout << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << mydouble.i << std::endl;


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
