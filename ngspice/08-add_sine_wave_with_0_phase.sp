RMS value calculation of a file.

*
V1 n1 0 SIN (0V 10V 1Hz 0 0 0)
V2 n1 n2 SIN (0V 10V 1Hz 0 0 0)
R1 n2 0 100

.control
tran 10U 2S
* measure AC RMS_voltage RMS v(int) from=0m to=1s
plot n2
.endc
.END
