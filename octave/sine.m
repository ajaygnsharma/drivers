% This is the sample rate. Dont confuse with actual sine wave frequency
%
fs=10000;
p=1/fs;
total_sample_time=2;
x_axis=[0:p:total_sample_time];

% This is the actual sine wave. t defines how long the sine wave runs for.
% Basically show the entire waveform for the x axis.
freq=1;
y_axis=sin(2 * pi * freq * x_axis);

plot(x_axis, y_axis);
