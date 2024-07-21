RMS value calculation of a file.

* NOTICE that the v2 cancels itself out
V1 0 n1  SIN (0V 10V 1Hz 0 0 0)
V2 n1 n2 SIN (0V 10V 1Hz 0 0 180degrees)
R1 n2 0 100

.control
tran 10U 2S
plot n1 n2
.endc
.END
