# NeuralRack [![build](https://github.com/brummer10/NeuralRack/actions/workflows/build.yml/badge.svg)](https://github.com/brummer10/NeuralRack/actions/workflows/build.yml)


<p align="center">
    <img src="https://github.com/brummer10/NeuralRack/blob/main/NeuralRack.png?raw=true" />
</p>

NeuralRack is a Neural Model and Impulse Response File loader for Linux/Windows.

It supports [*.nam files](https://www.tone3000.com/search?tags=103) and, or 
[*.json or .aidax files](https://www.tone3000.com/search?tags=23562) by using the 
[NeuralAudio](https://github.com/mikeoliphant/NeuralAudio) engine.

For Impulse Response File Convolution it use [FFTConvolver](https://github.com/HiFi-LoFi/FFTConvolver)

Resampling is done by [Libzita-resampler](https://kokkinizita.linuxaudio.org/linuxaudio/zita-resampler/resampler.html)

NeuralRack emulate a simple guitar effect chain with a pedal, a EQ a Amplifier and a Stereo Cabinet.

Optional, NeuralRack could run one Model, or the complete process in buffered mode to reduce the dsp load. 
The resulting latency will be reported to the host so that it could be compensated. 
For information the resulting latency will be shown on the GUI.

NeuralRack supports resampling when needed to match the expected sample rate of the 
loaded models. Both models and the IR Files may have different expectations regarding the sample rate.

## Packaging Status

[![AUR package](https://repology.org/badge/version-for-repo/aur/neuralrack.svg)](https://repology.org/project/neuralrack/versions)
[![FreeBSD port](https://repology.org/badge/version-for-repo/freebsd/neuralrack-lv2.svg)](https://repology.org/project/neuralrack-lv2/versions)

To build NeuralRack only as standalone application run
```shell
make standalone
```

To build NeuralRack with all favours (currently as LV2 plugin and as standalone application) run
```shell
make
```

## Dependencies

- libsndfile1-dev
- libcairo2-dev
- libx11-dev
- lv2-dev

## Optional Dependencies to build the standalone version

- libjack(-jackd2)-dev
- portaudio19-dev

## Building LV2 plug from source code

```shell
git clone https://github.com/brummer10//NeuralRack.git
cd NeuralRack.lv2
git submodule update --init --recursive
make lv2
make install # will install into ~/.lv2 ... AND/OR....
sudo make install # will install into /usr/lib/lv2
```
