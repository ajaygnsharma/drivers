RMS value calculation of a file.

* This circuit does not work. Meant to be parrallel AC.
V1 n1 0 SIN (0V 10V 1Hz 0 0 0)
V2 n1 0 SIN (0V 10V 1Hz 0 0 0)
R1 n1 0 100

.control
tran 10U 2S
plot n1
.endc
.END
