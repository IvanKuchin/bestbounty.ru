#ifndef __C_FLOAT__H__
#define __C_FLOAT__H__

#include <string>
#include <cstdlib>
#include <math.h>		// --- pow() in RoundWithPrecision
#include <ios>			// --- getloc()

#include "clog.h"
#include "utilities.h"

using namespace std;

class c_float
{
	private:
		double		val;
		int			precision;

		auto		RoundWithPrecision(double num, int precision) -> double;
		auto		RoundWithPrecision(double num) -> double;
		auto 		FixRussianLocale(string param) -> string;

	public:
					c_float();
					c_float(string param);
					c_float(string param, int prec_param);
					c_float(double param);
					c_float(double param,  int prec_param);

		void		Set(double param) 						{ val = RoundWithPrecision(param, precision); };
		void		Set(string param);

		double		Get()	const 							{ return val; };
		double		GetPrecision()	const					{ return precision; };

		long		GetWhole();
		long		GetFraction();

					operator double() 						{ return Get(); };
					operator string();
		c_float		operator+(const c_float&);
		c_float		operator-(const c_float&);
		c_float		operator*(const c_float&);
		c_float		operator/(const c_float&);
		c_float		operator/(const long int&);
		// c_float&	operator=(const c_float &);
};

ostream&	operator<<(ostream& os, const c_float &);

#endif
