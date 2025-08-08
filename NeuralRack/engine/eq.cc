// generated from file 'eq.dsp' by dsp2cc:
// Code generated with Faust 2.75.7 (https://faust.grame.fr)

#include <cmath>

template<class T> inline T mydsp_faustpower2_f(T x) {return (x * x);}

namespace eq {

class Dsp {
private:
	uint32_t fSampleRate;
	double fConst0;
	double fConst1;
	double fConst2;
	double fConst3;
	double fConst4;
	double fConst5;
	double fConst6;
	double fConst7;
	double fConst8;
	double fConst9;
	double fConst10;
	double fConst11;
	double fConst12;
	double fConst13;
	double fConst14;
	double fConst15;
	double fConst16;
	double fConst17;
	double fConst18;
	double fConst19;
	double fConst20;
	double fConst21;
	double fConst22;
	double fConst23;
	double fConst24;
	double fConst25;
	double fConst26;
	double fConst27;
	double fConst28;
	double fConst29;
	double fConst30;
	double fConst31;
	double fConst32;
	double fConst33;
	double fConst34;
	double fVec0[2];
	double fConst35;
	double fConst36;
	double fRec9[2];
	double fRec8[3];
	double fVec1[2];
	double fConst37;
	double fConst38;
	double fConst39;
	double fRec7[2];
	double fRec6[3];
	double fVec2[2];
	double fConst40;
	double fConst41;
	double fConst42;
	double fRec5[2];
	double fRec4[3];
	double fVec3[2];
	double fConst43;
	double fConst44;
	double fConst45;
	double fRec3[2];
	double fRec2[3];
	double fVec4[2];
	double fConst46;
	double fConst47;
	double fConst48;
	double fConst49;
	double fRec1[2];
	double fRec0[3];
	double fRec10[2];
	double fRec12[2];
	double fRec11[3];
	double fRec13[2];
	double fConst50;
	double fConst51;
	double fConst52;
	double fRec16[2];
	double fRec15[3];
	double fConst53;
	double fRec14[3];
	double fRec17[2];
	double fConst54;
	double fConst55;
	double fConst56;
	double fRec21[2];
	double fRec20[3];
	double fConst57;
	double fRec19[3];
	double fRec18[3];
	double fRec22[2];
	double fConst58;
	double fConst59;
	double fConst60;
	double fRec27[2];
	double fRec26[3];
	double fConst61;
	double fRec25[3];
	double fRec24[3];
	double fRec23[3];
	double fRec28[2];
	double fConst62;
	double fConst63;
	double fRec34[2];
	double fRec33[3];
	double fConst64;
	double fRec32[3];
	double fRec31[3];
	double fRec30[3];
	double fRec29[3];
	double fRec35[2];
#ifdef DEBUG
	inline void check_state_f(double v);
#endif
public:
	float  fVslider0;
	float  fVslider1;
	float  fVslider2;
	float  fVslider3;
	float  fVslider4;
	float  fVslider5;
	void del_instance(Dsp *p);
	inline void clear_state_f();
	inline void clear_state_internal();
	inline void init(uint32_t sample_rate);
	inline void compute(int count, float *input0, float *output0);
	Dsp();
	~Dsp();
};



Dsp::Dsp() {
	fVslider0 = 0.0;
	fVslider1 = -20.0;
	fVslider2 = 0.0;
	fVslider3 = 0.0;
	fVslider4 = 0.0;
	fVslider5 = -20.0;
}

Dsp::~Dsp() {
}

inline void Dsp::clear_state_f()
{
	for (int l0 = 0; l0 < 2; l0 = l0 + 1) fVec0[l0] = 0.0;
	for (int l1 = 0; l1 < 2; l1 = l1 + 1) fRec9[l1] = 0.0;
	for (int l2 = 0; l2 < 3; l2 = l2 + 1) fRec8[l2] = 0.0;
	for (int l3 = 0; l3 < 2; l3 = l3 + 1) fVec1[l3] = 0.0;
	for (int l4 = 0; l4 < 2; l4 = l4 + 1) fRec7[l4] = 0.0;
	for (int l5 = 0; l5 < 3; l5 = l5 + 1) fRec6[l5] = 0.0;
	for (int l6 = 0; l6 < 2; l6 = l6 + 1) fVec2[l6] = 0.0;
	for (int l7 = 0; l7 < 2; l7 = l7 + 1) fRec5[l7] = 0.0;
	for (int l8 = 0; l8 < 3; l8 = l8 + 1) fRec4[l8] = 0.0;
	for (int l9 = 0; l9 < 2; l9 = l9 + 1) fVec3[l9] = 0.0;
	for (int l10 = 0; l10 < 2; l10 = l10 + 1) fRec3[l10] = 0.0;
	for (int l11 = 0; l11 < 3; l11 = l11 + 1) fRec2[l11] = 0.0;
	for (int l12 = 0; l12 < 2; l12 = l12 + 1) fVec4[l12] = 0.0;
	for (int l13 = 0; l13 < 2; l13 = l13 + 1) fRec1[l13] = 0.0;
	for (int l14 = 0; l14 < 3; l14 = l14 + 1) fRec0[l14] = 0.0;
	for (int l16 = 0; l16 < 2; l16 = l16 + 1) fRec12[l16] = 0.0;
	for (int l17 = 0; l17 < 3; l17 = l17 + 1) fRec11[l17] = 0.0;
	for (int l19 = 0; l19 < 2; l19 = l19 + 1) fRec16[l19] = 0.0;
	for (int l20 = 0; l20 < 3; l20 = l20 + 1) fRec15[l20] = 0.0;
	for (int l21 = 0; l21 < 3; l21 = l21 + 1) fRec14[l21] = 0.0;
	for (int l23 = 0; l23 < 2; l23 = l23 + 1) fRec21[l23] = 0.0;
	for (int l24 = 0; l24 < 3; l24 = l24 + 1) fRec20[l24] = 0.0;
	for (int l25 = 0; l25 < 3; l25 = l25 + 1) fRec19[l25] = 0.0;
	for (int l26 = 0; l26 < 3; l26 = l26 + 1) fRec18[l26] = 0.0;
	for (int l28 = 0; l28 < 2; l28 = l28 + 1) fRec27[l28] = 0.0;
	for (int l29 = 0; l29 < 3; l29 = l29 + 1) fRec26[l29] = 0.0;
	for (int l30 = 0; l30 < 3; l30 = l30 + 1) fRec25[l30] = 0.0;
	for (int l31 = 0; l31 < 3; l31 = l31 + 1) fRec24[l31] = 0.0;
	for (int l32 = 0; l32 < 3; l32 = l32 + 1) fRec23[l32] = 0.0;
	for (int l34 = 0; l34 < 2; l34 = l34 + 1) fRec34[l34] = 0.0;
	for (int l35 = 0; l35 < 3; l35 = l35 + 1) fRec33[l35] = 0.0;
	for (int l36 = 0; l36 < 3; l36 = l36 + 1) fRec32[l36] = 0.0;
	for (int l37 = 0; l37 < 3; l37 = l37 + 1) fRec31[l37] = 0.0;
	for (int l38 = 0; l38 < 3; l38 = l38 + 1) fRec30[l38] = 0.0;
	for (int l39 = 0; l39 < 3; l39 = l39 + 1) fRec29[l39] = 0.0;
}

inline void Dsp::clear_state_internal()
{
	for (int l15 = 0; l15 < 2; l15 = l15 + 1) fRec10[l15] = 0.0;
	for (int l22 = 0; l22 < 2; l22 = l22 + 1) fRec17[l22] = 0.0;
	for (int l18 = 0; l18 < 2; l18 = l18 + 1) fRec13[l18] = 0.0;
	for (int l27 = 0; l27 < 2; l27 = l27 + 1) fRec22[l27] = 0.0;
	for (int l33 = 0; l33 < 2; l33 = l33 + 1) fRec28[l33] = 0.0;
	for (int l40 = 0; l40 < 2; l40 = l40 + 1) fRec35[l40] = 0.0;
}

#ifdef DEBUG
inline void Dsp::check_state_f(double v)
{
	if (std::fpclassify(v) == FP_SUBNORMAL) {
		fprintf(stderr, "subnormal detected\n");
	}
}
#endif

inline void Dsp::init(uint32_t sample_rate)
{
	fSampleRate = sample_rate;
	fConst0 = std::min<double>(1.92e+05, std::max<double>(1.0, double(fSampleRate)));
	fConst1 = std::tan(138.23007675795088 / fConst0);
	fConst2 = 1.0 / mydsp_faustpower2_f(fConst1);
	fConst3 = 2.0 * (1.0 - fConst2);
	fConst4 = 1.0 / fConst1;
	fConst5 = (fConst4 + -1.0000000000000004) / fConst1 + 1.0;
	fConst6 = 1.0 / ((fConst4 + 1.0000000000000004) / fConst1 + 1.0);
	fConst7 = std::tan(556.0618996853934 / fConst0);
	fConst8 = mydsp_faustpower2_f(fConst7);
	fConst9 = 2.0 * (1.0 - 1.0 / fConst8);
	fConst10 = 1.0 / fConst7;
	fConst11 = (fConst10 + -1.0000000000000004) / fConst7 + 1.0;
	fConst12 = (fConst10 + 1.0000000000000004) / fConst7 + 1.0;
	fConst13 = 1.0 / fConst12;
	fConst14 = std::tan(2221.1060060879836 / fConst0);
	fConst15 = mydsp_faustpower2_f(fConst14);
	fConst16 = 2.0 * (1.0 - 1.0 / fConst15);
	fConst17 = 1.0 / fConst14;
	fConst18 = (fConst17 + -1.0000000000000004) / fConst14 + 1.0;
	fConst19 = (fConst17 + 1.0000000000000004) / fConst14 + 1.0;
	fConst20 = 1.0 / fConst19;
	fConst21 = std::tan(8884.424024351934 / fConst0);
	fConst22 = mydsp_faustpower2_f(fConst21);
	fConst23 = 2.0 * (1.0 - 1.0 / fConst22);
	fConst24 = 1.0 / fConst21;
	fConst25 = (fConst24 + -1.0000000000000004) / fConst21 + 1.0;
	fConst26 = (fConst24 + 1.0000000000000004) / fConst21 + 1.0;
	fConst27 = 1.0 / fConst26;
	fConst28 = std::tan(35763.890768466204 / fConst0);
	fConst29 = mydsp_faustpower2_f(fConst28);
	fConst30 = 2.0 * (1.0 - 1.0 / fConst29);
	fConst31 = 1.0 / fConst28;
	fConst32 = (fConst31 + -1.0000000000000004) / fConst28 + 1.0;
	fConst33 = (fConst31 + 1.0000000000000004) / fConst28 + 1.0;
	fConst34 = 1.0 / fConst33;
	fConst35 = 1.0 - fConst31;
	fConst36 = 1.0 / (fConst31 + 1.0);
	fConst37 = 1.0 - fConst24;
	fConst38 = fConst24 + 1.0;
	fConst39 = 1.0 / fConst38;
	fConst40 = 1.0 - fConst17;
	fConst41 = fConst17 + 1.0;
	fConst42 = 1.0 / fConst41;
	fConst43 = 1.0 - fConst10;
	fConst44 = fConst10 + 1.0;
	fConst45 = 1.0 / fConst44;
	fConst46 = 1.0 / (fConst1 * fConst12);
	fConst47 = 1.0 - fConst4;
	fConst48 = fConst4 + 1.0;
	fConst49 = 1.0 / fConst48;
	fConst50 = 1.0 - fConst47 / fConst1;
	fConst51 = 1.0 / (fConst48 / fConst1 + 1.0);
	fConst52 = 1.0 / (fConst7 * fConst19);
	fConst53 = 1.0 / (fConst8 * fConst12);
	fConst54 = 1.0 - fConst43 / fConst7;
	fConst55 = 1.0 / (fConst44 / fConst7 + 1.0);
	fConst56 = 1.0 / (fConst14 * fConst26);
	fConst57 = 1.0 / (fConst15 * fConst19);
	fConst58 = 1.0 - fConst40 / fConst14;
	fConst59 = 1.0 / (fConst41 / fConst14 + 1.0);
	fConst60 = 1.0 / (fConst21 * fConst33);
	fConst61 = 1.0 / (fConst22 * fConst26);
	fConst62 = 1.0 - fConst37 / fConst21;
	fConst63 = 1.0 / (fConst38 / fConst21 + 1.0);
	fConst64 = 1.0 / (fConst29 * fConst33);
	clear_state_f();
	clear_state_internal();
}

inline void Dsp::compute(int count, float *input0, float *output0)
{
	double fSlow0 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider0));
	double fSlow1 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider1));
	double fSlow2 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider2));
	double fSlow3 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider3));
	double fSlow4 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider4));
	double fSlow5 = 0.0010000000000000009 * std::pow(1e+01, 0.05 * double(fVslider5));
	for (int i0 = 0; i0 < count; i0 = i0 + 1) {
		double fTemp0 = double(input0[i0]);
		fVec0[0] = fTemp0;
		fRec9[0] = -(fConst36 * (fConst35 * fRec9[1] - (fTemp0 + fVec0[1])));
		fRec8[0] = fRec9[0] - fConst34 * (fConst32 * fRec8[2] + fConst30 * fRec8[1]);
		double fTemp1 = fRec8[2] + fRec8[0] + 2.0 * fRec8[1];
		fVec1[0] = fTemp1;
		fRec7[0] = -(fConst39 * (fConst37 * fRec7[1] - fConst34 * (fTemp1 + fVec1[1])));
		fRec6[0] = fRec7[0] - fConst27 * (fConst25 * fRec6[2] + fConst23 * fRec6[1]);
		double fTemp2 = fRec6[2] + fRec6[0] + 2.0 * fRec6[1];
		fVec2[0] = fTemp2;
		fRec5[0] = -(fConst42 * (fConst40 * fRec5[1] - fConst27 * (fTemp2 + fVec2[1])));
		fRec4[0] = fRec5[0] - fConst20 * (fConst18 * fRec4[2] + fConst16 * fRec4[1]);
		double fTemp3 = fRec4[2] + fRec4[0] + 2.0 * fRec4[1];
		fVec3[0] = fTemp3;
		fRec3[0] = -(fConst45 * (fConst43 * fRec3[1] - fConst20 * (fTemp3 + fVec3[1])));
		fRec2[0] = fRec3[0] - fConst13 * (fConst11 * fRec2[2] + fConst9 * fRec2[1]);
		double fTemp4 = fRec2[2] + fRec2[0] + 2.0 * fRec2[1];
		fVec4[0] = fTemp4;
		fRec1[0] = 1e-18 -(fConst49 * (fConst47 * fRec1[1] - fConst46 * (fTemp4 - fVec4[1]))) + 1e-18;
		fRec0[0] = fRec1[0] - fConst6 * (fConst5 * fRec0[2] + fConst3 * fRec0[1]);
		fRec10[0] = fSlow0 + 0.999 * fRec10[1];
		fRec12[0] = -(fConst49 * (fConst47 * fRec12[1] - fConst13 * (fTemp4 + fVec4[1])));
		fRec11[0] = fRec12[0] - fConst6 * (fConst5 * fRec11[2] + fConst3 * fRec11[1]);
		fRec13[0] = fSlow1 + 0.999 * fRec13[1];
		double fTemp5 = fConst3 * fRec14[1];
		fRec16[0] = 1e-18 -(fConst45 * (fConst43 * fRec16[1] - fConst52 * (fTemp3 - fVec3[1]))) + 1e-18;
		fRec15[0] = 1e-18 + fRec16[0] - fConst13 * (fConst11 * fRec15[2] + fConst9 * fRec15[1]) - 1e-18;
		fRec14[0] = fConst53 * (fRec15[2] + (fRec15[0] - 2.0 * fRec15[1])) - fConst51 * (fConst50 * fRec14[2] + fTemp5);
		fRec17[0] = fSlow2 + 0.999 * fRec17[1];
		double fTemp6 = fConst3 * fRec18[1];
		double fTemp7 = fConst9 * fRec19[1];
		fRec21[0] = 1e-18 -(fConst42 * (fConst40 * fRec21[1] - fConst56 * (fTemp2 - fVec2[1]))) + 1e-18;
		fRec20[0] = 1e-18 + fRec21[0] - fConst20 * (fConst18 * fRec20[2] + fConst16 * fRec20[1]) - 1e-18;
		fRec19[0] = 1e-18 + fConst57 * (fRec20[2] + (fRec20[0] - 2.0 * fRec20[1])) - fConst55 * (fConst54 * fRec19[2] + fTemp7) - 1e-18;
		fRec18[0] = fRec19[2] + fConst55 * (fTemp7 + fConst54 * fRec19[0]) - fConst51 * (fConst50 * fRec18[2] + fTemp6);
		fRec22[0] = fSlow3 + 0.999 * fRec22[1];
		double fTemp8 = fConst3 * fRec23[1];
		double fTemp9 = fConst9 * fRec24[1];
		double fTemp10 = fConst16 * fRec25[1];
		fRec27[0] = 1e-18 -(fConst39 * (fConst37 * fRec27[1] - fConst60 * (fTemp1 - fVec1[1]))) + 1e-18;
		fRec26[0] = 1e-18 + fRec27[0] - fConst27 * (fConst25 * fRec26[2] + fConst23 * fRec26[1]) - 1e-18;
		fRec25[0] = 1e-18 + fConst61 * (fRec26[2] + (fRec26[0] - 2.0 * fRec26[1])) - fConst59 * (fConst58 * fRec25[2] + fTemp10) - 1e-18;
		fRec24[0] = 1e-18 + fRec25[2] + fConst59 * (fTemp10 + fConst58 * fRec25[0]) - fConst55 * (fConst54 * fRec24[2] + fTemp9) - 1e-18;
		fRec23[0] = fRec24[2] + fConst55 * (fTemp9 + fConst54 * fRec24[0]) - fConst51 * (fConst50 * fRec23[2] + fTemp8);
		fRec28[0] = fSlow4 + 0.999 * fRec28[1];
		double fTemp11 = fConst3 * fRec29[1];
		double fTemp12 = fConst9 * fRec30[1];
		double fTemp13 = fConst16 * fRec31[1];
		double fTemp14 = fConst23 * fRec32[1];
		fRec34[0] = -(fConst36 * (fConst35 * fRec34[1] - fConst31 * (fTemp0 - fVec0[1])));
		fRec33[0] = 1e-18 + fRec34[0] - fConst34 * (fConst32 * fRec33[2] + fConst30 * fRec33[1]) - 1e-18;
		fRec32[0] = 1e-18 + fConst64 * (fRec33[2] + (fRec33[0] - 2.0 * fRec33[1])) - fConst63 * (fConst62 * fRec32[2] + fTemp14) - 1e-18;
		fRec31[0] = 1e-18 + fRec32[2] + fConst63 * (fTemp14 + fConst62 * fRec32[0]) - fConst59 * (fConst58 * fRec31[2] + fTemp13) - 1e-18;
		fRec30[0] = 1e-18 + fRec31[2] + fConst59 * (fTemp13 + fConst58 * fRec31[0]) - fConst55 * (fConst54 * fRec30[2] + fTemp12) - 1e-18;
		fRec29[0] = fRec30[2] + fConst55 * (fTemp12 + fConst54 * fRec30[0]) - fConst51 * (fConst50 * fRec29[2] + fTemp11);
		fRec35[0] = fSlow5 + 0.999 * fRec35[1];
		output0[i0] = float(fRec35[0] * (fRec29[2] + fConst51 * (fTemp11 + fConst50 * fRec29[0])) + fRec28[0] * (fRec23[2] + fConst51 * (fTemp8 + fConst50 * fRec23[0])) + fRec22[0] * (fRec18[2] + fConst51 * (fTemp6 + fConst50 * fRec18[0])) + fRec17[0] * (fRec14[2] + fConst51 * (fTemp5 + fConst50 * fRec14[0])) + fConst6 * (fRec13[0] * (fRec11[2] + fRec11[0] + 2.0 * fRec11[1]) + fConst2 * fRec10[0] * (fRec0[0] + fRec0[2] - 2.0 * fRec0[1])));
		fVec0[1] = fVec0[0];
		fRec9[1] = fRec9[0];
		fRec8[2] = fRec8[1];
		fRec8[1] = fRec8[0];
		fVec1[1] = fVec1[0];
		fRec7[1] = fRec7[0];
		fRec6[2] = fRec6[1];
		fRec6[1] = fRec6[0];
		fVec2[1] = fVec2[0];
		fRec5[1] = fRec5[0];
		fRec4[2] = fRec4[1];
		fRec4[1] = fRec4[0];
		fVec3[1] = fVec3[0];
		fRec3[1] = fRec3[0];
		fRec2[2] = fRec2[1];
		fRec2[1] = fRec2[0];
		fVec4[1] = fVec4[0];
		fRec1[1] = fRec1[0];
		fRec0[2] = fRec0[1];
		fRec0[1] = fRec0[0];
		fRec10[1] = fRec10[0];
		fRec12[1] = fRec12[0];
		fRec11[2] = fRec11[1];
		fRec11[1] = fRec11[0];
		fRec13[1] = fRec13[0];
		fRec16[1] = fRec16[0];
		fRec15[2] = fRec15[1];
		fRec15[1] = fRec15[0];
		fRec14[2] = fRec14[1];
		fRec14[1] = fRec14[0];
		fRec17[1] = fRec17[0];
		fRec21[1] = fRec21[0];
		fRec20[2] = fRec20[1];
		fRec20[1] = fRec20[0];
		fRec19[2] = fRec19[1];
		fRec19[1] = fRec19[0];
		fRec18[2] = fRec18[1];
		fRec18[1] = fRec18[0];
		fRec22[1] = fRec22[0];
		fRec27[1] = fRec27[0];
		fRec26[2] = fRec26[1];
		fRec26[1] = fRec26[0];
		fRec25[2] = fRec25[1];
		fRec25[1] = fRec25[0];
		fRec24[2] = fRec24[1];
		fRec24[1] = fRec24[0];
		fRec23[2] = fRec23[1];
		fRec23[1] = fRec23[0];
		fRec28[1] = fRec28[0];
		fRec34[1] = fRec34[0];
		fRec33[2] = fRec33[1];
		fRec33[1] = fRec33[0];
		fRec32[2] = fRec32[1];
		fRec32[1] = fRec32[0];
		fRec31[2] = fRec31[1];
		fRec31[1] = fRec31[0];
		fRec30[2] = fRec30[1];
		fRec30[1] = fRec30[0];
		fRec29[2] = fRec29[1];
		fRec29[1] = fRec29[0];
		fRec35[1] = fRec35[0];
	}
}

Dsp *plugin() {
	return new Dsp();
}

void Dsp::del_instance(Dsp *p)
{
	delete p;
}
} // end namespace eq
