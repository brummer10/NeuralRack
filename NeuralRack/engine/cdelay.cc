// generated from file 'cdeleay.dsp' by dsp2cc:
// Code generated with Faust 2.70.3 (https://faust.grame.fr)

#include <cmath>

namespace cdeleay {

class Dsp {
private:
	uint32_t fSampleRate;
	int IOTA0;
	double fVec0[16384];
	float *fHslider0_;
	double fRec4[2];
	double fRec0[2];
	double fRec1[2];
	double fRec2[2];
	double fRec3[2];


public:
	float delay;
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
	for (int l0 = 0; l0 < 16384; l0 = l0 + 1) fVec0[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec4[l1] = 0.0;
	for (int l2 = 0; l2 < 2; l2 = l2 + 1) fRec0[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fRec1[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec2[l4] = 0.0;
	for (int l5 = 0; l5 < 2; l5 = l5 + 1) fRec3[l5] = 0.0;
	IOTA0 = 0;
}

inline void Dsp::init(uint32_t sample_rate)
{
	fSampleRate = sample_rate;
	delay = 0.0;
	clear_state_f();
}

void Dsp::compute(int count, float *input0, float *output0)
{
	double fSlow0 = 0.0001000000000000009 * std::fabs(delay);
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = double(input0[i0]);
		fVec0[IOTA0 & 16383] = fTemp0;
		fRec4[0] = fSlow0 + 0.9999 * fRec4[1];
		double fTemp1 = ((fRec0[1] != 0.0) ? (((fRec1[1] > 0.0) & (fRec1[1] < 1.0)) ? fRec0[1] : 0.0) : (((fRec1[1] == 0.0) & (fRec4[0] != fRec2[1])) ? 0.0009765625 : (((fRec1[1] == 1.0) & (fRec4[0] != fRec3[1])) ? -0.0009765625 : 0.0)));
		fRec0[0] = fTemp1;
		fRec1[0] = std::max<double>(0.0, std::min<double>(1.0, fRec1[1] + fTemp1));
		fRec2[0] = (((fRec1[1] >= 1.0) & (fRec3[1] != fRec4[0])) ? fRec4[0] : fRec2[1]);
		fRec3[0] = (((fRec1[1] <= 0.0) & (fRec2[1] != fRec4[0])) ? fRec4[0] : fRec3[1]);
		double fTemp2 = fVec0[(IOTA0 - int(std::min<double>(8192.0, std::max<double>(0.0, fRec2[0])))) & 16383];
		output0[i0] = float(fTemp2 + fRec1[0] * (fVec0[(IOTA0 - int(std::min<double>(8192.0, std::max<double>(0.0, fRec3[0])))) & 16383] - fTemp2));
		IOTA0 = IOTA0 + 1;
		fRec4[1] = fRec4[0];
		fRec0[1] = fRec0[0];
		fRec1[1] = fRec1[0];
		fRec2[1] = fRec2[0];
		fRec3[1] = fRec3[0];
	}
}

void Dsp::connect(uint32_t port,void* data)
{
	switch (port)
	{
	case 8: 
		fHslider0_ = static_cast<float*>(data); // , 0.0, -4096.0, 4096.0, 12.0 
		break;
	default:
		break;
	}
}

Dsp *plugin() {
	return new Dsp();
}

void Dsp::del_instance(Dsp *p)
{
	delete p;
}
} // end namespace cdeleay
