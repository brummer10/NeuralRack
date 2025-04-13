
#include <cmath>

namespace noisegate {

class Dsp {
private:
	uint32_t fSampleRate;
	inline float sqrf(float x);
	float ngate;
public:
	float threshold;
	void del_instance(Dsp *p);
	void clear_state_f();
	void init(uint32_t sample_rate);
	void compute(int count, float *input0);
	void computeLeft(int count, float *output0);
	void computeRight(int count, float *output0);
	Dsp();
	~Dsp();
};

Dsp::Dsp() {
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
}

inline void Dsp::init(uint32_t sample_rate)
{
	fSampleRate = sample_rate;
	threshold = 0.017f; // 0.017f, 0.01f, 0.31f, 0.001f
    ngate = 0.0;
	clear_state_f();
}

inline float Dsp::sqrf(float x)
{
    return x * x;
}

inline void Dsp::compute(int count, float *input0)
{
    float fSlow0 = sqrf(threshold * 0.1);    
    float sumnoise = 0;
    for (int i = 0; i < count; i++) {
        sumnoise += sqrf(input0[i]);
    }
    if (sumnoise/count > fSlow0) {
        ngate = 1.0; // -75db 0.001 = 65db
    } else if (ngate > 0.01) {
        ngate *= 0.996;
    }
}

inline void Dsp::computeLeft(int count, float *output0)
{
    for (int i = 0; i < count; i++) {
        output0[i] *= ngate;
    }
}

inline void Dsp::computeRight(int count, float *output0)
{
    for (int i = 0; i < count; i++) {
        output0[i] *= ngate;
    }    
}

Dsp *plugin() {
	return new Dsp();
}

void Dsp::del_instance(Dsp *p)
{
	delete p;
}
} // end namespace noisegate
