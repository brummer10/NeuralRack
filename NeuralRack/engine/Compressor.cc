// generated from file 'compressor.dsp' by dsp2cc:
// Code generated with Faust 2.79.3 (https://faust.grame.fr)

#include <cmath>

namespace compressor {

class Dsp {
private:
	uint32_t fSampleRate;
	float fConst0;
	float fConst1;
	float fConst2;
	float fRec1[2];
	float fConst3;
	float fRec0[2];

public:
	float threshold;
	float release;
	float attack;
	float ratio;
	void connect(uint32_t port,void* data);
	void del_instance(Dsp *p);
	void clear_state_f();
	void init(uint32_t sample_rate);
	void compute(int count, float *output0);
	Dsp();
	~Dsp();
};

Dsp::Dsp() {
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) fRec1[l0] = 0.0f;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec0[l1] = 0.0f;
}

inline void Dsp::init(uint32_t sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<float>(1.92e+05f, std::max<float>(1.0f, float(fSampleRate)));
	fConst1 = std::exp(-(1e+01f / fConst0));
	fConst2 = 1.0f - fConst1;
	fConst3 = 1.0f / fConst0;
	attack = 0.002f; // 0.0f, 1.0f, 0.001f 
	release = 0.5f; // 0.0f, 1e+01f, 0.01f 
	threshold = -2e+01f; // -96.0f, 1e+01f, 0.1f 
	ratio = 2.0f; // 1.0f, 2e+01f, 0.1f 
	clear_state_f();
}

void Dsp::compute(int count, float *output0)
{
	float fSlow0 = float(threshold);
	float fSlow1 = std::exp(-(fConst3 / std::max<float>(fConst3, release)));
	float fSlow2 = std::exp(-(fConst3 / std::max<float>(fConst3, attack)));
	float fSlow3 = 1.0f - float(ratio);
	float fSlow4 = 0.05f * fSlow3;
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		float fTemp0 = float(output0[i0]);
		fRec1[0] = fConst1 * fRec1[1] + fConst2 * std::fabs(fTemp0);
		float fTemp1 = fSlow2 * float(fRec0[1] < fRec1[0]) + fSlow1 * float(fRec0[1] >= fRec1[0]);
		fRec0[0] = fRec0[1] * fTemp1 + fRec1[0] * (1.0f - fTemp1);
		float fTemp2 = std::max<float>(0.0f, 2e+01f * std::log10(std::max<float>(1.1754944e-38f, fRec0[0])) - fSlow0);
		float fTemp3 = std::min<float>(1.0f, std::max<float>(0.0f, 1e+03f * fTemp2));
		output0[i0] = float(fTemp0 * std::pow(1e+01f, fSlow4 * (fTemp2 * fTemp3 / (1.0f - fSlow3 * fTemp3))));
		fRec1[1] = fRec1[0];
		fRec0[1] = fRec0[0];
	}
}


void Dsp::connect(uint32_t port,void* data)
{
}

Dsp *plugin() {
	return new Dsp();
}

void Dsp::del_instance(Dsp *p)
{
	delete p;
}
} // end namespace compressor
