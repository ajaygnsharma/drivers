RMS value calculation of a file.
RMS value calculation of a file.

*
V1 int 0 SIN (20V 10V 1Hz)
R1 int 0 100

.control
tran 10U 1S
* measure AC RMS_voltage RMS v(int) from=0m to=1s
plot int
.endc
.END
