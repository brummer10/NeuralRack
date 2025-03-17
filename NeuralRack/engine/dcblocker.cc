/*
 * dcblocker.cc
 *
 * SPDX-License-Identifier:  BSD-3-Clause
 *
 * Copyright (C) 2024 brummer <brummer@web.de>
 */

#include <cmath>

namespace dcblocker {

class Dsp {
private:
	uint32_t fSampleRate;
	double fConst1;
	double fVec0[2];
	double fConst2;
	double fRec0[2];

public:
	void connect(uint32_t port,void* data);
	void del_instance(Dsp *p);
	void clear_state_f();
	void init(uint32_t sample_rate);
	void compute(int count, float *input0, float *output0);
	Dsp();
	~Dsp();
};



Dsp::Dsp() {
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) fVec0[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0;
}

inline void Dsp::init(uint32_t sample_rate)
{
	fSampleRate = sample_rate;
	double fConst0 = 3.141592653589793 / std::min<double>(1.92e+05, std::max<double>(1.0, double(fSampleRate)));
	fConst1 = 1.0 - fConst0;
	fConst2 = 1.0 / (fConst0 + 1.0);
	clear_state_f();
}

void Dsp::compute(int count, float *input0, float *output0)
{
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = double(input0[i0]);
		fVec0[0] = fTemp0;
		fRec0[0] = fConst2 * (fTemp0 - fVec0[1] + fConst1 * fRec0[1]);
		output0[i0] = float(fRec0[0]);
		fVec0[1] = fVec0[0];
		fRec0[1] = fRec0[0];
	}
}

Dsp *plugin() {
	return new Dsp();
}

void Dsp::del_instance(Dsp *p)
{
	delete p;
}
} // end namespace dcblocker
