 
*First simple simulation
V0 IN 0 PWL ( 0 0 ) ( 5e-9 3) (10e-9 1)
R1 IN 0 500
.TRAN 1P 20N
.MEAS tRAN sample find I(V0) AT=10n
.CONTROL
listing e
destroy all
SAVE V(IN) I(V0)
***
RUN
let abs_cur_vin = abs(i(v0))
MEAS TRAN sample_abs find abs_cur_vin AT=10n
***
display
PLOT V(IN)
display
plot I(V0)
.ENDC
.END
